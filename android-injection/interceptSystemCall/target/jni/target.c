#include <stdio.h>

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
