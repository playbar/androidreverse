
package com.inline.hook;

import android.app.Activity;
import android.opengl.GLException;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

public class HelloJni extends Activity {

    String mStr;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
//        GLESHook.initHook();
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        Loggvc.e("HelloJni");
        setContentView(R.layout.activity_hello_jni);
        TextView tv = (TextView)findViewById(R.id.hello_textview);
        mStr = stringFromJNI();
        int ilen = mStr.length();
        Log.e("test", "len:" + ilen);
        tv.setText( mStr );
    }

    public void onClick(View v){
        if(v.getId() == R.id.button) {
            testNativeArray();
            nativeMsg();
        }
    }

    public void testNativeArray()
    {
        for( int count = 0; count < 4; ++count) {
            DiskInfo[] infos = getStructArray();
            for (int i = 0; i < infos.length; i++) {
                Log.e("native_array", infos[i].name + " : " + infos[i].serialNo);
            }
            Log.e("test", "str:" + mStr);
        }
        Log.e("native_array", "done");
    }

    public native String  stringFromJNI();
    public native String stringFromJNI_1();
    public native DiskInfo[] getStructArray();
    public native void nativeMsg();

    /* This is another native method declaration that is *not*
     * implemented by 'hello-jni'. This is simply to show that
     * you can declare as many native methods in your Java code
     * as you want, their implementation is searched in the
     * currently loaded native libraries only the first time
     * you call them.
     *
     * Trying to call this function will result in a
     * java.lang.UnsatisfiedLinkError exception !
     */
//    public native String  unimplementedStringFromJNI();

    /* this is used to load the 'hello-jni' library on application
     * startup. The library has already been unpacked into
     * /data/data/com.bar.hellojni/lib/libhello-jni.so at
     * installation time by the package manager.
     */

    static
    {
        try {
            System.loadLibrary("glhook");
        }catch (Exception e){
            Log.e("error", e.getMessage());
        }
    }
}
