#include <stdio.h>

//gcc -g -fno-pie -no-pie -o printfx2 printfx2.c   
int main(){
    int i;

    i = 1;
    printf("Calling printf - time %d\n",i);
    i++;
    printf("Calling printf - time %d\n",i);
}