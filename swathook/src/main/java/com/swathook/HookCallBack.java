package com.swathook;

import java.lang.reflect.Member;

public class HookCallBack{

	protected void beforeHookedMethod(HookParam par)
	{
	}
	
	protected void afterHookedMethod(HookParam par)
	{
	}
	
	public static class HookParam
	{
		
		public Member method;
		public Object thisObject;
		public Object[] args;
		private Object result = null;
		boolean returnEarly = false;
		
		public Object getResult()
		{
			return result;
		}
		
		public void setResult(Object result) {
			this.result = result;
			this.returnEarly = true;
		}
		
		public void setParam(Member method,Object thisObject,Object[] args)
		{
			this.method = method;
			if(method!=null)
				System.out.println("setParam->"+method.getName());
			this.thisObject = thisObject;
			this.args = args;
		}
		
	}
}
