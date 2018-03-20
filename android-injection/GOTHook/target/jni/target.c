#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <dlfcn.h>
//void show_msg()
//{
//	char str1[] = "huluwa";
//	char str2[] = "shejing";
//	if (strlen(str1) > 3)
//	{
//		printf("str1=\"%s\" 's length > 3 \n", str1);
//	} else
//	{
//		printf("str1=\"%s\" 's length <=3 \n", str2);
//	}
//
//	if (strcmp(str1, str2) == 0)
//	{
//		printf("str1=\"%s\" is equal to str2=\"%s\" \n\n", str1, str2);
//	} else
//	{
//		printf("str1=\"%s\" is not equal to str2=\"%s\" \n\n", str1, str2);
//	}
//}

typedef void (*ShowMsg)();

int main(int argc, char** argv)
{
	void *handle = dlopen("/data/local/tmp/libtarget.so", RTLD_NOW);
	if (!handle)
	{
		fprintf(stderr, "dlopen error:%s\n", dlerror());
		return -1;
	}
	dlerror();

	void * sym = dlsym(handle, "show_msg");
	if (sym == NULL)
	{
		fprintf(stderr, "dlsym error:%s\n", dlerror());
		return -1;
	}
	ShowMsg show_msg = (ShowMsg)(sym);
	while (1)
	{
		show_msg();
		sleep(10);
	}
	return 0;
}
