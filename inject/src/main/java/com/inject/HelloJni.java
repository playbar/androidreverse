
package com.inject;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

public class HelloJni extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        Loggvc.e("HelloJni");
        setContentView(R.layout.activity_hello_jni);
        TextView tv = (TextView)findViewById(R.id.hello_textview);
        tv.setText( stringFromJNI_1() );
    }

    public void onClick(View v){
        if(v.getId() == R.id.button) {
            nativeMsg();
        }
    }
    /* A native method that is implemented by the
     * 'hello-jni' native library, which is packaged
     * with this application.
     */
    public native String  stringFromJNI();
    public native String stringFromJNI_1();
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
        System.loadLibrary("inject");
    }
}
