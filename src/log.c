#include "log.h"

static void log_internal(const char *level, const char *fmt, va_list ap)
{
    fprintf(stdout, "[%s] ", level);
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);
}

void log_info(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_internal("INFO", fmt, ap);
    va_end(ap);
}

void log_error(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    log_internal("ERROR", fmt, ap);
    va_end(ap);
}
