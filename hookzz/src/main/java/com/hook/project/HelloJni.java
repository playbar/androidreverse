/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
package com.hook.project;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.hook.R;
import com.hook.project.Loggvc;


public class HelloJni extends Activity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        /* Retrieve our TextView and set its content.
         * the text is retrieved by calling a native
         * function.
         */
        Loggvc.e("log");
        setContentView(R.layout.activity_hello_jni);
        TextView tv = (TextView)findViewById(R.id.hello_textview);
        tv.setText( stringFromJNITest() );
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
    public native String  stringFromJNITest();
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
        System.loadLibrary("hook");
    }
}
