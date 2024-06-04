#include"kernel.h"
#include<stdio.h>

void panic(const char* s){
    printf("Kernel panic due to %s\n",s);
    for(;;);
}