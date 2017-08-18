package com.hook.project;

import java.lang.reflect.Field;
import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

import android.content.Context;
import android.content.Intent;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

public class HookUtils {
	private static final String TAG = "hook";
	private Class<?> proxyActivity;
	private Context context;
	private Object activityThreadValue;

	public HookUtils(Context context, Class<?> proxyActivity) {
		this.context = context;
		this.proxyActivity = proxyActivity;
	}

	public void hookAms() throws Exception {
		Class<?> forName = Class.forName("android.app.ActivityManagerNative");
		Field defaultField = forName.getDeclaredField("gDefault");
		defaultField.setAccessible(true);
		Object defaultValue = defaultField.get(null);
		Class<?> forName2 = Class.forName("android.util.Singleton");
		Field instanceField = forName2.getDeclaredField("mInstance");
		instanceField.setAccessible(true);
		Object iActivityManagerObject = instanceField.get(defaultValue);
		Class<?> iActivity = Class.forName("android.app.IActivityManager");
		InvocationHandler handler = new AmsInvocationHandler(iActivityManagerObject);
		Object proxy = Proxy.newProxyInstance(Thread.currentThread().getContextClassLoader(),
				new Class<?>[] { iActivity }, handler);
		instanceField.set(defaultValue, proxy);
	}

	class AmsInvocationHandler implements InvocationHandler {
		private Object iActivity;

		public AmsInvocationHandler(Object iActivity) {
			this.iActivity = iActivity;
		}

		@Override
		public Object invoke(Object proxy, Method method, Object[] args) throws Throwable {
			Log.i(TAG, "invoke, " + method.getName());
			if ("startActivity".contains(method.getName())) {
				Intent intent = null;
				int index = 0;
				for (int i = 0; i < args.length; i++) {
					if (args[i] instanceof Intent) {
						index = i;
						break;
					}
				}
				intent = new Intent(context, proxyActivity);
				Intent realIntent = (Intent) args[index];
				intent.putExtra("oldIntent", realIntent);
				args[index] = intent;
			}
			return method.invoke(iActivity, args);
		}
	}

	public void hookSystemHandler() throws Exception {
		Class<?> forName = Class.forName("android.app.ActivityThread");
		Field field = forName.getDeclaredField("sCurrentActivityThread");
		field.setAccessible(true);
		activityThreadValue = field.get(forName);
		Field mH = forName.getDeclaredField("mH");
		mH.setAccessible(true);
		Object handler = mH.get(activityThreadValue);
		Class<?> handlerClass = handler.getClass().getSuperclass();
		Field callbackField = handlerClass.getDeclaredField("mCallback");
		callbackField.setAccessible(true);
		HookCallback callback = new HookCallback((Handler) handler);
		callbackField.set(handler, callback);
		Log.i(TAG, "hook system handler completed.");
	}

	class HookCallback implements Handler.Callback {
		Handler handler;

		public HookCallback(Handler handler) {
			this.handler = handler;
		}

		@Override
		public boolean handleMessage(Message msg) {
			Log.i(TAG, "message callback.");
			if (msg.what == 100) {
				Log.i("hook", "handle message 100.");
				handleLaunchActivity(msg);
			}
			handler.handleMessage(msg);
			return true;
		}

		private void handleLaunchActivity(Message msg) {
			Object obj = msg.obj;
			try {
				Field intentField = obj.getClass().getDeclaredField("intent");
				intentField.setAccessible(true);
				Intent proxyIntent = (Intent) intentField.get(obj);
				Intent realIntent = proxyIntent.getParcelableExtra("oldIntent");
				if (realIntent != null) {
					proxyIntent.setComponent(realIntent.getComponent());
				}
			} catch (Exception e) {
				e.printStackTrace();
			}
		}
	}
}
