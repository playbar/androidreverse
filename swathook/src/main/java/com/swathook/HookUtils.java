package com.swathook;

import java.lang.reflect.Constructor;
import java.lang.reflect.Member;
import java.lang.reflect.Method;

public class HookUtils {
	// javah -classpath bin/classes -d jni com.swathook.HookUtils

	static {
		System.loadLibrary("SwatHookV2");
	}

	public native String stringFromJNI();


	public static int hookMethod(Member method, String methodsig,
			HookCallBack callback) {
		int result = -1;
		if (method instanceof Method || method instanceof Constructor<?>) {
			String clsdes = method.getDeclaringClass().getName()
					.replace('.', '/');
			String methodname = method.getName();
			result = hookMethodNative(clsdes, methodname, methodsig, callback);
		}
		return result;
	}

	public static int hookMethod(String clsdes, String methodname,
			String methodsig, HookCallBack callback) {
		int result = -1;
		if (clsdes != null && methodname != null) {
			result = hookMethodNative(clsdes, methodname, methodsig, callback);
		}
		return result;
	}

	public static int hookMethod2(Member method, String methodsig) {
		int result = -1;

		if (method instanceof Method || method instanceof Constructor<?>) {
			String clsdes = method.getDeclaringClass().getName()
					.replace('.', '/');

			String methodname = method.getName();

			result = hookMethodNative(clsdes, methodname, methodsig,
					new HookCallBack() {
						@Override
						protected void afterHookedMethod(HookParam par) {
							System.out.println("afterHookedMethod->"
									+ par.getResult());
							super.afterHookedMethod(par);
						}

						@Override
						protected void beforeHookedMethod(HookParam par) {
							par.args[1] = "test";
							for (Object ob : par.args)
								System.out.println("ob->" + ob.toString());
						}
					});
		}
		return result;
	}

	private static native int hookMethodNative(String clsdes,
			String methodname, String methodsig, HookCallBack callback);
}
