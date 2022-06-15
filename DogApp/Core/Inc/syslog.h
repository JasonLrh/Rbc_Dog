#ifndef SYSLOG_H
#define SYSLOG_H

#include <stdio.h>
#include <stdarg.h>

#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BROWN)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_D       LOG_COLOR(LOG_COLOR_BLUE)
#define LOG_COLOR_V       LOG_COLOR(LOG_COLOR_CYAN)
#define LOG_COLOR_N

#define LOG_FORMAT(letter, format)  LOG_COLOR_ ## letter "[%s:%d]" LOG_RESET_COLOR ":\t" format "\n"

#define ST_LOG(format, ...)  uart_printf(LOG_FORMAT(N,format), __FILE__, __LINE__, ##__VA_ARGS__)
#define ST_LOGI(format, ...) uart_printf(LOG_FORMAT(I,format), __FILE__, __LINE__, ##__VA_ARGS__)
#define ST_LOGW(format, ...) uart_printf(LOG_FORMAT(W,format), __FILE__, __LINE__, ##__VA_ARGS__)
#define ST_LOGD(format, ...) uart_printf(LOG_FORMAT(D,format), __FILE__, __LINE__, ##__VA_ARGS__)
#define ST_LOGE(format, ...) uart_printf(LOG_FORMAT(E,format), __FILE__, __LINE__, ##__VA_ARGS__)

int uart_printf(const char *fmt, ...);

#endif