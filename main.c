#define _BSD_SOURCE
#define _DEFAULT_SOURCE

#include <threads.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <termios.h>

#define CLRD    "\033[0J"
#define CLRU    "\033[1J"
#define HOME    "\033[H"
#define HIDE    "\033[?25l"
#define SHOW    "\033[?25h"
#define NEXT    "\033[1E"
#define SET     "\0337"
#define RET     "\0338"

#define MAGENTA "\033[35m"
#define RESET   "\033[0m"

enum ui_state_t {
    none,
    calc,
    stream,
};

typedef struct types_t types_t;

struct types_t{
    void (*serial)( types_t *); 
    enum ui_state_t state;
    char decimal[256];
    char hex[256];
    char binary[256];
    int _fd;
    char port[256];
    char sbuf[256];
};

void parse(const char * buf, types_t *state){
    state->state = none;
    state->decimal[0] = state->hex[0] = state->binary[0] = '\0';

    if (!buf || strlen(buf) == 0)
        return;

    char *endptr = NULL;
    long value = 0;

    if((strcmp(buf, "ACM0") == 0) || (strcmp(buf, "USB0") == 0)){
        state->state = stream;
        // append buf to /dev/tty
        // then pass it to the function pointer
        snprintf(state->port, sizeof(state->port), "%s%s", "/dev/tty", buf);
        state->serial(state);
        return;
    }

    if (strncmp(buf, "0x", 2) == 0 || strncmp(buf, "0X", 2) == 0) {
        value = strtol(buf + 2, &endptr, 16);
    } else if (strncmp(buf, "0b", 2) == 0 || strncmp(buf, "0B", 2) == 0) {
        value = 0;
        const char *p = buf + 2;
        while (*p == '0' || *p == '1') {
            value = (value << 1) | (*p - '0');
            p++;
        }
        endptr = (char *)p;
    } else if (isdigit((unsigned char)buf[0]) || (buf[0] == '-' && isdigit((unsigned char)buf[1]))) {
        value = strtol(buf, &endptr, 10);
    } else {
        return;
    }

    if (*endptr != '\0') {
        return;
    }

    state->state = calc;

    snprintf(state->decimal, sizeof(state->decimal), "%ld", value);
    snprintf(state->hex, sizeof(state->hex), "0x%lX", value);

    char bin[256] = {0};
    int pos = 0;
    unsigned long uval = (unsigned long)value;

    if (value == 0) {
        strcpy(bin, "0b0");
    } else {
        bin[pos++] = '0';
        bin[pos++] = 'b';
        int started = 0;
        for (int i = sizeof(unsigned long) * 8 - 1; i >= 0; i--) {
            if (uval & (1UL << i)) {
                started = 1;
                bin[pos++] = '1';
            } else if (started) {
                bin[pos++] = '0';
            }
        }
        bin[pos] = '\0';
    }

    strncpy(state->binary, bin, sizeof(state->binary) - 1);

}

void oUI(types_t *state){
    printf("[+] Dec: %s \n", state->decimal);
    printf("[+] Hex: %s \n", state->hex);
    printf("[+] Bin: %s \n", state->binary);
}

void sUI(types_t * state){
    for(;;){
    int rAm = read(state->_fd, state->sbuf, sizeof(state->sbuf));
    write(STDOUT_FILENO, state->sbuf, rAm);
    }
}

void serial(types_t *state){
    state->_fd = open(state->port, O_RDWR | O_NOCTTY);
    if(state->_fd < 0)
        return;

    struct termios tty = {};
    if(tcgetattr(state->_fd, &tty) != 0){
        close(state->_fd);
        return;
    }

    cfmakeraw(&tty);
    speed_t speed = B115200;

    cfsetispeed(&tty, speed);
    cfsetospeed(&tty, speed);

    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 0;

    if(tcsetattr(state->_fd, TCSANOW, &tty) != 0){
        close(state->_fd);
        return;
    }

    tcflush(state->_fd, TCIFLUSH);
}

// TODO inline math operations
int main(int argc, char *argv[]){ (void)argc; (void)argv;
    char uiBuff[256];
    strcpy(uiBuff, "");
    types_t state;
    state.state = none;
    state.serial = serial;

    printf(CLRU);
    printf(HOME);
    printf(CLRD);
    printf(SET);

    for(;;){
        printf(RET);
        printf(MAGENTA);
        printf("[λ] :: %s\n", uiBuff);
        parse(uiBuff, &state);

        if(state.state == calc)
            oUI(&state);

        if(state.state == stream)
            sUI(&state);

        printf(RESET);

        (void)scanf("%s", uiBuff); 

        printf(CLRU);
    }

    printf("\n");
    fflush(stdout);
    return 0;
}

// what do we want to do ? 
// so the user has told us either ACM0 or USB0
// create the serial port, by passing the state itself to the function of state
//
