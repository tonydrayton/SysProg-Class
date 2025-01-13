#include <stdio.h>

int main() {
    printf("The size of a char is: %zu bytes\n", sizeof(char));
    printf("The size of an int is: %zu bytes\n", sizeof(int));
    printf("The size of a long is: %zu bytes\n", sizeof(long));
    printf("The size of an unsigned int is: %zu bytes\n", sizeof(unsigned int));
    return 0;
}