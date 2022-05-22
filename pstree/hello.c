#include <stdio.h>

int main(void){
    const char *name_str = "Name:";
    printf("size of name_str is %ld\n", sizeof(*name_str));
    const char buf[512];
    printf("size of buf is %ld\n", sizeof(buf));
}