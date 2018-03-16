//Demo

#include <stdio.h>
#include <unistd.h>

int count = 0;
 
void sevenWeapons(int number)
{
    char* str = "Hello, inject!";
    printf("%s %d, pid=%d\n",str,number, getpid());
}

int main(int argc, char* argv[])
{
    while(1)
    {
        sevenWeapons(count);
        count++;
        sleep(1);
    }

    return 0;
}