/*
 * importdex.cpp
 *
 *  Created on: 2014年6月24日
 *      Author: boyliang
 */

#include <stdio.h>
#include <stddef.h>
//#include <jni.h>
#include <android_runtime/AndroidRuntime.h>

#include "log.h"
#include "importdex.h"

using namespace android;

static const char JSTRING[] = "Ljava/lang/String;";
static const char JCLASS_LOADER[] = "Ljava/lang/ClassLoader;";
static const char JCLASS[] = "Ljava/lang/Class;";

static JNIEnv* jni_env;
static char sig_buffer[512];

//EntryClass entryClass;

//ClassLoader.getSystemClassLoader()
static jobject getSystemClassLoader(){

	LOGI("getSystemClassLoader is Executing!!");

	jclass class_loader_claxx = jni_env->FindClass("java/lang/ClassLoader");
	snprintf(sig_buffer, 512, "()%s", JCLASS_LOADER);

	LOGI("sig_buffer is %s",sig_buffer);

	jmethodID getSystemClassLoader_method = jni_env->GetStaticMethodID(class_loader_claxx, "getSystemClassLoader", sig_buffer);

	LOGI("getSystemClassLoader is finished!!");

	return jni_env->CallStaticObjectMethod(class_loader_claxx, getSystemClassLoader_method);

}

static bool jni_exception(){
   if(jni_env->ExceptionCheck())
	{
	   jthrowable exc;
       exc = jni_env->ExceptionOccurred();
	   jboolean isCopy = false;
	   jni_env->ExceptionClear();
       jmethodID toString = jni_env->GetMethodID(jni_env->FindClass("java/lang/Object"), "toString", "()Ljava/lang/String;");
       jstring s = (jstring)jni_env->CallObjectMethod(exc, toString);
       const char* utf = jni_env->GetStringUTFChars(s, &isCopy);
       LOGE("ExceptionOccurred:%s",utf);
       return true;
	}
	return false;
}

__attribute__ ((__constructor__))
void callback(char* param) {
    LOGI("param=%s", param);
    char* path = param;
    if (param == NULL) {
        path = "/data/inject.apk";
    }
    path = "/data/inject.apk";
	LOGI("Main is Executing!!");
	JavaVM* jvm = AndroidRuntime::getJavaVM();
	LOGI("jvm is %p",jvm);
    JavaVMAttachArgs args = {JNI_VERSION_1_4, NULL, NULL};
	jvm->AttachCurrentThread(&jni_env, (void*) &args);
	//TODO 使用JNIEnv

	//jvm->DetachCurrentThread();

	LOGI("jni_env is %p", jni_env);
    LOGI("path=%s", path);

	jstring apk_path = jni_env->NewStringUTF("/data/inject.apk");
	jstring dex_out_path = jni_env->NewStringUTF("/data");
	jclass dexloader_claxx = jni_env->FindClass("dalvik/system/DexClassLoader");

	// LOGI("apk_path:%s",apk_path);
	// LOGI("dex_out_path:%s",dex_out_path);

	snprintf(sig_buffer, 512, "(%s%s%s%s)V", JSTRING, JSTRING, JSTRING, JCLASS_LOADER);

	LOGI("sig_buffer is %s",sig_buffer);

	jmethodID dexloader_init_method = jni_env->GetMethodID(dexloader_claxx, "<init>", sig_buffer);

	snprintf(sig_buffer, 512, "(%s)%s", JSTRING, JCLASS);

	LOGI("sig_buffer is %s",sig_buffer);

	jmethodID loadClass_method = jni_env->GetMethodID(dexloader_claxx, "loadClass", sig_buffer);
    jobject nullObj;
	jobject class_loader = getSystemClassLoader();
    LOGI("getSystemClassLoader %p", class_loader);
	// if(JNI_TRUE == jni_env->IsSameObject(class_loader, nullObj)){
	  // LOGI("Failed GetClassLoader");
	// }else{
	   // LOGI("Succeeded GetClassLoader");
	// }
	//check_value(class_loader);


	jobject dex_loader_obj = jni_env->NewObject(dexloader_claxx, dexloader_init_method, apk_path, dex_out_path, NULL, class_loader);
	if(jni_exception()){
	  return;
	}
	LOGI("step---1");
	// if(JNI_TRUE == jni_env->IsSameObject(dex_loader_obj, nullObj)){
 //    	  LOGI("Failed dex_loader_obj");
 //    	}else{
 //    	   LOGI("Succeeded dex_loader_obj");
 //    	}
	jstring class_name = jni_env->NewStringUTF("com.demo.inject2.EntryClass");
	jclass entry_class = static_cast<jclass>(jni_env->CallObjectMethod(dex_loader_obj, loadClass_method, class_name));
	if(jni_exception()){
      return;
    }
	LOGI("step---2");
	LOGI("jni_env:%p",jni_env);
	LOGI("step---2-1");
	//LOGI("entry_class:%s",entry_class);
	jmethodID invoke_method = jni_env->GetStaticMethodID(entry_class, "invoke", "(I)[Ljava/lang/Object;");
    if(jni_exception()){
      return;
    }
	//check_value(invoke_method);
	LOGI("step---3");
	jobjectArray objectarray = (jobjectArray) jni_env->CallStaticObjectMethod(entry_class, invoke_method, 0);
	LOGI("step---4");
	jvm->DetachCurrentThread();

	LOGI("Main is finished");

}
