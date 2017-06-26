
#include "log.h"
#include "print.h"

#define LOG_DEBUG

static int logUsb(const char * tag, int *varg);

Bool verbose;

Bool isLogVerbose(){
    return verbose;
}

void setLogVerbose(Bool enable) {
    verbose = enable;
}

int logError(const char * format, ...) {
    register int *varg = (int *)(&format);
    return logUsb("E", varg);
}

int logDebug(const char * format, ...) {
#ifdef LOG_DEBUG
    register int *varg = (int *)(&format);
    return logUsb("D", varg);
#else
    return 0;
#endif
}

int logInfo(const char * format, ...){
    register int *varg = (int *)(&format);
    return verbose? logUsb("I", varg): 0;
}

int logResponse(const char * format, ...){
    register int *varg = (int *)(&format);
    return logUsb("R", varg);
}

static int logUsb(const char * tag, int *varg) {
    uint32_t length = 0;

    length += usbUartFormat(tag);
    length += usbUartFormat(": ");
    length += print(NULL, UART_USB, varg);
    length += usbUartFormat("\n");

    return length;

}
