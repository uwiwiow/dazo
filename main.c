/* -------------------- INCLUDES -------------------- */
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

/* -------------------- DEFINES -------------------- */
#define info(FORMAT, ...)                                                                                       \
    fprintf(stdout, "%s -> %s():%i \r\n\t" FORMAT "\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__);\

#define error(ERROR, FORMAT, ...)                                                           \
    if (ERROR) {                                                                            \
        fprintf(stderr, "\033[1;31m%s -> %s():%i -> Error(%i):\r\n\t%s\r\n\t" FORMAT "\r\n",\
        __FILE_NAME__, __FUNCTION__, __LINE__, errno, strerror(errno), ##__VA_ARGS__);      \
        exit(EXIT_FAILURE);                                                                 \
    }

/* -------------------- DATA -------------------- */
struct termios orig_termios;

/* -------------------- TERMINAL -------------------- */
void disableRawMode();

void enableRawMode();

/* -------------------- INIT -------------------- */
int main(){
    enableRawMode();

    while (1) {
        char c;
        error(read(STDIN_FILENO, &c, 1) == -1, "read %d ('%c')", c, c)

        if (iscntrl(c)) {
            printf("%d\r\n", c);
        } else {
            printf("%d ('%c')\r\n", c, c);
        }
        if (c == 'q') break;
    }
    return 0;
}

/* -------------------- FUNCTIONS -------------------- */
void disableRawMode() {
    info("tcsetattr c_lflag: %u", orig_termios.c_lflag)
    error(tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios) == -1,
          "tcsetattr c_lflag: %u", orig_termios.c_lflag)
}

void enableRawMode() {
    error(tcgetattr(STDIN_FILENO, &orig_termios) == -1,
          "tcgetattr c_lflag: %u", orig_termios.c_lflag)
    atexit(disableRawMode);

    struct termios raw = orig_termios;
    info("tcgetattr c_lflag: %u", raw.c_lflag)

    cfmakeraw(&raw);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 1;

    info("tcsetattr c_lflag: %u", raw.c_lflag)
    error(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1,
          "tcsetattr c_lflag: %u", raw.c_lflag)
}