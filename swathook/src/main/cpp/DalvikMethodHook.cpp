#include <android_runtime/AndroidRuntime.h>

#include "JavaMethodHook.h"
#include "common.h"
#include "dvm.h"

using android::AndroidRuntime;

#ifdef DEBUG
#define STATIC
#else
#define STATIC static
#endif

//函数原型 dexProtoComputeArgsSize
STATIC int calcMethodArgsSize(const char* shorty) {
	int count = 0;

	/* Skip the return type. */
	shorty++;

	for (;;) {
		switch (*(shorty++)) {
		case '\0': {
			return count;
		}
		case 'D':
		case 'J': {
			count += 2;
			break;
		}
		default: {
			count++;
			break;
		}
		}
	}

	return count;
}

STATIC u4 dvmPlatformInvokeHints(const char* shorty) {
	const char* sig = shorty;
	int padFlags, jniHints;
	char sigByte;
	int stackOffset, padMask;

	stackOffset = padFlags = 0;
	padMask = 0x00000001;

	/* Skip past the return type */
	sig++;

	while (true) {
		sigByte = *(sig++);

		if (sigByte == '\0')
			break;

		if (sigByte == 'D' || sigByte == 'J') {
			if ((stackOffset & 1) != 0) {
				padFlags |= padMask;
				stackOffset++;
				padMask <<= 1;
			}
			stackOffset += 2;
			padMask <<= 2;
		} else {
			stackOffset++;
			padMask <<= 1;
		}
	}

	jniHints = 0;

	if (stackOffset > DALVIK_JNI_COUNT_SHIFT) {
		/* too big for "fast" version */
		jniHints = DALVIK_JNI_NO_ARG_INFO;
	} else {
		assert((padFlags & (0xffffffff << DALVIK_JNI_COUNT_SHIFT)) == 0);
		stackOffset -= 2;           // r2/r3 holds first two items
		if (stackOffset < 0)
			stackOffset = 0;
		jniHints |= ((stackOffset + 1) / 2) << DALVIK_JNI_COUNT_SHIFT;
		jniHints |= padFlags;
	}

	return jniHints;
}

STATIC int dvmComputeJniArgInfo(const char* shorty) {
	const char* sig = shorty;
	int returnType, jniArgInfo;
	u4 hints;

	/* The first shorty character is the return type. */
	switch (*(sig++)) {
	case 'V':
		returnType = DALVIK_JNI_RETURN_VOID;
		break;
	case 'F':
		returnType = DALVIK_JNI_RETURN_FLOAT;
		break;
	case 'D':
		returnType = DALVIK_JNI_RETURN_DOUBLE;
		break;
	case 'J':
		returnType = DALVIK_JNI_RETURN_S8;
		break;
	case 'Z':
	case 'B':
		returnType = DALVIK_JNI_RETURN_S1;
		break;
	case 'C':
		returnType = DALVIK_JNI_RETURN_U2;
		break;
	case 'S':
		returnType = DALVIK_JNI_RETURN_S2;
		break;
	default:
		returnType = DALVIK_JNI_RETURN_S4;
		break;
	}

	jniArgInfo = returnType << DALVIK_JNI_RETURN_SHIFT;

	hints = dvmPlatformInvokeHints(shorty);

	if (hints & DALVIK_JNI_NO_ARG_INFO) {
		jniArgInfo |= DALVIK_JNI_NO_ARG_INFO;
	} else {
		assert((hints & DALVIK_JNI_RETURN_MASK) == 0);
		jniArgInfo |= hints;
	}

	return jniArgInfo;
}

