# Android Injection
Android Injection是关于Android平台上ptrace注入的一些小实践(主要参考了各位大神的博客，在此表示感谢！)。下面对各个练习进行概述。
- [exercise1-interceptSystemCall](#exercise1-interceptsystemcall)

  该练习实现了对系统调用write的拦截，比较适合初学者入门。
- [exercise2-injectMethod](#exercise2-injectmethod)

  该练习实现了对方法的注入，修改了指定方法的指令，让函数直接返回。
- [exercise3-GOTHook](#exercise3-gothook)

  该练习实现了GOT hook。

## 环境
1. 采用的ndk版本为ndk-r10d，编译前需要配置ndk环境变量
2. 程序均在Android 5.1 32bit模拟器上运行
3. python2.7

## exercise1-interceptSystemCall
**目标：**拦截系统调用write(fd,str,size)，并反转参数str。

- 相关程序说明


1. exercise1-interceptSystemCall/target是被注入的程序，该程序很简单，就是循环调用printf，如下所示。如果注入成功，那么printf输出的字符串就会反转。

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
* exercise1-interceptSystemCall/interceptSysCall是注入程序，用于拦截目标程序的系统调用write，并反转参数str。


- 编译


1. 在exercise1-interceptSystemCall/target/jni目录下打开终端，输入命令：

  ```c
  ndk-build  // 编译
  python push.py  //将target push 到/data/local/tmp目录下
  ```
* 在exercise1-interceptSystemCall/interceptSysCall/jni目录下打开终端，输入命令：
  ```c
  ndk-build //编译
  python push.py  //将interceptSysCall push 到/data/local/tmp目录下
  ```


* 运行


1. 运行target：在任意终端下输入如下命令：
  ```c
  adb shell ./data/local/tmp/target
  ```
  得到如下结果:

  ![e1_r_before_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/1/1.png "e1_r1_before_injection")
* 通过adb shell ps命令找到target的pid，假设为3940
* 运行interceptSysCall：在任意终端下输入如下命令：
  ```c
  adb shell ./data/local/tmp/interceptSysCall 3940 //3940为target的pid
  ```
  此时可以看到target的输出已经变化，如下所示，说明注入成功。

  ![e1_r_after_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/1/4.png "e1_r1_after_injection")

## exercise2-injectMethod
**目标：**修改libtarget.so中hacked_method()函数的指令，使hacked_method()直接返回。hacked_method()的定义如下：
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
- 相关程序说明


1. exercise2-injectMethod/target是被注入的程序，在该程序中加载libtarget.so，并循环调用hacked_method()，将hacked_method()的返回值输出。
* exercise2-injectMethod/injectTarget是注入程序，用于修改hacked_method()的返回值。
* 注入点的确定

  在IDA中查看hacked_method()在libtarget.so中的偏移offset=0xCF8，且为thumb指令，如下图所示：

  ![e2_ida](https://github.com/ManyFace/AndroidInjection/blob/master/images/2/4.png "e2_ida")

  因而可以通过覆盖0xCF8和0xCFA处的指令，修改函数的返回值，并让函数返回。这里覆盖的指令为：
  ```c
  mov r0, #2  //设置函数的返回值为2
  mov pc, lr  //返回
  ```
  这两条指令的二进制为shell_code="\x02\x20\xF7\x46"，（其实更严谨一点应该首先判断lr处的指令是arm还是thumb，然后修改
  状态寄存器和pc）libtarget.so在内存中的基址base_addr可以从/proc/pid/maps中获取，从而计算得到注入shell_code的地址为
  base_addr+offset。如果注入成功，那么hacked_method()的返回值就恒为2。


- 编译


1. 在exercise2-injectMethod/target/jni目录下打开终端，输入命令：
  ```c
  ndk-build  // 编译
  python push.py  //将target push 到/data/local/tmp目录下
  ```
* 在exercise2-injectMethod/injectTarget/jni目录下打开终端，输入命令：
  ```c
  ndk-build //编译
  python push.py  //将injectTarget push 到/data/local/tmp目录下
  ```


* 运行


1. 运行target：在任意终端下输入如下命令：
  ```c
  adb shell ./data/local/tmp/target
  ```
  得到如下结果:

  ![e2_r_before_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/2/1.PNG "e2_r_before_injection")
* 通过adb shell ps命令找到target的pid，假设为8326
* 运行injectTarget：在任意终端下输入如下命令：
  ```c
  adb shell ./data/local/tmp/injectTarget 8326 //8326为target的pid
  ```
  此时可以看到target的输出已经变化，如下所示，说明注入成功。

  ![e2_r_after_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/2/3.PNG "e2_r_after_injection")

## exercise3-GOTHook
**目标：**古河大大很早之前发过注入so的例子，是通过注入shellcode来实现的。本练习将那段shellcode改为c语言来实现，以方便理解，并在此基础上添加了GOT Hook的功能。
- 相关程序说明


1. exercise3-GOTHook/target是被注入的程序，在该程序中加载libtarget.so，并循环调用libtarget.so中的show_msg()函数，show_msg()函数的定义如下：
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
* exercise3-GOTHook/injectSo是注入的程序，它将exercise3-GOTHook/libhook动态链接库注入到target中，并调用exercise3-GOTHook/libhook的hook_init()函数获取libtarget.so中strlen()和strcmp()函数在GOT中的地址，最后修改GOT，将地址改为hook函数的地址，从而实现GOT Hook。

  strlen()对应的hook函数为int hook_strlen(char * str)，其定义如下：
  ```c
  int hook_strlen(char * str)
  {
  	int ret = strlen(str);
  	return 1; //modify return value
  }
  ```
  strcmp()对应的hook函数为int hook_strcmp(const char *str1, const char *str2)，其定义如下：
  ```c
  int hook_strcmp(const char *str1, const char *str2)
  {
  	int ret = strcmp(str1, str2);
  	return 0; //modify return value
  }
  ```
* 如何执行注入so中的初始化函数

  在injectSo将so注入到target中后，可以通过dlsym()找到初始化函数的地址，然后设置相应的寄存器值来执行初始化函数。当然还有个相对简单的方法：在注入so中定义一个构造函数void before_main() \__attribute__((constructor))，通过IDA查看该so可以知道，before_main()函数是.init array section的一个元素，如下图所示。从而before_main()函数就会在用dlopen()加载该so的时候执行，但是before_main()不能有参数。

  ![e3_init_array](https://github.com/ManyFace/AndroidInjection/blob/master/images/3/1.PNG "e3_init_array")
* ARM状态切换

  ARM有两类跳转指令可以进行状态切换：BLX和BX。
  - BLX会将返回地址存在寄存器LR中。
  - BLX和BX会改变处理器的状态，从ARM转到Thumb状态或者从Thumb转到ARM状态。
  - BLX label总会改变处理器的状态。
  - BLX Rm和BX Rm会根据Rm的第0位bit[0]来确定如何转换状态：
    - 如果bit[0]是0，处理器将会转为或者保持ARM状态。
    - 如果bit[1]是1，处理器将会转为或者保持Thumb状态，注意进入Thumb状态时，地址是奇数，因而需要对齐，将地址最低位置0。

  当处理器转换状态时，需要修改状态寄存器cpsr(R16)的T标志位(第5位)，如果目标状态是ARM状态，T标志位置0；如果目标状态是Thumb状态，T标志位置1。


- 编译


1. 在exercise3-GOTHook/target/jni目录下打开终端，输入命令：

  ```c
  ndk-build  // 编译
  python push.py  //将target push 到/data/local/tmp目录下
  ```
* 在exercise3-GOTHook/libhook/jni目录下打开终端，输入命令：

  ```c
  ndk-build //编译
  python push.py  //将libhook.so push 到/data/local/tmp目录下
  ```
* 在exercise3-GOTHook/injectSo/jni目录下打开终端，输入命令：
  ```c
  ndk-build //编译
  python push.py  //将injectSo push 到/data/local/tmp目录下
  ```


* 运行


1. 运行target：在任意终端下输入如下命令：
  ```c
  adb shell ./data/local/tmp/target
  ```
  得到如下结果:

  ![e3_r_before_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/3/2.PNG "e3_r_before_injection")
* 通过adb shell ps命令找到target的pid，假设为1883
* 运行injectSo：在任意终端下输入如下命令：
  ```c
  adb shell ./data/local/tmp/injectSo 1883 //1883为target的pid
  ```
  此时可以看到target的输出已经变化，如下所示，说明注入成功。

  ![e3_r_after_injection](https://github.com/ManyFace/AndroidInjection/blob/master/images/3/3.PNG "e3_r_after_injection")

## 参考
1. http://blog.csdn.net/zhangmiaoping23/article/details/17919611
* https://mikecvet.wordpress.com/2010/08/14/ptrace-tutorial/
* http://blog.sina.com.cn/s/blog_88b60ea001017bc9.html
* http://blog.csdn.net/cos_sin_tan/article/details/7667582
* http://infocenter.arm.com/help/index.jsp?topic=/com.arm.doc.dui0204g/Cjaghefc.html
* http://blog.csdn.net/myarrow/article/details/9630377
* http://www.kanxue.com/bbs/showthread.php?t=141355&highlight=%E5%8F%A4%E6%B2%B3
* http://www.kanxue.com/bbs/showthread.php?t=207710
