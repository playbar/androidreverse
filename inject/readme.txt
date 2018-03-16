1)注入进程

　　1.编程思路分为以下几个步骤

　　①.每个进程都在/proc目录下，以进程id为文件夹名，所以可以通过/proc/<pid>/cmdline文件中中读取进程名称，和我们需要注入的进程名称比较，获得进程id

　　②.以root身份运行注入程序，通过ptrace函数，传入PTRACE_ATIACH附加到目标进程，PTRACE_SETREGS设置进程寄存器，PTRACE_GETREGS获得目标寄存器.更多可以访问ptrace的使用

　　③.调用mmap在对方进程空间分配内存，保存要加载的so文件路径，so中函数的名称，so中函数需要传入的参数。

　　　由于每个模块在进程中加载地址不一致，所以我们首先获得目标进程中libc.so文件基址TargetBase，再获得自身libc.so基址SelfBase，再根据mmap-SelfBase+TargetBase获得目标进程中mmap的地址。

        同理获得目标进程中dlopen()函数地址、dlsym()函数地址、dlclose()函数地址

　　④.调用dlopen()函数加载so库，调用dlsym()函数获得so库中函数的地址，调用so库中函数的地址，测试注入成功！调用dlclose()函数卸载so库。