STATIC jclass dvmFindJNIClass(JNIEnv *env, const char *classDesc) {
	jclass classObj = env->FindClass(classDesc);

	if (classObj == NULL) {
		jclass clazzApplicationLoaders = env->FindClass(
				"android/app/ApplicationLoaders");
		CHECK_VALID(clazzApplicationLoaders);

		jfieldID fieldApplicationLoaders = env->GetStaticFieldID(
				clazzApplicationLoaders, "gApplicationLoaders",
				"Landroid/app/ApplicationLoaders;");
		CHECK_VALID(fieldApplicationLoaders);

		jobject objApplicationLoaders = env->GetStaticObjectField(
				clazzApplicationLoaders, fieldApplicationLoaders);
		CHECK_VALID(objApplicationLoaders);

		jfieldID fieldLoaders = env->GetFieldID(clazzApplicationLoaders,
				"mLoaders", "Ljava/util/Map;");
		CHECK_VALID(fieldLoaders);

		jobject objLoaders = env->GetObjectField(objApplicationLoaders,
				fieldLoaders);
		CHECK_VALID(objLoaders);

		jclass clazzHashMap = env->GetObjectClass(objLoaders);
		static jmethodID methodValues = env->GetMethodID(clazzHashMap, "values",
				"()Ljava/util/Collection;");
		jobject values = env->CallObjectMethod(objLoaders, methodValues);
		jclass clazzValues = env->GetObjectClass(values);
		static jmethodID methodToArray = env->GetMethodID(clazzValues,
				"toArray", "()[Ljava/lang/Object;");
		jobjectArray classLoaders = (jobjectArray) env->CallObjectMethod(values,
				methodToArray);

		int size = env->GetArrayLength(classLoaders);
		jstring param = env->NewStringUTF(classDesc);

		for (int i = 0; i < size; i++) {
			jobject classLoader = env->GetObjectArrayElement(classLoaders, i);
			jclass clazzCL = env->GetObjectClass(classLoader);
			static jmethodID loadClass = env->GetMethodID(clazzCL, "loadClass",
					"(Ljava/lang/String;)Ljava/lang/Class;");
			classObj = (jclass) env->CallObjectMethod(classLoader, loadClass,
					param);

			if (classObj != NULL) {
				break;
			}
		}
	}
	//局部变量返回，直接返回值使用会报错JNI ERROR (app bug): attempt to use stale local reference 0xfbc00025
	return (jclass) env->NewGlobalRef(classObj);
}

STATIC ClassObject* dvmFindClass(const char *classDesc) {
	JNIEnv *env = AndroidRuntime::getJNIEnv();
	assert(env != NULL);

	char *newclassDesc = dvmDescriptorToName(classDesc);

	jclass jnicls = dvmFindJNIClass(env, newclassDesc);
	ClassObject *res =
			jnicls ?
					static_cast<ClassObject*>(dvmDecodeIndirectRef(
							dvmThreadSelf(), jnicls)) :
					NULL;
	free(newclassDesc);
	return res;
}

/*
 * Return a new Object[] array with the contents of "args".  We determine
 * the number and types of values in "args" based on the method signature.
 * Primitive types are boxed.
 *
 * Returns NULL if the method takes no arguments.
 *
 * The caller must call dvmReleaseTrackedAlloc() on the return value.
 *
 * On failure, returns with an appropriate exception raised.
 */
STATIC ArrayObject* dvmBoxMethodArgs(const Method* method, const u4* args) {
	const char* desc = &method->shorty[1]; // [0] is the return type.

	/* count args */
	size_t argCount = dexProtoGetParameterCount(&method->prototype);

	STATIC ClassObject* java_lang_object_array = dvmFindSystemClass(
			"[Ljava/lang/Object;");

	/* allocate storage */
	ArrayObject* argArray = dvmAllocArrayByClass(java_lang_object_array,
			argCount, ALLOC_DEFAULT);
	if (argArray == NULL)
		return NULL;

	Object** argObjects = (Object**) (void*) argArray->contents;

	/*
	 * Fill in the array.
	 */
	size_t srcIndex = 0;
	size_t dstIndex = 0;
	while (*desc != '\0') {
		char descChar = *(desc++);
		JValue value;

		switch (descChar) {
		case 'Z':
		case 'C':
		case 'F':
		case 'B':
		case 'S':
		case 'I':
			value.i = args[srcIndex++];
			argObjects[dstIndex] = (Object*) dvmBoxPrimitive(value,
					dvmFindPrimitiveClass(descChar));
			/* argObjects is tracked, don't need to hold this too */
			dvmReleaseTrackedAlloc(argObjects[dstIndex], NULL);
			dstIndex++;
			break;
		case 'D':
		case 'J':
			value.j = dvmGetArgLong(args, srcIndex);
			srcIndex += 2;
			argObjects[dstIndex] = (Object*) dvmBoxPrimitive(value,
					dvmFindPrimitiveClass(descChar));
			dvmReleaseTrackedAlloc(argObjects[dstIndex], NULL);
			dstIndex++;
			break;
		case '[':
		case 'L':
			argObjects[dstIndex++] = (Object*) args[srcIndex++];
			break;
		}
	}

	return argArray;
}

