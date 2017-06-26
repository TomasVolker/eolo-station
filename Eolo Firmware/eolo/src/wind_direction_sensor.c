
#include "board.h"
#include "utils.h"

#include "semphr.h"

#include "sapi_adc.h"

#include "print.h"

#include "wind_direction_sensor.h"

#define WIND_DIRECTION_REALTIME_SAMPLE_PERIOD millis(500)

volatile float realTimeWindDirection;

void initWindDirectionSensor(){

    realTimeWindDirection = 0;

    adcConfig(ADC_ENABLE);

}

float getRealTimeWindDirection(){
    return realTimeWindDirection;
}

float computeWindDirection(int adcValue) {

    if (adcValue < 75) {
        return 112.5f;
    }

    if (adcValue < 88) {
        return 67.5f;
    }

    if (adcValue < 110) {
        return 90.0f;
    }

    if (adcValue < 156) {
        return 157.5f;
    }

    if (adcValue < 214) {
        return 135.0f;
    }

    if (adcValue < 265) {
        return 202.5f;
    }

    if (adcValue < 346) {
        return 180.0f;
    }

    if (adcValue < 433) {
        return 22.5f;
    }

    if (adcValue < 530) {
        return 45.0f;
    }

    if (adcValue < 615) {
        return 247.5f;
    }

    if (adcValue < 667) {
        return 225.0f;
    }

    if (adcValue < 744) {
        return 337.5f;
    }

    if (adcValue < 807) {
        return 0.0f;
    }

    if (adcValue < 857) {
        return 292.5f;
    }

    if (adcValue < 916) {
        return 315.0f;
    }

    return 270.0f;

}

void windDirectionTask(void* args){

    initWindDirectionSensor();

    uint16_t measured;

    while(1) {

        vTaskDelay(WIND_DIRECTION_REALTIME_SAMPLE_PERIOD);

        measured = adcRead(CH1);

        realTimeWindDirection = computeWindDirection(measured);

    }

}