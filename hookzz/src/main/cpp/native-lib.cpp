#include <jni.h>
#include <android/log.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <linux/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <string>

// log标签
#define  TAG    "hello_load"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#include "hookzz.h"

#define TEST_printf 1
#define TEST_send 1
#define TEST_sendto 1
#define TEST_connect 1
#define TEST_freeaddrinfo 1
#define TEST_getaddrinfo 0
#define TEST_case_001 0


void common_pre_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    LOGI("<<< common_pre_call");
}

void common_post_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    LOGI("<<< common_post_call");
}
#if TEST_case_001

void getaddrinfo_pre_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    LOGI(">>> getaddrinfo_pre_call");
}

void send_pre_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    LOGI(">>> send_pre_call!");
    ZzHookPrePost((void *)getaddrinfo, getaddrinfo_pre_call, common_post_call);
}

__attribute__((constructor)) void hook_send() {
    ZzEnableDebugMode();
    ZzHookPrePost((void *)send, send_pre_call, common_post_call);
    send(-1, "test", 4, 0);

    struct addrinfo *ai, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = (AI_V4MAPPED | AI_ADDRCONFIG);
    hints.ai_family = AF_UNSPEC;
    int error = getaddrinfo( "46.23.43.12", "80", &hints, &ai);

}
#endif


#if TEST_getaddrinfo
int (*orig_getaddrinfo)(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res);
int fake_getaddrinfo(const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res) {
    LOGI("### fake start");
    int t= orig_getaddrinfo("test", service, hints, res);
    LOGI("### fake end");
    return t;
}
__attribute__((constructor)) void hook_getaddrinfo() {
    void *getaddrinfo_addr = (void *)getaddrinfo;
    ZzHookReplace((void *) getaddrinfo_addr, (void *) fake_getaddrinfo, (void **) &orig_getaddrinfo);
    // ZzHook((void *) getaddrinfo_addr, (void *) fake_getaddrinfo, (void **) &orig_getaddrinfo, common_pre_call, common_post_call, true);
    struct addrinfo *ai, hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = (AI_V4MAPPED | AI_ADDRCONFIG);
    hints.ai_family = AF_UNSPEC;
    int error = getaddrinfo( "46.23.43.12", "80", &hints, &ai);
}
#endif

#if TEST_printf
int (*orig_printf)(const char * format, ...);
int fake_printf(const char * format, ...) {
    puts("call printf");

    char *stack[16];
    void *args_stack;
    va_list args;
    va_start(args, format);
    args_stack = *(void **)&args;
    memcpy(stack, args_stack, sizeof(char *) * 16);
    va_end(args);

    // how to hook variadic function? fake a original copy stack.
    // [move to
    // detail-1](http://jmpews.github.io/2017/08/29/pwn/%E7%9F%AD%E5%87%BD%E6%95%B0%E5%92%8C%E4%B8%8D%E5%AE%9A%E5%8F%82%E6%95%B0%E7%9A%84hook/)
    // [move to detail-2](https://github.com/jmpews/HookZzModules/tree/master/AntiDebugBypass)
    int x = orig_printf(format, stack[0], stack[1], stack[2], stack[3], stack[4], stack[5],
                        stack[6], stack[7], stack[8], stack[9], stack[10], stack[11], stack[12],
                        stack[13], stack[14], stack[15]);
    return x;
}

void printf_post_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    if (STACK_CHECK_KEY(cs, "format")) {
        char *format = STACK_GET(cs, "format", char *);
        puts(format);
    }
    puts("<<< printf-post-call");
}

#if defined(__arm64__) || defined(__aarch64__)
void printf_pre_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    puts((char *)rs->general.regs.x0);
    STACK_SET(cs, "format", rs->general.regs.x0, char *);
    puts(">>> printf-pre-call");
}
#else
void printf_pre_call(RegState *rs, ThreadStack *ts, CallStack *cs, const HookEntryInfo *info) {
    puts((char *)rs->general.regs.r0);
    STACK_SET(cs, "format", rs->general.regs.r0, char *);
    puts(">>> printf-pre-call");
}
#endif
__attribute__((constructor)) void test_hook_printf() {
    void *printf_ptr = (void *)printf;

//    if(freopen("/data/data/com.zz.hooktester/test.output","w",stdout) == NULL)
//        __android_log_print(ANDROID_LOG_DEBUG, "HOOK_TESTER", "%s", "open stdout file error!");

    ZzEnableDebugMode();
    ZzHook((void *)printf_ptr, (void *)fake_printf, (void **)&orig_printf, printf_pre_call,
           printf_post_call, false);

//    char data[] = "hello.world";
//    FILE *fd = fopen("/data/local/tmp/test.output", "wb+");
//    int errorno = errno;
//    fwrite(data, strlen(data), 1, fd);

    printf("HookZzzzzzz, %d, %p, %d, %d, %d, %d, %d, %d, %d\n", 1, (void *)2, 3, (char)4, (char)5,
           (char)6, 7, 8, 9);
//    fclose(stdout);
}
#endif

