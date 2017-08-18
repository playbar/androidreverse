package com.hook.project;

import android.app.Application;

public class HookApplication extends Application{

	@Override
	public void onCreate() {
		super.onCreate();
		HookUtils hook = new HookUtils(this, HookActivity.class);
		try {
			hook.hookAms();
			hook.hookSystemHandler();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
