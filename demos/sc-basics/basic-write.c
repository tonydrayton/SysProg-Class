#include <unistd.h>

int main(){
    write(STDOUT_FILENO, "Hello there\n", 12);
}