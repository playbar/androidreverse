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
package com.asm.test;

import android.app.Activity;
import android.app.ActivityManager;
import android.content.Context;
import android.os.Bundle;
import android.os.Debug;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.BufferedReader;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.text.DecimalFormat;
import java.text.NumberFormat;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;


public class HelloJni extends Activity {

    public static final String TAG = "meminfo";

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
        tv.setText( stringFromJNI() );
    }

    public void onClick(View v){
        if(v.getId() == R.id.button) {
//            nativeMsg();
            long strTotalMem = getTotalMem();
            long strFreeMem = getFreeMem(this);
            int mem = getAppMemory();
            Log.e("meminfo","Total: " + strTotalMem + ", Free: " + strFreeMem +", mem : " + mem);

            displayBriefMemory();
            getRunningAppProcessInfo();

        }
    }
    /* A native method that is implemented by the
     * 'hello-jni' native library, which is packaged
     * with this application.
     */
    public native String  stringFromJNI();
    public native String stringFromJNI_1();
    public native void nativeMsg();

    public static int getAppMemory() {
        Debug.MemoryInfo memoryInfo = new Debug.MemoryInfo();
        Debug.getMemoryInfo(memoryInfo);
        // dalvikPrivateClean + nativePrivateClean + otherPrivateClean;
        int totalPrivateClean = memoryInfo.getTotalPrivateClean();
        // dalvikPrivateDirty + nativePrivateDirty + otherPrivateDirty;
        int totalPrivateDirty = memoryInfo.getTotalPrivateDirty();
        // dalvikPss + nativePss + otherPss;
        int totalPss = memoryInfo.getTotalPss();
        // dalvikSharedClean + nativeSharedClean + otherSharedClean;
        int totalSharedClean = memoryInfo.getTotalSharedClean();
        // dalvikSharedDirty + nativeSharedDirty + otherSharedDirty;
        int totalSharedDirty = memoryInfo.getTotalSharedDirty();
        // dalvikSwappablePss + nativeSwappablePss + otherSwappablePss;
        int totalSwappablePss = memoryInfo.getTotalSwappablePss();

        int total = totalPrivateClean + totalPrivateDirty + totalPss + totalSharedClean + totalSharedDirty + totalSwappablePss;
        return total ;
    }



    private void displayBriefMemory() {

        final ActivityManager activityManager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);

        ActivityManager.MemoryInfo info = new ActivityManager.MemoryInfo();

        activityManager.getMemoryInfo(info);

        Log.e(TAG,"系统总内存:"+(info.totalMem >> 10)+"k");

        Log.e(TAG,"系统剩余内存:"+(info.availMem >> 10)+"k");

        Log.e(TAG,"系统是否处于低内存运行："+info.lowMemory);

        Log.e(TAG,"当系统剩余内存低于"+info.threshold+"时就看成低内存运行");

        int memory = activityManager.getMemoryClass();
        float maxMemory = (float) (Runtime.getRuntime().maxMemory() * 1.0/ (1024 * 1024));
        float totalMemory = (float) (Runtime.getRuntime().totalMemory() * 1.0/ (1024 * 1024));
        float freeMemory = (float) (Runtime.getRuntime().freeMemory() * 1.0/ (1024 * 1024));

        Log.e(TAG, "memory : " + memory +", maxMemory = " +maxMemory + ", totalMem = "
                + totalMemory + ", freeMem = " + freeMemory );

    }


    // 获得系统进程信息
    private void getRunningAppProcessInfo() {
        // 通过调用ActivityManager的getRunningAppProcesses()方法获得系统里所有正在运行的进程
        final ActivityManager activityManager = (ActivityManager) getSystemService(ACTIVITY_SERVICE);
        List<ActivityManager.RunningAppProcessInfo> appProcessList = activityManager.getRunningAppProcesses();

        for (ActivityManager.RunningAppProcessInfo appProcessInfo : appProcessList) {
            // 进程ID号
            int pid = appProcessInfo.pid;
            // 用户ID 类似于Linux的权限不同，ID也就不同 比如 root等
            int uid = appProcessInfo.uid;
            // 进程名，默认是包名或者由属性android：process=""指定
            String processName = appProcessInfo.processName;
            // 获得该进程占用的内存
            int[] myMempid = new int[] { pid };
            // 此MemoryInfo位于android.os.Debug.MemoryInfo包中，用来统计进程的内存信息
            Debug.MemoryInfo[] memoryInfo = activityManager.getProcessMemoryInfo(myMempid);
            // 获取进程占内存用信息 kb单位
            int memSize = memoryInfo[0].dalvikPrivateDirty;

            Log.e(TAG, "processName: " + processName + "  pid: " + pid
                    + " uid:" + uid + " memorySize is -->" + memSize + "kb");

            // 获得每个进程里运行的应用程序(包),即每个应用程序的包名
            String[] packageList = appProcessInfo.pkgList;
            Log.e(TAG, "process id is " + pid + "has " + packageList.length);
            for (String pkg : packageList) {
                Log.e(TAG, "packageName " + pkg + " in process id is -->"+ pid);
            }
        }
    }


    /*
    （1）MemTotal: 所有可用RAM大小。（即物理内存减去一些预留位和内核的二进制代码大小）
（2）MemFree: LowFree与HighFree的总和，被系统留着未使用的内存。
（3）Buffers: 用来给文件做缓冲大小。
（4）Cached: 被高速缓冲存储器（cache memory）用的内存的大小（等于diskcache minus SwapCache）。
（5）SwapCached:被高速缓冲存储器（cache memory）用的交换空间的大小。已经被交换出来的内存，仍然被存放在swapfile中，用来在需要的时候很快的被替换而不需要再次打开I/O端口。
（6）Active: 在活跃使用中的缓冲或高速缓冲存储器页面文件的大小，除非非常必要，否则不会被移作他用。
（7）Inactive: 在不经常使用中的缓冲或高速缓冲存储器页面文件的大小，可能被用于其他途径。
（8）SwapTotal: 交换空间的总大小。
（9）SwapFree: 未被使用交换空间的大小。
（10）Dirty: 等待被写回到磁盘的内存大小。
（11）Writeback: 正在被写回到磁盘的内存大小。
（12）AnonPages：未映射页的内存大小。
（13）Mapped: 设备和文件等映射的大小。
（14）Slab: 内核数据结构缓存的大小，可以减少申请和释放内存带来的消耗。
（15）SReclaimable:可收回Slab的大小。
（16）SUnreclaim：不可收回Slab的大小（SUnreclaim+SReclaimable＝Slab）。
（17）PageTables：管理内存分页页面的索引表的大小。
（18）NFS_Unstable:不稳定页表的大小。

     */


    public static long getTotalMem() {
        try {
            FileReader fr = new FileReader("/proc/meminfo");
            BufferedReader br = new BufferedReader(fr);
            String text = br.readLine();
            String[] array = text.split("\\s+");
            Log.w(TAG, text);
            // 单位为KB
            return Long.valueOf(array[1]);
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
        return -1;
    }


    public static long getFreeMem(Context context) {
        ActivityManager manager = (ActivityManager) context
                .getSystemService(Activity.ACTIVITY_SERVICE);
        ActivityManager.MemoryInfo info = new ActivityManager.MemoryInfo();
        manager.getMemoryInfo(info);
        // 单位Byte
        return info.availMem;
    }


    private static final String FILENAME_PROC_MEMINFO = "/proc/meminfo";
    /**
     * 获取手机内存总大小
     * @return
     */
    public static String getTotalMemorySize() {
        try {
            FileReader fr = new FileReader(FILENAME_PROC_MEMINFO);
            BufferedReader br = new BufferedReader(fr, 2048);
//            String memoryLine = br.readLine();
            String subMemoryLine = "";
            String Line = "";
            while ((Line = br.readLine()) != null)
            {
                if (Line.contains("MemTotal:")){
                    subMemoryLine = Line.substring(Line.indexOf("MemTotal:"));
                    break;
                }

            }
            br.close();
            Matcher mer = Pattern.compile("^[0-9]+$").matcher(subMemoryLine.replaceAll("\\D+", ""));
            //如果为正整数就说明数据正确的，确保在Double.parseDouble中不会异常
            if (mer.find()) {
                long memSize = Integer.parseInt(subMemoryLine.replaceAll("\\D+", "")) ;
                double mem = (Double.parseDouble(memSize + "")/1024)/1024;
                NumberFormat nf = new DecimalFormat( "0.0 ");
                mem = Double.parseDouble(nf.format(mem));
                //Log.e(LOG_TAG,"=========mem================ " + mem);
                return String.valueOf(mem);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return "Unavailable";
    }

    /**
     * 获取手机剩余内存大小
     * @return
     */
    public static String getFreeMemorySize() {
        try {
            FileReader fr = new FileReader(FILENAME_PROC_MEMINFO);
            BufferedReader br = new BufferedReader(fr, 2048);
//            String memoryLine = br.readLine();
            String subMemoryLine = "";
            String Line = "";
            while ((Line = br.readLine()) != null)
            {
                if (Line.contains("MemAvailable:")){
                    subMemoryLine = Line.substring(Line.indexOf("MemAvailable:"));
                    break;
                }

            }
            br.close();
            Matcher mer = Pattern.compile("^[0-9]+$").matcher(subMemoryLine.replaceAll("\\D+", ""));
            //如果为正整数就说明数据正确的，确保在Double.parseDouble中不会异常
            if (mer.find()) {
                long memSize = Integer.parseInt(subMemoryLine.replaceAll("\\D+", "")) ;
                double mem = (Double.parseDouble(memSize + "")/1024)/1024;
                NumberFormat nf = new DecimalFormat( "0.0 ");
                mem = Double.parseDouble(nf.format(mem));
                //Log.e(LOG_TAG,"=========mem================ " + mem);
                return String.valueOf(mem);
            }
        } catch (IOException e) {
            e.printStackTrace();
        }
        return "Unavailable";
    }



    static
    {
        System.loadLibrary("reverse");
    }
}
