#include "types.h"
#include "stat.h"
#include "user.h"

int a = 1;

int main(void)
{
    printf(1,"Parent and Child share the global variable a \n");

    int pid = fork();
    if(pid==0)
    {
        printf(1,"Child: a = %d\n",a);
        a = 2;
        printf(1,"Child: a = %d\n",a);
        exit();
    }
    printf(1,"Parent: a = %d\n",a);
    wait();
    printf(1,"Parent: a = %d\n",a);
    exit();
}