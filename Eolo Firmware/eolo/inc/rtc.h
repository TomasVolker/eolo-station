#ifndef RTC_H
#define RTC_H

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "sapi_rtc.h"

typedef struct {
    rtc_t time;
    float averageWindSpeed;
    float averageWindDirection;
    float averageRain;
} Measurement;

extern xQueueHandle measurementQueue;

void initRtc();

void periodicTask(void * args);

#endif
