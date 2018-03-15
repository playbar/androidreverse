package com.android.inject;

import android.content.Intent;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;

public class MainActivity extends AppCompatActivity {

    static {
        System.loadLibrary("hello");
        System.loadLibrary("inject");
    }

    TextView mTextView;
    EditText mEditView;
    Button mButton;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        mTextView = (TextView) findViewById(R.id.text);
        mEditView = (EditText) findViewById(R.id.input);
        mButton = (Button) findViewById(R.id.btn);

        // start service!
        Intent intent = new Intent(MainActivity.this, ChildService.class);
        startService(intent);

        // first say hello!
        sayHello();

        // second to attach!
        mButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                inject();
            }
        });
    }

    // the so basic function
    public native void sayHello();

    // inject into the process
    public native int injectProcess(String processName, String soName, String function, String param);
    public native int injectProcess2(int pid, String so, String fun, String param);

    public int injectProcessWrapper(String input){
        if(input.equals("")){
            input = "com.android.inject:child";
        }

        String soPath = MainActivity.this.getApplicationInfo().nativeLibraryDir + "/libhello.so";
        String fun = "hook_entry";
        String param = "This is a hook!";

        try{
            int pid = Integer.parseInt(input);
            return injectProcess2(pid, soPath, fun, param);
        }catch (Exception ex){
            return injectProcess(input, soPath, fun, param);
        }
    }

    public void inject(){
        String param = mEditView.getText().toString();
        if(injectProcessWrapper(param) > 0){
            mTextView.setText("Success!");
        }else {
            mTextView.setText("Failed");
        }
    }
}
