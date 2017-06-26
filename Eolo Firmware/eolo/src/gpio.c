#include "gpio.h"

#include "sapi_gpio.h"

void initGpio(){

	gpioConfig((gpioMap_t) 0, GPIO_ENABLE);

	gpioConfig(TEC1, GPIO_INPUT);
	gpioConfig(TEC2, GPIO_INPUT);
	gpioConfig(TEC3, GPIO_INPUT);
	gpioConfig(TEC4, GPIO_INPUT);

	gpioConfig(LEDR, GPIO_OUTPUT);
	gpioConfig(LEDG, GPIO_OUTPUT);
	gpioConfig(LEDB, GPIO_OUTPUT);
	gpioConfig(LED1, GPIO_OUTPUT);
	gpioConfig(LED2, GPIO_OUTPUT);
	gpioConfig(LED3, GPIO_OUTPUT);

}
