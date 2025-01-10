typedef unsigned long ptr_reg_t;

//compile with gcc -g -o sc-basics ./sc-basics.c
#define STD_OUT     1

//functions showing what application code would do
void some_application_code(char *);

//functions showing what systems code would do
unsigned int systems_code_strlen(char *);
void systems_code_str_to_upper(char *);
long systems_code_print_str(char *);
long do_write_syscall(int, char *, int);

//functions that are processor specific
long do_real_syscall(long, long, long, long, long, long, long);

#ifdef __aarch64__
    //> ausyscall aarch64 --dump | grep write
    #define WRITE_SYSCALL_NUM 64  
#endif

#ifdef __x86_64__ 
    //> ausyscall aarch64 --dump | grep write
    #define WRITE_SYSCALL_NUM 64
#endif

long test_sc();

int main(){
    char msg[] = "Hi there Class!\n";
    some_application_code(msg);
}


void some_application_code(char *msg){
    //we will do some demo application code just to demonstrate
    //the process

    //lets use some systems code to convert the message to upper
    //case
    systems_code_str_to_upper(msg);

    //now lets use some systems code to print the message
    systems_code_print_str(msg);
}

unsigned int systems_code_strlen(char *str){
    int len = 0;

    while (*str != '\0'){
        len++;
        str++;
    }
    return len;
}

void systems_code_str_to_upper(char *str) {
    char *ptr = str;
    if (!ptr) return;
    
    while (*ptr) {
        if (*ptr >= 'a' && *ptr <= 'z') {
            *ptr = *ptr - 32;
        }
        ptr++;
    }
}

long systems_code_print_str(char *str){
    int len = systems_code_strlen(str);

    long rc = do_write_syscall(STD_OUT, str, len);

    return rc;
}

long do_write_syscall(int fd, char * buff, int len){
    return  do_real_syscall(WRITE_SYSCALL_NUM, 
                     (long)fd, 
                     (ptr_reg_t)buff,  // Properly cast to uintptr_t
                     (long)len,
                     0, 0, 0);
}



#ifdef __aarch64__
long do_real_syscall(long number, long a1, long a2, long a3, 
             long a4, long a5, long a6){
    register long syscall_num __asm__("x8") = number;
    register long arg0 __asm__("x0") = a1;
    register long arg1 __asm__("x1") = a2;
    register long arg2 __asm__("x2") = a3;
    register long arg3 __asm__("x3") = a4;
    register long arg4 __asm__("x4") = a5;
    register long arg5 __asm__("x5") = a6;

    __asm__ volatile (
        "svc 0x0        \n\t"   //execute syscall to OS
        : "+r" (arg0)
        : "r" (syscall_num), "r" (arg1), "r" (arg2), 
        "r" (arg3), "r" (arg4), "r" (arg5)
        : "memory"
    );

    return arg0;
}
#endif

#ifdef __x86_64__
long do_real_syscall(long number, long a1, long a2, long a3, 
                long a4, long a5, long a6){
    register long syscall_num __asm__("rax") = number;
    register long arg0 __asm__("rdi") = a1;
    register long arg1 __asm__("rsi") = a2;
    register long arg2 __asm__("rdx") = a3;
    register long arg3 __asm__("r10") = a4;
    register long arg4 __asm__("r8")  = a5;
    register long arg5 __asm__("r9")  = a6;

    __asm__ volatile (
        "syscall          \n\t"   // Invoke the syscall
        : "+r" (arg0)              // Output: return value is in rdi
        : "r" (syscall_num), "r" (arg1), "r" (arg2), 
          "r" (arg3), "r" (arg4), "r" (arg5) // Inputs: syscall number and arguments
        : "memory"                 // Clobber memory
    );

    return arg0;
}
#endif