STATIC ArrayObject* dvmGetMethodParamTypes(const Method* method,
		const char* methodsig) {
	/* count args */
	size_t argCount = dexProtoGetParameterCount(&method->prototype);
	STATIC ClassObject* java_lang_object_array = dvmFindSystemClass(
			"[Ljava/lang/Object;");

	/* allocate storage */
	ArrayObject* argTypes = dvmAllocArrayByClass(java_lang_object_array,
			argCount, ALLOC_DEFAULT);
	if (argTypes == NULL) {
		return NULL;
	}

	Object** argObjects = (Object**) argTypes->contents;
	const char *desc = (const char *) (strchr(methodsig, '(') + 1);

	/*
	 * Fill in the array.
	 */
	size_t desc_index = 0;
	size_t arg_index = 0;
	bool isArray = false;
	char descChar = desc[desc_index];

	while (descChar != ')') {

		switch (descChar) {
		case 'Z':
		case 'C':
		case 'F':
		case 'B':
		case 'S':
		case 'I':
		case 'D':
		case 'J':
			if (!isArray) {
				argObjects[arg_index++] = dvmFindPrimitiveClass(descChar);
				isArray = false;
			} else {
				char buf[3] = { 0 };
				memcpy(buf, desc + desc_index - 1, 2);
				argObjects[arg_index++] = dvmFindSystemClass(buf);
			}

			desc_index++;
			break;

		case '[':
			isArray = true;
			desc_index++;
			break;

		case 'L':
			int s_pos = desc_index, e_pos = desc_index;
			while (desc[++e_pos] != ';')
				;
			s_pos = isArray ? s_pos - 1 : s_pos;
			isArray = false;

			size_t len = e_pos - s_pos + 1;
			char buf[128] = { 0 };
			memcpy((void *) buf, (const void *) (desc + s_pos), len);
			argObjects[arg_index++] = dvmFindClass(buf);
			desc_index = e_pos + 1;
			break;
		}

		descChar = desc[desc_index];
	}

	return argTypes;
}

STATIC void method_handler(const u4* args, JValue* pResult,
		const Method* method, struct Thread* self) {
	HookInfo* info = (HookInfo*) method->insns;
	LOGI("entry %s->%s", info->classDesc, info->methodName);

	Method* originalMethod = reinterpret_cast<Method*>(info->originalMethod);
	Object* thisObject = (Object*) args[0];

	ArrayObject* argTypes = dvmBoxMethodArgs(originalMethod, args + 1);
	pResult->l = (void *) dvmInvokeMethod(thisObject, originalMethod, argTypes,
			(ArrayObject *) info->paramTypes, (ClassObject *) info->returnType,
			true);

	dvmReleaseTrackedAlloc((Object *) argTypes, self);
}

//=============================
//add by #123
//2014-10-14
//=============================
int ClearException(JNIEnv *jenv,bool showEx) {
	jthrowable exception = jenv->ExceptionOccurred();
	if (exception != NULL) {
		if (showEx)
			jenv->ExceptionDescribe();
		jenv->ExceptionClear();
		return true;
	}
	return false;
}

static inline void setTargetMethodNative(Method* method) {
	if (method != NULL) {
		int argsSize = calcMethodArgsSize(method->shorty);
		if (!dvmIsStaticMethod(method)) //非静态方法需要增加参数 this
			argsSize++;

		SET_METHOD_FLAG(method, ACC_NATIVE);
		method->registersSize = method->insSize = argsSize;
		method->outsSize = 0;
		method->jniArgInfo = dvmComputeJniArgInfo(method->shorty);
	}
}

/**
 * invokeObj this,静态函数时为NULL
 * meth 调用方法
 * argsize 参数个数
 *argsObjectContents 参数数组
 *methodsig 方法签名
 */
