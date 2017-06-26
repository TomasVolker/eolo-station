#ifndef EOLO_LOG_H
#define EOLO_LOG_H

#include "board.h"

Bool isLogVerbose();

void setLogVerbose(Bool enable);

int logError(const char * format, ...);

int logDebug(const char * format, ...);

int logInfo(const char * format, ...);

int logResponse(const char * format, ...);

#endif //EOLO_LOG_H
