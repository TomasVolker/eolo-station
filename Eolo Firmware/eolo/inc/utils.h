#ifndef UTILS_H
#define UTILS_H

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#define millis(x) ((x) / (portTICK_RATE_MS))

#define seconds(x) millis(1000*(x))

#endif