Object* invokeMethod(Object* invokeObj, const Method* meth, int argsize,
		Object** argsObjectContents, const char * methodsig) {
	if (meth != NULL) {
		int len = dexProtoGetParameterCount(&meth->prototype);
		if (argsize == len) {
			STATIC ClassObject* java_lang_object_array = dvmFindSystemClass(
					"[Ljava/lang/Object;");
			ArrayObject* argList = dvmAllocArrayByClass(java_lang_object_array,
					len, ALLOC_DEFAULT);
			Object ** argsContents = (Object**) argList->contents;
			for (int i = 0; i < len; i++)
				argsContents[i] = argsObjectContents[i];

			ArrayObject * paramTypes = dvmGetMethodParamTypes(meth, methodsig);
			ClassObject * returnType = dvmGetBoxedReturnType(meth);
			Object *result = dvmInvokeMethod(invokeObj, meth, argList,
					paramTypes, returnType,
					true);
			::Thread* self = dvmThreadSelf();
			dvmReleaseTrackedAlloc(argList, self);
			dvmReleaseTrackedAlloc(paramTypes, self);
			return result;
		} else
			LOGE("Error:invokeMethod argsize is %d but you input %d", len,
					argsize);
	} else
		LOGE("Error:invokeMethod meth is %p", meth);
	return NULL;
}
static void handleInvoker(const u4* args, JValue* pResult, const Method* method,
		::Thread* self) {
	LOGI("JNI calling %p (%s.%s:%s):", method->insns, method->clazz->descriptor,
			method->name, method->shorty);

	HookInfo* info = (HookInfo*) method->insns;
	Method* originalMethod = reinterpret_cast<Method*>(info->originalMethod);

	Object* thisObject = NULL;
	ArrayObject* argsArray = NULL;
	if (!dvmIsStaticMethod(method)) {
		thisObject = (Object*) args[0];
		argsArray = dvmBoxMethodArgs(originalMethod, args + 1);
	} else
		argsArray = dvmBoxMethodArgs(originalMethod, args);

	JNIEnv *env = AndroidRuntime::getJNIEnv();
	//init hookparam
	static const char * setParamSig =
			"(Ljava/lang/reflect/Member;Ljava/lang/Object;[Ljava/lang/Object;)V";
	const static jclass classHookParam = dvmFindJNIClass(env,
			"com/swathook/HookCallBack$HookParam");
	jmethodID constructorHookParam = env->GetMethodID(classHookParam, "<init>",
			"()V");
	jobject jobHookParam = env->NewObject(classHookParam, constructorHookParam);
	jmethodID methodsetParam = env->GetMethodID(classHookParam, "setParam",
			setParamSig);

	char *orMethodDesc = dvmDescriptorToName(originalMethod->clazz->descriptor);
	jclass targetClass = dvmFindJNIClass(env, orMethodDesc);
	free(orMethodDesc);
	jobject originalReflected = env->ToReflectedMethod(targetClass,
			(jmethodID) originalMethod, dvmIsStaticMethod(method));
	LOGD("originalReflected->%p", originalReflected);

	Object *parOb = dvmDecodeIndirectRef(self, jobHookParam);
	if (methodsetParam != NULL) {
		Object* setParamArgsList[] = { dvmDecodeIndirectRef(self,
				originalReflected), thisObject, argsArray };
		invokeMethod(parOb, (Method*) methodsetParam, 3, setParamArgsList,
				setParamSig);
	}

	//init callback
	static const char * callbackMethodSig =
			"(Lcom/swathook/HookCallBack$HookParam;)V";
	jclass classHookCallBack = NULL;
	if (info->callback != NULL) {
		char * callBackDes = dvmDescriptorToName(
				((Object *) info->callback)->clazz->descriptor);
		classHookCallBack = dvmFindJNIClass(env, callBackDes);
		free(callBackDes);

		//invoke callback beforeHookedMethod
		jmethodID methodHookCallBack = env->GetMethodID(classHookCallBack,
				"beforeHookedMethod", callbackMethodSig);
		LOGD("beforeHookedMethod->%p", methodHookCallBack);
		Object* argslist[] = { parOb };
		invokeMethod((Object *) info->callback, (Method*) methodHookCallBack, 1,
				argslist, callbackMethodSig);
		ClearException(env,true);
	}

	Object * result = NULL;
	jobject jObresult = NULL;
	//check result is OK
	jfieldID returnEarlyId = env->GetFieldID(classHookParam, "returnEarly",
			"Z");
	jboolean returnEarly = env->GetBooleanField(jobHookParam, returnEarlyId);
	jmethodID getResultID = env->GetMethodID(classHookParam, "getResult",
			"()Ljava/lang/Object;");

	if (returnEarly == JNI_FALSE) {
		//invoke Original Method
		LOGD("invoke Original Method");
		result = dvmInvokeMethod(thisObject, originalMethod, argsArray,
				(ArrayObject *) info->paramTypes,
				(ClassObject *) info->returnType,
				true);
		//invoke HookParam setResult
		static const char * setResultSig = "(Ljava/lang/Object;)V";
		jmethodID setResultID = env->GetMethodID(classHookParam, "setResult",
				setResultSig);
		if (setResultID != NULL) {
			Object* argslist[] = { result };
			invokeMethod(parOb, (Method*) setResultID, 1, argslist,
					setResultSig);
		}

		//invoke callback afterHookedMethod
		if (classHookCallBack != NULL) {
			jmethodID methodHookCallBack = env->GetMethodID(classHookCallBack,
					"afterHookedMethod", callbackMethodSig);
			LOGD("afterHookedMethod->%p", methodHookCallBack);
			Object* argslist[] = { parOb };
			invokeMethod((Object *) info->callback,
					(Method*) methodHookCallBack, 1, argslist,
					callbackMethodSig);
			returnEarly = env->GetBooleanField(jobHookParam, returnEarlyId);
			if (returnEarly == JNI_TRUE)
				jObresult = env->CallObjectMethod(jobHookParam, getResultID);
		}
	} else
		jObresult = env->CallObjectMethod(jobHookParam, getResultID);

	if (jObresult != NULL)
		result = dvmDecodeIndirectRef(self, jObresult);

	if (((ClassObject *) info->returnType)->primitiveType != PRIM_VOID)
		pResult->l = (void *) result;
}

