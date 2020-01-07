package com.valgrind.test;

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

    static
    {
        System.loadLibrary("tcmalloctest");
    }
}