#if TEST_send
ssize_t (*orig_send)(int, const void *, size_t, int);
ssize_t fake_send (int __fd, const void *__buf, size_t __n, int __flags) {
    LOGI("### called fake_send");
    ssize_t x = orig_send(__fd, __buf, __n, __flags);
    return x;
}
__attribute__((constructor)) void hook_send() {
    ZzEnableDebugMode();
    ZzHook((void *)send, (void *)fake_send, (void **)&orig_send, common_pre_call, common_post_call, true);
    send(-1, "test", 4, 0);
}
#endif

#if TEST_sendto
#include <strings.h>
ssize_t (*orig_sendto)(int socket, const void *buffer, size_t length, int flags,
                       const struct sockaddr *dest_addr, socklen_t dest_len);
ssize_t fake_sendto (int __socket, const void *__buffer, size_t __length, int __flags, const struct sockaddr *__dest_addr, socklen_t __dest_len) {
    LOGI("### called fake_sendto");
    ssize_t x = orig_sendto(__socket, __buffer, __length, __flags, __dest_addr, __dest_len);
    return x;
}
__attribute__((constructor)) void hook_sendto() {
    struct sockaddr_in addr;
    int sockfd, len = 0;
    int addr_len = sizeof(struct sockaddr_in);
    char buffer[256];
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror ("soshcket");
    }
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(3306);
    addr.sin_addr.s_addr = htonl(INADDR_ANY) ;

    ZzEnableDebugMode();
    // ZzHook((void *)sendto, NULL, NULL, precall, postcall, TRUE);

    ZzHook((void *)sendto, (void *)fake_sendto, (void **)&orig_sendto, common_pre_call, common_post_call, false);
    sendto(-1, "test", 4, 0, (struct sockaddr *)&addr, addr_len);
}
#endif

#if TEST_connect
int (*orig_connect)(int, const struct sockaddr*, socklen_t);
int fake_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    LOGI("### called fake_connect");

    struct sockaddr_in *sin = (struct sockaddr_in *) (addr);
    char* host = inet_ntoa(sin->sin_addr);
    unsigned short port = ntohs(sin->sin_port);
    LOGI("connect %s:%d", host, port);

    int x = orig_connect(sockfd, addr, addrlen);
    return x;
}
__attribute__((constructor)) void test_hook_connect() {
    ZzEnableDebugMode();
    ZzHook((void *)connect, (void *)fake_connect, (void **)&orig_connect, common_pre_call, common_post_call, false);
    LOGI("test_hook_connect:%p", orig_connect);
    int socket_desc;
    struct sockaddr_in server;
    //Create socket
    socket_desc = socket(AF_INET , SOCK_STREAM , 0);
    if (socket_desc == -1)
    {
        printf("Could not   socket");
    }
    server.sin_addr.s_addr = inet_addr("74.125.235.20");
    server.sin_family = AF_INET;
    server.sin_port = htons( 80 );
    //Connect to remote server
    if (connect(socket_desc , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        puts("connect error");
    }
}
#endif

#if TEST_freeaddrinfo
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <err.h>
void (*orig_freeaddrinfo)(struct addrinfo *ai);
void fake_freeaddrinfo(struct addrinfo *ai) {
    LOGI("### called fake_freeaddrinfo");
    orig_freeaddrinfo(ai);
}

__attribute__((constructor)) void test_hook_freeaddrinfo() {
    ZzEnableDebugMode();
    ZzHook((void *)freeaddrinfo, (void *)fake_freeaddrinfo, (void **)&orig_freeaddrinfo, NULL, NULL, true);

    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP address

    if ((rv = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("socket");
            continue;
        }

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("bind");
            continue;
        }

        break; // if we get here, we must have connected successfully
    }

    if (p == NULL) {
        // looped off the end of the list with no successful bind
        fprintf(stderr, "failed to bind socket\n");
        exit(2);
    }
    freeaddrinfo(servinfo); // all done with this structure
}


#endif


extern "C" JNIEXPORT jstring JNICALL
Java_com_hook_project_HelloJni_stringFromJNITest(
        JNIEnv *env,
        jobject /* this */)
{
    std::string hello = "Hello from C++";
    test_hook_printf();
    return env->NewStringUTF(hello.c_str());
}