extern int __attribute__ ((visibility ("hidden"))) dalvik_java_method_hook(
		JNIEnv* env, HookInfo *info, jobject callback) {

	const char* classDesc = info->classDesc;
	const char* methodName = info->methodName;
	const char* methodSig = info->methodSig;
	const bool isStaticMethod = info->isStaticMethod;

	jclass classObj = dvmFindJNIClass(env, classDesc);
	if (classObj == NULL) {
		LOGE("[-] %s class not found", classDesc);
		return -1;
	}

	jmethodID methodId = env->GetMethodID(classObj, methodName, methodSig);
	if (ClearException(env,false))
		methodId = env->GetStaticMethodID(classObj, methodName, methodSig);

	if (methodId == NULL) {
		LOGE("[-] %s->%s method not found", classDesc, methodName);
		return -1;
	}

	// backup method
	Method* method = (Method*) methodId;
	info->isStaticMethod = dvmIsStaticMethod(method);
	if (method->nativeFunc == handleInvoker) {
		LOGW("[*] %s->%s method had been hooked", classDesc, methodName);
		free(info);
		return -1;
	}

	Method* bakMethod = (Method*) malloc(sizeof(Method));
	memcpy(bakMethod, method, sizeof(Method));

	// init info
	info->originalMethod = (void *) bakMethod;
	info->returnType = (void *) dvmGetBoxedReturnType(bakMethod);
	info->paramTypes = dvmGetMethodParamTypes(bakMethod, info->methodSig);
	info->callback = NULL;
	if (callback != NULL) {
		::Thread* self = dvmThreadSelf();
		info->callback = dvmDecodeIndirectRef(self, callback);
	}

	// set method native
	setTargetMethodNative(method);

	// save info to insns
	method->insns = (u2*) info;

	// bind the bridge func，only one line
	method->nativeFunc = handleInvoker;
	LOGI("[+] %s->%s was hooked\n", classDesc, methodName);

	return 0;
}

