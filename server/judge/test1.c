#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(void) {

    close(0);
    int num1,num2;
    int fd = open("./temp/inject.txt", O_RDONLY);
    scanf("%d %d", &num1, &num2);
    printf("sum = %d\n",num1 + num2);
}

