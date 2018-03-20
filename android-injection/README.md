# Android Injection
This project contains several exercises about injection using ptrace on Android platform (I really appreciate those who share knowledge about Android injection on their blogs).
First, let's have a quick look at each exercise.
- [exercise1-interceptSystemCall](#exercise1-interceptsystemcall)

  Exercise1 implements intercepting system call--write.
- [exercise2-injectMethod](#exercise2-injectmethod)

  Exercise2 is about method injection. It modifies a method's instructions to make the method return directly without execution.
- [exercise3-GOTHook](#exercise3-gothook)

  Exercise3 achieves GOT hook.

## Dependency
1. ndk
2. Android5.1 Emulator(32bit-arm)
3. python2.7

## exercise1-interceptSystemCall
**Goal:** Intercept system call--write(fd,str,size) and reverse its second parameter "str".

- Program Explanation


1. exercise1-interceptSystemCall/target is the injected program which calls printf() in a dead cycle. Its source code is as follows. If injected successfully, the output string will be reversed.

  ``` c
  int main()
  {
  	int count = 0;
  	while (1)
  	{
  		printf("Target is running:%d\n", count);
  		count++;
  		sleep(10);
  	}
  	return 0;
  }
  ```
* exercise1-interceptSystemCall/interceptSysCall is used to inject into target process to intercept system call write() and reverse its second parameter.


- Compilation


1. Open a terminal in the directory "exercise1-interceptSystemCall/target/jni" and run the following commands:

  ```c
  ndk-build  // compile
  python push.py  //push target to /data/local/tmp
  ```
* Open a terminal in the directory "exercise1-interceptSystemCall/interceptSysCall/jni" and run the following commands:
  ```c
  ndk-build //compile
  python push.py  //push interceptSysCall to /data/local/tmp
  ```


* Run


1. Run target process:
  ```c
  adb shell ./data/local/tmp/target
  ```
  The result is as follows:

  ![e1_r_before_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/1/1.png "e1_r1_before_injection")
* Run command "adb shell ps" to find the pid of target process.
* Run interceptSysCall process:
  ```c
  adb shell ./data/local/tmp/interceptSysCall 3940 //3940 is the pid of target process
  ```
  The output string of target process is reversed after running interceptSysCall process. The result figure below indicates injection succeeds.

  ![e1_r_after_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/1/4.png "e1_r1_after_injection")

## exercise2-injectMethod
**Goal:** Modify the instructions of hacked_method() defined in libtarget.so to make it return directly without execution. The definition of hacked_method() is as follows.
```c
int hacked_method(int p)
{
	p=p*2;
	p=p+3;
	p=p*3;
	p=p-5;
	return p%100;
}
```
- Program Explanation


1. exercise2-injectMethod/target is the injected program which loads libtarget.so, calls hacked_method() in a dead cycle and outputs the return value of hacked_method().
* exercise2-injectMethod/injectTarget is used to inject into target process and modify the return value of hacked_method().
* Find the injection address

  Open libtarget.so in IDA and find hacked_method(). From the figure below, we can learn that the offset of this method is 0xCF8 and the type of its instructions is thumb.

  ![e2_ida](https://github.com/ManyFace/AndroidInjection/blob/master/images/2/4.png "e2_ida")

  So we can override two instructions whose addresses are 0xCF8 and 0xCFA and the new instructions are as follows.
  ```c
  mov r0, #2  //change the return value to 2
  mov pc, lr  //return
  ```
  The binary of new instructions is "\x02\x20\xF7\x46"(Strictly speaking, we should comfirm the type of instruction located at lr and then modify the value of cpsr and pc registers).
  The base address of libtarget.so in memory can be obtained from proc file "/proc/pid/maps" and then the injection address is (base address + offset). If injected successfully, the return value of hacked_method() will always be 2.


- Compilation


1. Open a terminal in the directory "exercise2-injectMethod/target/jni" and run the following commands:

  ```c
  ndk-build  // compile
  python push.py  //push target to /data/local/tmp
  ```
* Open a terminal in the directory "exercise2-injectMethod/injectTarget/jni" and run the following commands:
  ```c
  ndk-build //compile
  python push.py  //push injectTarget to /data/local/tmp
  ```


* Run


1. Run target process:
  ```c
  adb shell ./data/local/tmp/target
  ```
  The result is as follows:

  ![e2_r_before_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/2/1.PNG "e2_r_before_injection")
* Run command "adb shell ps" to find the pid of target process.
* Run injectTarget process:
  ```c
  adb shell ./data/local/tmp/injectTarget 8326 //8326 is the pid of target process
  ```
  The output of target process is changed after running injectTarget process. The result figure below indicates injection succeeds.

  ![e2_r_after_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/2/3.PNG "e2_r_after_injection")

## exercise3-GOTHook
**Goal:** Guhe had already given an example about injecting so before. His main idea is to use shellcode to load library in remote process. This exercise is based on his example but purely uses c language to accomplish injecting so instead of shellcode which is hard to be understood. Specially, this exercise implements GOT hook.
- Program Explanation


1. exercise3-GOTHook/target is the injected program which loads libtarget.so and call show_msg(), which is defined in libtarget.so, in a dead cycle. The definition of show_msg() is as follows.
  ```c
  void show_msg()
  {
    char str1[] = "huluwa";
    char str2[] = "shejing";
    if (strlen(str1) > 3)
    {
      printf("str1=\"%s\" 's length > 3 \n", str1);
    } else
    {
      printf("str1=\"%s\" 's length <=3 \n", str2);
    }

    if (strcmp(str1, str2) == 0)
    {
      printf("str1=\"%s\" is equal to str2=\"%s\" \n\n", str1, str2);
    } else
    {
      printf("str1=\"%s\" is not equal to str2=\"%s\" \n\n", str1, str2);
    }
  }
  ```
* exercise3-GOTHook/injectSo is used to inject libhook.so(exercise3-GOTHook/libhook) into target process and call hook_init() defined in libhook.so to gain entry addresses of strlen() and strcmp() in GOT, and then change relocation addresses to the addresses of hook functions.

  The corresponding hook function of strlen() is hook_strlen(char * str) which is defined as follows：
  ```c
  int hook_strlen(char * str)
  {
  	int ret = strlen(str);
  	return 1; //modify return value
  }
  ```
  The corresponding hook function of strcmp() is hook_strcmp(const char *str1, const char *str2) which is defined as follows：
  ```c
  int hook_strcmp(const char *str1, const char *str2)
  {
  	int ret = strcmp(str1, str2);
  	return 0; //modify return value
  }
  ```
* How to call a initial function in libhook.so

  After injecting libhook.so into target process, injectSo process can call dlsym() to get the address of initial function in libhook.so and then modify the registers to call initial function. Of course there exists an easier way. Define a constructor in libhook.so, such as "void before_main() \__attribute__((constructor))". From the figure below, we can learn that before_main() is actually an entry of .init array section, so before_main() will be called when libhook.so is loaded by calling dlopen(). Unfortunately, before_main() can't have parameters.

  ![e3_init_array](https://github.com/ManyFace/AndroidInjection/blob/master/images/3/1.PNG "e3_init_array")
* Processor State Switch

  Both BLX and BX instrucitons can exchange instruction set.
  - The BLX instruction copy the address of next instruction into lr register.
  - The BLX and BX instructions can change the processor state from ARM to Thumb, or from Thumb to ARM.
  - BLX label always changes the state.
  - BLX Rm and BX Rm derive the target state from bit[0] of Rm:
    - if bit[0] of Rm is 0, the processor changes to, or remains in, ARM state.
    - if bit[0] of Rm is 1, the processor changes to, or remains in, Thumb state. When changing to Thumb state, the instruction address must be 2-byte aligned.

  When processor changes state, the T flag (bit[5]) of cpsr register should be changed too. If target state is ARM, T flag should be reseted. If target state is Thumb, T flag should be seted.


- Compilation


1. Open a terminal in the directory "exercise3-GOTHook/target/jni" and run the following commands:

  ```c
  ndk-build  // compile
  python push.py  //push target to /data/local/tmp
  ```
* Open a terminal in the directory "exercise3-GOTHook/libhook/jni" and run the following commands:

  ```c
  ndk-build //compile
  python push.py  //push libhook.so to /data/local/tmp
  ```
* Open a terminal in the directory "exercise3-GOTHook/injectSo/jni" and run the following commands:
  ```c
  ndk-build //compile
  python push.py  //push injectSo to /data/local/tmp
  ```


* Run


1. Run target process:
  ```c
  adb shell ./data/local/tmp/target
  ```
  The result is as follows:

  ![e3_r_before_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/3/2.PNG "e3_r_before_injection")
* Run command "adb shell ps" to find the pid of target process.
* Run injectSo process:
  ```c
  adb shell ./data/local/tmp/injectSo 1883 //1883 is the pid of target process
  ```
  The output of target process is changed after running injectSo process. The result figure below indicates injection succeeds.

  ![e3_r_after_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/3/3.PNG "e3_r_after_injection")

## Reference
1. http://blog.csdn.net/zhangmiaoping23/article/details/17919611
* https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/
* http://blog.sina.com.cn/s/blog_88b60ea001017bc9.html
* http://blog.csdn.net/cos_sin_tan/article/details/7667582
* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204g/Cjaghefc.html
* http://blog.csdn.net/myarrow/article/details/9630377
* http://www.kanxue.com/bbs/showthread.php?t=141355&highlight=%E5%8F%A4%E6%B2%B3
* http://www.kanxue.com/bbs/showthread.php?t=207710
