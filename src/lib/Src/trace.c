#define _GNU_SOURCE
#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "trace.h"

void trace_log(char *tag, int line_number, char * fmt, ...) {
    va_list va_list;
    char * output_buf;
    char * output_fmt;
    va_start(va_list, fmt);
    asprintf(&output_fmt, "[%s]\t@no.%d\t%s\r\n", tag, line_number, fmt);
    vasprintf(&output_buf, output_fmt, va_list);
    output(output_buf);
    va_end(va_list);
    free(output_buf);
    free(output_fmt);
}

void trace_log_header(char *tag, int line_number)
{
    char * output_buf;
    asprintf(&output_buf, "[%s] @no.%d\t", tag, line_number);
    output(output_buf);
    free(output_buf);
}

void trace_log_content(char * fmt, ...)
{
    char * output_buf;
    va_list va_list;
    va_start(va_list, fmt);
    vasprintf(&output_buf, fmt, va_list);
    output(output_buf);
    free(output_buf);
}

void trace_log_tail()
{
    output("\r\n");
}

void output(char * msg) {
    printf("%s", msg);
}