package cn.syang2forever.androidinject;

import android.app.Service;
import android.content.Intent;
import android.os.IBinder;

// This is a basic service do nothing but we can inject into this process!
public class ChildService extends Service {

    static {
        System.loadLibrary("self");
    }

    public ChildService() {
    }

    @Override
    public IBinder onBind(Intent intent) {
        // TODO: Return the communication channel to the service.
        return null;
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        // test mmap function
        mmapNative();

        // test dlopen dlsym dlclose dlerror
        String path = ChildService.this.getApplicationInfo().nativeLibraryDir + "/libhello.so";
        String fun = "hook_entry";
        String param = "This is a debug test!";
        dlNatvie(path, fun, param);
        return super.onStartCommand(intent, flags, startId);
    }

    // use the self part to help debug
    public native void mmapNative();
    public native void dlNatvie(String path, String function, String param);
}
