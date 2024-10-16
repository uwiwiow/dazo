#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/ioctl.h>


    /** DEFINES */
#define CTRL(x)	(x&037)


    /** DATA */
struct editorConfig {
    int screenrows;
    int screencols;
    struct termios orig_termios;
};

struct editorConfig E;


    /** TRACE LOG */
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
// TODO add trace log

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


    /** TERMINAL */
void disableRawMode() {
    info("tcsetattr c_lflag: %u", E.orig_termios.c_lflag)
    error(tcsetattr(STDIN_FILENO, TCSAFLUSH, &E.orig_termios) == -1,
          "tcsetattr c_lflag: %u", E.orig_termios.c_lflag)
}

void enableRawMode() {
    error(tcgetattr(STDIN_FILENO, &E.orig_termios) == -1,
          "tcgetattr c_lflag: %u", E.orig_termios.c_lflag)
    atexit(disableRawMode);

    struct termios raw = E.orig_termios;
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

        // return line if failed
int getCursorPosition(int *rows, int *cols) {
    char buf[32];
    unsigned int i = 0;
    if (write(STDOUT_FILENO, "\x1b[6n", 4) != 4) return __LINE__;
    while (i < sizeof(buf) - 1) {
        if (read(STDIN_FILENO, &buf[i], 1) != 1) break;
        if (buf[i] == 'R') break;
        i++;
    }
    buf[i] = '\0';
    if (buf[0] != '\x1b' || buf[1] != '[') return __LINE__;
    if (sscanf(&buf[2], "%d;%d", rows, cols) != 2) return __LINE__;
    return 0;
}

        // return line if failed
int getWindowSize(int *rows, int *cols) {
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1 || ws.ws_col == 0) {
        if (write(STDOUT_FILENO, "\x1b[999C\x1b[999B", 12) != 12) return __LINE__;
        return getCursorPosition(rows, cols);
    } else {
        *cols = ws.ws_col;
        *rows = ws.ws_row;
        return 0;
    }
}


    /** APPEND BUFFER */
struct abuf {
    char *b;
    int len;
};

#define ABUF_INIT {NULL, 0}

void abAppend(struct abuf *ab, const char *s, int len) {
    char *new = realloc(ab->b, ab->len + len); // p gets more allocated mem, conserve data

    if (new == NULL) return;
    memcpy(&new[ab->len], s, len); // add the new data at the end
    ab->b = new; // in the abuf struct set *new on *b bc of realloc
    ab->len += len; // in the abuf struct set new len from *s
}

void abFree(struct abuf *ab) {
    free(ab->b);
}


    /** OUTPUT */
void editorDrawRows(struct abuf *ab) {
    int y;
    for (y = 0; y < E.screenrows; y++) {
        abAppend(ab, "~", 1);

        abAppend(ab, "\x1b[J", 3); // clear line

        if (y < E.screenrows - 1) abAppend(ab, "\r\n", 2);

    }
}

void editorRefreshScreen() {
    struct abuf ab = ABUF_INIT;

    abAppend(&ab, "\x1b[?25l", 6); // hide cursor
    abAppend(&ab, "\x1b[H", 3); // place the cursor in top left

    editorDrawRows(&ab); // draw ~

    abAppend(&ab, "\x1b[H", 3); // place the cursor in top left
    abAppend(&ab, "\x1b[?25h", 6); // show cursor

    write(STDOUT_FILENO, ab.b, ab.len);
    abFree(&ab);
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
void initEditor() {
    int r = getWindowSize(&E.screenrows, &E.screencols);
    error(r != 0, "getWindowSize(): line %d\r\n\trows:%d  cols:%d", r, E.screenrows, E.screencols)
}

int main(){
    enableRawMode();
    initEditor();

    while (1) {
        editorRefreshScreen();
        editorProcessReadKey();
    }

    return 0;
}
