
package com.picovr.picovrlib;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.widget.TextView;

import com.reverse.Loggvc;
import com.reverse.R;

public class VrLib extends Activity {

    private int TouchpadX;
    private int TouchpadY;
    private int HomeKeyPress;
    private int AppKeyPress;
    private int ClickKeyPress;
    private int VolumeUpKeyPress;
    private int VolumeDownKeyPress;
    private int BatteryLevel;

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
        tv.setText( "test" );
    }

    public void onClick(View v){
        if(v.getId() == R.id.button) {
//            nativeMsg();
            byte []arrbyte = {0x0D, 0x10, (byte)0x80, (byte)0xFC, 0x1B, 0x01, 0x04, 0x30, (byte)0xC0,
                    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x54, 0x00, 0x00, (byte)0xC0, (byte)0x98};
            nativeSensorEvent(arrbyte);
            nativeLark2KeyEvent(TouchpadX, TouchpadY, HomeKeyPress, AppKeyPress,
                    ClickKeyPress, VolumeUpKeyPress, VolumeDownKeyPress, BatteryLevel);
            Loggvc.i("onClick");
        }
    }

    public static native void nativeLark2KeyEvent(int paramInt1, int paramInt2, int paramInt3, int paramInt4, int paramInt5, int paramInt6, int paramInt7, int paramInt8);
    public static native void nativeSensorEvent(byte[] paramArrayOfByte);


    static
    {
        try {
            System.loadLibrary("PicoPlugin");
        }catch (Exception e){
            e.printStackTrace();
        }
    }
}
