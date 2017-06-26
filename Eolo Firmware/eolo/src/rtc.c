

#include <log.h>
#include <string_utils.h>
#include "rtc.h"

#include "board.h"

#include "FreeRTOS.h"

#include "semphr.h"

#include "print.h"

#include "wind_speed_sensor.h"
#include "wind_direction_sensor.h"
#include "rain_sensor.h"

#include "trigonometry.h"

#define MEASUREMENT_QUEUE_SIZE 60

xQueueHandle measurementQueue;

SemaphoreHandle_t secondSemaphore;
SemaphoreHandle_t minuteSemaphore;

float averageWindNorth;
float averageWindEast;
float averageRain;

void initRtc(){

    measurementQueue = xQueueCreate(MEASUREMENT_QUEUE_SIZE, sizeof(Measurement));

	secondSemaphore = xSemaphoreCreateBinary();
	minuteSemaphore = xSemaphoreCreateBinary();

	Chip_RTC_CntIncrIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC, ENABLE);
	Chip_RTC_AlarmIntConfig(LPC_RTC, RTC_AMR_CIIR_IMSEC | RTC_AMR_CIIR_IMMIN | RTC_AMR_CIIR_IMHOUR, DISABLE);

	Chip_RTC_Enable(LPC_RTC, ENABLE);

}

void enableRtcInterrupts(){

	Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);
	Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_ALARM);

	NVIC_EnableIRQ((IRQn_Type) RTC_IRQn);
}

void RTC_IRQHandler(void) {

	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	if (Chip_RTC_GetIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE)) {
		Chip_RTC_ClearIntPending(LPC_RTC, RTC_INT_COUNTER_INCREASE);

		xSemaphoreGiveFromISR(secondSemaphore, &xHigherPriorityTaskWoken);

		if (Chip_RTC_GetTime(LPC_RTC, RTC_TIMETYPE_SECOND) == 0) {
			xSemaphoreGiveFromISR(minuteSemaphore, &xHigherPriorityTaskWoken);
		}

	}

	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

}

void periodicTask(void * args) {

	enableRtcInterrupts();

    float windSpeed;
    float windDirection;

    averageWindNorth = 0;
    averageWindEast = 0;

    averageRain = 0;

    int count = 0;

	while(1) {

		xSemaphoreTake(secondSemaphore, portMAX_DELAY);

        windSpeed = getRealTimeWindSpeed() + 0.0001f;
        windDirection = getRealTimeWindDirection();

        averageWindNorth += (float) (windSpeed * cos(radians(windDirection)));
        averageWindEast += (float) (windSpeed * sin(radians(windDirection)));

        averageRain += getRealTimeRain();

        count++;

        if(xSemaphoreTake(minuteSemaphore, 0)) {

            Measurement measurement;

            rtcRead(&(measurement.time));

            averageWindNorth /= count;
            averageWindEast /= count;

            windSpeed = (float) hypot(averageWindEast, averageWindNorth);
            windDirection= (float) degrees(atan2(averageWindEast, averageWindNorth));

            if(windDirection < 0){
                windDirection += 360;
            }

            measurement.averageWindSpeed = windSpeed;
            measurement.averageWindDirection = windDirection;
            measurement.averageRain = averageRain/count;

            if(isLogVerbose()) {
                logInfo("NEW MEASUREMENT %d.%d kts %d.%d deg %d.%d mm",
                        i(windSpeed),
                        d(windSpeed,2),
                        i(windDirection),
                        d(windDirection, 2),
                        i(averageRain),
                        d(averageRain, 2)
                );
            }

            if(!xQueueSendToBack(measurementQueue, &measurement, 0)){

                Measurement droppedMeasurement;

                xQueueReceive(measurementQueue, &droppedMeasurement, 0);

                xQueueSendToBack(measurementQueue, &measurement, 0);

            }

            averageWindNorth = 0;
            averageWindEast = 0;
            count = 0;
        }

	}

}
