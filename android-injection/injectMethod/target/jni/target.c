#include <stdio.h>
#include <dlfcn.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <dlfcn.h>

//int hacked_method(int p)
//{
//	p=p*2;
//	p=p+3;
//	p=p*3;
//	p=p-5;
//	return p%100;
//}

typedef int (*HackedMethod)(int);

int main()
{

	void *handle = dlopen("/data/local/tmp/libtarget.so", RTLD_NOW);
	if(!handle)
	{
		fprintf(stderr,"dlopen error:%s\n",dlerror());
		return -1;
	}
	dlerror();

	void * sym = dlsym(handle,"hacked_method");
	if (sym==NULL)
	{
		fprintf(stderr,"dlsym error:%s\n",dlerror());
		return -1;
	}
	HackedMethod hacked_method=(HackedMethod)(sym);

	int count=1;
	while (1)
	{
		printf("hacked_method() return:%d.\n\n", (*hacked_method)(count));
		count++;
		sleep(5);
	}
	return 0;
}
