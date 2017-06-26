#ifndef PRINT_H
#define PRINT_H

#include "uart.h"
#include "modem.h"
#include "sapi_uart.h"

int sprintf(char *out, const char *format, ...);
int debugFormat(const char *format, ...);
int modemUartFormat(const char *format, ...);
int usbUartFormat(const char *format, ...);

int print(char **out, uartMap_t uart, int *varg);

#endif
