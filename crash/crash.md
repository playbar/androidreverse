使用objdump进行Android crash 日志 分析

崩溃日志
11-08 14:05:38.833 2002-2011/? E/ANDR-PERF-RESOURCEQS: Failed to apply optimization [2, 0]
11-08 14:05:38.846 23213-23213/? A/libc: Fatal signal 11 (SIGSEGV), code 1, fault addr 0x0 in tid 23213 (com.crash.test), pid 23213 (com.crash.test)
11-08 14:05:38.882 23263-23263/? A/DEBUG: *** *** *** *** *** *** *** *** *** *** *** *** *** *** *** ***
11-08 14:05:38.882 23263-23263/? A/DEBUG: Build fingerprint: 'google/walleye/walleye:8.1.0/OPM1.171019.021/4565141:user/release-keys'
11-08 14:05:38.882 23263-23263/? A/DEBUG: Revision: 'MP1'
11-08 14:05:38.883 23263-23263/? A/DEBUG: ABI: 'arm'
11-08 14:05:38.883 23263-23263/? A/DEBUG: pid: 23213, tid: 23213, name: com.crash.test  >>> com.crash.test <<<
11-08 14:05:38.883 23263-23263/? A/DEBUG: signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x0
11-08 14:05:38.883 23263-23263/? A/DEBUG: Cause: null pointer dereference
11-08 14:05:38.883 23263-23263/? A/DEBUG:     r0 00000000  r1 0000000a  r2 7b6c4bbd  r3 ffadfed0
11-08 14:05:38.883 23263-23263/? A/DEBUG:     r4 f1645044  r5 00000001  r6 00000000  r7 ffadff80
11-08 14:05:38.883 23263-23263/? A/DEBUG:     r8 00000000  r9 f05da000  sl ffae0180  fp ffae010c
11-08 14:05:38.883 23263-23263/? A/DEBUG:     ip d5ef6fd0  sp ffadff68  lr d5ef40f1  pc d5ef405a  cpsr 200b0030
11-08 14:05:38.883 23263-23263/? A/DEBUG: backtrace:
11-08 14:05:38.883 23263-23263/? A/DEBUG:     #00 pc 0000105a  /data/app/com.crash.test-S56RBWKtbdRhEOOO60he2A==/lib/arm/libcrash.so (willCrash+13)
11-08 14:05:38.883 23263-23263/? A/DEBUG:     #01 pc 000010ed  /data/app/com.crash.test-S56RBWKtbdRhEOOO60he2A==/lib/arm/libcrash.so (Java_com_crash_test_HelloJni_nativeMsg+80)
11-08 14:05:38.883 23263-23263/? A/DEBUG:     #02 pc 00002063  /data/app/com.crash.test-S56RBWKtbdRhEOOO60he2A==/oat/arm/base.odex (offset 0x2000)
11-08 14:05:39.198 873-873/? E//system/bin/tombstoned: Tombstone written to: /data/tombstones/tombstone_07
11-08 14:05:39.217 799-799/? E/lowmemorykiller: Error writing /proc/23213/oom_score_adj; errno=22
11-08 14:05:39.235 1151-1276/? E/InputDispatcher: channel '10cde5b com.crash.test/com.crash.test.HelloJni (server)' ~ Channel is unrecoverably broken and will be disposed!
11-08 14:05:39.324 2002-2011/? E/ANDR-PERF-RESOURCEQS: Failed to apply optimization [2, 0]
11-08 14:05:39.329 763-23272/? E/ACDB-LOADER: Error: ACDB AudProc vol returned = -19


** 方式1：使用arm-linux-androideabi-addr2line  定位出错位置
以arm架构的CPU为例，执行如下命令：
/program/ndk/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-addr2line -e /mywork/github/androidreverse/crash/build/intermediates/cmake/debug/obj/armeabi-v7a/libcrash.so 0000105a 0000130b
 
-e：指定so文件路
0000105a 0000130b：出错的汇编指令地址

结果如下：
/program/ndk/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-addr2line -e /mywork/github/androidreverse/crash/build/intermediates/cmake/debug/obj/armeabi-v7a/libcrash.so 0000105a 0000130b
/mywork/github/androidreverse/crash/src/main/cpp/hellojni.c:128
/mywork/github/androidreverse/crash/src/main/cpp/hellojni.c:165

