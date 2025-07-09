#include <stdio.h>
#include <string.h>
#include <threads.h>

#define CLEAR   "\033[2j"
#define HOME    "\033[H"
#define HIDE    "\033[?25l"
#define SHOW    "\033[?25h"
#define NEXT    "\033[1E"
#define SET     "\0337"
#define RET     "\0338"

// given any input, e.g. a string, decimal, hex, binary, display it as all said representations 
int main(int argc, char *argv[]){ (void)argc; (void)argv;
    int count = 0;
    struct timespec time;
    time.tv_nsec = 1000;
    printf(CLEAR);
    printf(SET);
    for(;;){
        printf(RET);
        printf("\033[35m");

        printf("λ count: %i\n", count);
        printf("[!] warning\n");
        printf("[+] Access\n");

        printf("\033[0m");
        thrd_sleep(&time, NULL);
        count++;
    }
    printf("\n");
    fflush(stdout);
    return 0;
}
