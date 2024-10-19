#include "log.h"

static int logTypeLevel = LOG_INFO;

void setTraceLogLevel(int logType) {
    logTypeLevel = logType;
}

void log_info(const char *format, ...) {
    if (LOG_INFO >= logTypeLevel) {
        va_list args;
        va_start(args, format);
        fprintf(stdout, "\r\n%s -> %s():%i \r\n\t", __FILE_NAME__, __FUNCTION__, __LINE__);
        vfprintf(stdout, format, args);
        printf("\r\n");
        va_end(args);
    }
}

void log_error(int errorCondition, const char *format, ...) {
    if (LOG_ERROR >= logTypeLevel) {
        if (errorCondition) {
            va_list args;
            va_start(args, format);
            fprintf(stderr, "\033[1;31m\r\n%s -> %s():%i -> Error(%i):\r\n\t%s\r\n\t",
            __FILE_NAME__, __FUNCTION__, __LINE__, errno, strerror(errno));
            vfprintf(stderr, format, args);
            printf("\r\n");
            va_end(args);
            exit(EXIT_FAILURE);
        }
    }
}
