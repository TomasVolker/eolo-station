
#include "board.h"
#include "utils.h"

#include "semphr.h"

#include "sapi_gpio.h"

#include "print.h"

#include "rain_sensor.h"

#define RAIN_MM_PER_TICK 0.2794f

#define RAIN_GPIO GPIO1

#define RAIN_INTERRUPT_INDEX 1
#define RAIN_INTERRUPT_NUMBER PIN_INT1_IRQn
#define RAIN_INPUT_PORT 3
#define RAIN_INPUT_PIN 3

#define RAIN_MAX_SENSOR_TICKS 10000

#define RAIN_MIN_REALTIME_WINDOW seconds(30)
#define RAIN_MAX_REALTIME_WINDOW seconds(60)

#define RAIN_DEBOUNCE_TICKS millis(5)

volatile float realTimeRain;
volatile int lastRainTimestamp;
xSemaphoreHandle rainSemaphore;

void initRainSensor(){

    realTimeRain = 0;
    lastRainTimestamp = 0;

    rainSemaphore = xSemaphoreCreateCounting(RAIN_MAX_SENSOR_TICKS, 0);

    gpioConfig(RAIN_GPIO, GPIO_INPUT);


    Chip_SCU_GPIOIntPinSel(RAIN_INTERRUPT_INDEX, RAIN_INPUT_PORT, RAIN_INPUT_PIN);

    /* Configure channel interrupt as edge sensitive and falling edge interrupt */
    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(RAIN_INTERRUPT_INDEX));
    Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH(RAIN_INTERRUPT_INDEX));
    Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH(RAIN_INTERRUPT_INDEX));

    /* Enable interrupt in the NVIC */
    NVIC_ClearPendingIRQ(RAIN_INTERRUPT_NUMBER);
    NVIC_EnableIRQ(RAIN_INTERRUPT_NUMBER);

}

float getRealTimeRain(){
    return realTimeRain;
}

float computeRain(int timeTicks, int rainTickCount){

    float deltaMinutes = ((float) timeTicks)/(60*configTICK_RATE_HZ);

    return RAIN_MM_PER_TICK * rainTickCount / deltaMinutes;

}

void GPIO1_IRQHandler(void) {

    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH(RAIN_INTERRUPT_INDEX));

    int now = xTaskGetTickCountFromISR();

    if(now - lastRainTimestamp > RAIN_DEBOUNCE_TICKS) {
        xSemaphoreGiveFromISR(rainSemaphore, &xHigherPriorityTaskWoken);
    }

    lastRainTimestamp = now;

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);

}



void rainTask(void* args){

    initRainSensor();

    int lastTimestamp = xTaskGetTickCount();
    int tickCount;
    int now;

    while(1) {

        vTaskDelay(RAIN_MIN_REALTIME_WINDOW);

        tickCount = 1;

        while(xSemaphoreTake(rainSemaphore, 0)){
            tickCount++;
        }

        BaseType_t lastTickRead = xSemaphoreTake(rainSemaphore, RAIN_MAX_REALTIME_WINDOW-RAIN_MIN_REALTIME_WINDOW);

        if(lastTickRead || tickCount > 1){

            now = xTaskGetTickCount();
            realTimeRain = computeRain(now - lastTimestamp, tickCount);
            lastTimestamp = now;

        } else {

            //Timeout
            realTimeRain = 0;

        }

    }

}
