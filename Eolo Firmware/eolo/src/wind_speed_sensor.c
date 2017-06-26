
#include "board.h"
#include "utils.h"

#include "semphr.h"

#include "sapi_gpio.h"

#include "print.h"

#include "wind_speed_sensor.h"

#define WIND_SPEED_KNOTS_PER_TICK 1.2965126f

#define WIND_SPEED_GPIO GPIO3

#define WIND_SPEED_INTERRUPT_INDEX 0
#define WIND_SPEED_INTERRUPT_NUMBER PIN_INT0_IRQn
#define WIND_SPEED_INPUT_PORT 5
#define WIND_SPEED_INPUT_PIN 15

#define WIND_SPEED_MAX_SENSOR_TICKS 10000

#define WIND_SPEED_MIN_REALTIME_WINDOW millis(400)
#define WIND_SPEED_MAX_REALTIME_WINDOW seconds(3)

#define WIND_SPEED_DEBOUNCE_TICKS millis(5)

volatile float realTimeWindSpeed;
volatile int lastWindSpeedTimestamp;

xSemaphoreHandle windSpeedSemaphore;

void initWindSpeedSensor(){

    realTimeWindSpeed = 0;
    lastWindSpeedTimestamp = 0;

    windSpeedSemaphore = xSemaphoreCreateCounting(WIND_SPEED_MAX_SENSOR_TICKS, 0);

    gpioConfig(WIND_SPEED_GPIO, GPIO_INPUT);

    Chip_SCU_GPIOIntPinSel(WIND_SPEED_INTERRUPT_INDEX, WIND_SPEED_INPUT_PORT, WIND_SPEED_INPUT_PIN);

    /* Configure channel interrupt as edge sensitive and falling edge interrupt */
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(WIND_SPEED_INTERRUPT_INDEX));
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(WIND_SPEED_INTERRUPT_INDEX));
    Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(WIND_SPEED_INTERRUPT_INDEX));

    /* Enable interrupt in the NVIC */
    NVIC_ClearPendingIRQ(WIND_SPEED_INTERRUPT_NUMBER);
    NVIC_EnableIRQ(WIND_SPEED_INTERRUPT_NUMBER);

}

float getRealTimeWindSpeed(){
    return realTimeWindSpeed;
}

float computeWindSpeed(int timeTicks, int anemometerTickCount){

    float deltaSeconds = ((float) timeTicks)/configTICK_RATE_HZ;

    return WIND_SPEED_KNOTS_PER_TICK * anemometerTickCount / deltaSeconds;

}

void GPIO0_IRQHandler(void) {

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(WIND_SPEED_INTERRUPT_INDEX));

    int now = xTaskGetTickCountFromISR();

    if(now - lastWindSpeedTimestamp > WIND_SPEED_DEBOUNCE_TICKS) {
        xSemaphoreGiveFromISR(windSpeedSemaphore, &xHigherPriorityTaskWoken);
    }

    lastWindSpeedTimestamp = now;

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

}



void windSpeedTask(void* args){

    initWindSpeedSensor();

    int lastTimestamp = xTaskGetTickCount();
    int tickCount;
    int now;

    while(1) {

        vTaskDelay(WIND_SPEED_MIN_REALTIME_WINDOW);

        tickCount = 1;

        while(xSemaphoreTake(windSpeedSemaphore, 0)){
            tickCount++;
        }

        BaseType_t lastTickRead = xSemaphoreTake(windSpeedSemaphore, WIND_SPEED_MAX_REALTIME_WINDOW-WIND_SPEED_MIN_REALTIME_WINDOW);

        if(lastTickRead || tickCount > 1){

            now = xTaskGetTickCount();
            realTimeWindSpeed = computeWindSpeed(now - lastTimestamp, tickCount);
            lastTimestamp = now;

        } else {

            //Timeout
            realTimeWindSpeed = 0;

        }

    }

}