** 方法2： 
/program/ndk/toolchains/arm-linux-androideabi-4.9/prebuilt/darwin-x86_64/bin/arm-linux-androideabi-objdump -S -D /mywork/github/androidreverse/crash/build/intermediates/cmake/debug/obj/armeabi-v7a/libcrash.so > ./dump.log  


在生成的asm文件中，找出我们开始定位到的那两个出错的汇编指令地址, 在文件中搜索105a

void willCrash()
{
    104c:	b580      	push	{r7, lr}
    104e:	466f      	mov	r7, sp
    1050:	b086      	sub	sp, #24
    1052:	2000      	movs	r0, #0
    int *p = NULL;
    1054:	9005      	str	r0, [sp, #20]
    *p = 10;
    1056:	9805      	ldr	r0, [sp, #20]
    1058:	210a      	movs	r1, #10
    105a:	6001      	str	r1, [r0, #0]
    LOGE("Fun : %s, Line : %d", __FUNCTION__, __LINE__ );
    105c:	4668      	mov	r0, sp
    105e:	2181      	movs	r1, #129	; 0x81
    1060:	6001      	str	r1, [r0, #0]
    1062:	480b      	ldr	r0, [pc, #44]	; (1090 <willCrash+0x44>)
    1064:	4478      	add	r0, pc
    1066:	490b      	ldr	r1, [pc, #44]	; (1094 <willCrash+0x48>)
    1068:	4479      	add	r1, pc
    106a:	4a0b      	ldr	r2, [pc, #44]	; (1098 <willCrash+0x4c>)
    106c:	447a      	add	r2, pc
    106e:	2306      	movs	r3, #6
    1070:	9004      	str	r0, [sp, #16]
    1072:	4618      	mov	r0, r3
    1074:	9b04      	ldr	r3, [sp, #16]
    1076:	9103      	str	r1, [sp, #12]
    1078:	4619      	mov	r1, r3
    107a:	f8dd c00c 	ldr.w	ip, [sp, #12]
    107e:	9202      	str	r2, [sp, #8]
    1080:	4662      	mov	r2, ip
    1082:	9b02      	ldr	r3, [sp, #8]
    1084:	f7ff ec5a 	blx	93c <__android_log_print@plt>
}

** 方式3
 adb logcat | ndk-stack -sym /mywork/github/androidreverse/crash/build/intermediates/cmake/debug/obj/armeabi-v7a
 
当程序发生crash时，会输出如下信息
adb logcat | ndk-stack -sym /mywork/github/androidreverse/crash/build/intermediates/cmake/debug/obj/armeabi-v7a
 ********** Crash dump: **********
 Build fingerprint: 'google/walleye/walleye:8.1.0/OPM1.171019.021/4565141:user/release-keys'
 pid: 9816, tid: 9816, name: com.crash.test  >>> com.crash.test <<<
 signal 11 (SIGSEGV), code 1 (SEGV_MAPERR), fault addr 0x0
 Stack frame #00 pc 0000105a  /data/app/com.crash.test-PeLkKWs4KTzIW-hM2zoEig==/lib/arm/libcrash.so (willCrash+13): Routine willCrash at /mywork/github/androidreverse/crash/src/main/cpp/hellojni.c:128
 Stack frame #01 pc 0000130b  /data/app/com.crash.test-PeLkKWs4KTzIW-hM2zoEig==/lib/arm/libcrash.so (Java_com_crash_test_HelloJni_stringFromJNI+54): Routine Java_com_crash_test_HelloJni_stringFromJNI at /mywork/github/androidreverse/crash/src/main/cpp/hellojni.c:165
 Stack frame #02 pc 000020f3  /data/app/com.crash.test-PeLkKWs4KTzIW-hM2zoEig==/oat/arm/base.odex (offset 0x2000)
 Crash dump is completed

 
 先获取日志再分析：
 adb logcat > crash.log  
 ndk-stack -sym /mywork/github/androidreverse/crash/build/intermediates/cmake/debug/obj/armeabi-v7a -dump crash.log  
 
** 方法4
直接使用IDA，跳转到对应的地址即可
