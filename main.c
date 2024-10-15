#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


    /** DEFINES */
#define info(FORMAT, ...)                                                                                              \
    fprintf(stdout, "%s -> %s():%i \r\n\t" FORMAT "\r\n", __FILE_NAME__, __FUNCTION__, __LINE__, ##__VA_ARGS__);       \

#define error(ERROR, FORMAT, ...)                                                                                      \
    if (ERROR) {                                                                                                       \
        write(STDOUT_FILENO, "\x1b[2J", 4);                                                                            \
        write(STDOUT_FILENO, "\x1b[H", 3);                                                                             \
        fprintf(stderr, "\033[1;31m%s -> %s():%i -> Error(%i):\r\n\t%s\r\n\t" FORMAT "\r\n",                           \
        __FILE_NAME__, __FUNCTION__, __LINE__, errno, strerror(errno), ##__VA_ARGS__);                                 \
        exit(EXIT_FAILURE);                                                                                            \
    }

#define CTRL(x)	(x&037)
// TODO add trace log


    /** DATA */
struct termios orig_termios;

typedef enum {
    LOG_ALL = 0,        // Display all logs
    LOG_TRACE,          // Trace logging, intended for internal use only
    LOG_DEBUG,          // Debug logging, used for internal debugging, it should be disabled on release builds
    LOG_INFO,           // Info logging, used for program execution info
    LOG_WARNING,        // Warning logging, used on recoverable failures
    LOG_ERROR,          // Error logging, used on unrecoverable failures
    LOG_FATAL,          // Fatal logging, used to abort program: exit(EXIT_FAILURE)
    LOG_NONE            // Disable logging
} TraceLogLevel;



    /** TERMINAL */
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

char editorReadKey() {
    ssize_t nread;
    char c;

    while ((nread = read(STDIN_FILENO, &c, 1)) != 1)
        error(nread == -1, "read %d ('%c')", c, c)

    return c;
}


    /** OUTPUT */
void editorDrawRows() {
    int y;
    for (y = 0; y < 24; y++) {
        write(STDOUT_FILENO, "~\r\n", 3);
    }
}

void editorRefreshScreen() {
    write(STDOUT_FILENO, "\x1b[2J", 4);
    write(STDOUT_FILENO, "\x1b[H", 3);

    editorDrawRows();

    write(STDOUT_FILENO, "\x1b[H", 3);
}


    /** INPUT */
void editorProcessReadKey() {
    char c = editorReadKey();

    switch (c) {
        case CTRL('q'): {}
            exit(EXIT_SUCCESS);
            break;
    }
}


    /** INIT */
int main(){
    enableRawMode();

    while (1) {
        editorRefreshScreen();
        editorProcessReadKey();
    }

    return 0;
}
