
#include "board.h"

#include "utils.h"

#include "Eeprom.h"


#include "sapi_board.h"

#include "main.h"

#include "control.h"

#include "gpio.h"
#include "uart.h"
#include "rtc.h"

#include "wind_speed_sensor.h"
#include "wind_direction_sensor.h"
#include "rain_sensor.h"
#include "webservice.h"

#include "http.h"

void validateEeprom(){

    if(Board_EEPROM_readByte(EEPROM_ADDRESS_CHECK) != EEPROM_CHECK_TOKEN) {

        Board_EEPROM_clearData(EEPROM_ADDRESS_APN, MODEM_APN_MAX_SIZE);

        Board_EEPROM_clearData(EEPROM_ADDRESS_APN_USERNAME, MODEM_USERNAME_MAX_SIZE);

        Board_EEPROM_clearData(EEPROM_ADDRESS_APN_PASSWORD, MODEM_PASSWORD_MAX_SIZE);

        Board_EEPROM_clearData(EEPROM_ADDRESS_SERVER_URL, SERVER_URL_MAX_SIZE);

        Board_EEPROM_clearData(EEPROM_ADDRESS_SERVER_PATH, SERVER_PATH_MAX_SIZE);

        Board_EEPROM_clearData(EEPROM_ADDRESS_SERVER_PORT, 4);

        Board_EEPROM_clearData(EEPROM_ADDRESS_STATION_ID, 4);

        Board_EEPROM_writeByte(EEPROM_ADDRESS_CHECK, EEPROM_CHECK_TOKEN);
    }

}

int main(void) {

	boardConfig();

    Board_EEPROM_init();

    validateEeprom();

    initGpio();

	initUart();

    initRtc();

	//inicio tareas
    initUartTasks(tskIDLE_PRIORITY +1);

	xTaskCreate(cmdManagerTask, (const char *)"cmdManagerTask", 3*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+10, 0);

    xTaskCreate(windSpeedTask, (const char *)"windSpeedTask", 2*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+20, 0);
    xTaskCreate(rainTask, (const char *)"rainTask", 2*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+20, 0);
    xTaskCreate(windDirectionTask, (const char *)"windDirectionTask", 2*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+20, 0);

    xTaskCreate(modemManagerTask, (const char *)"modemManagerTask", 3*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+15, 0);

    xTaskCreate(periodicTask, (const char *)"periodicTask", 2*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+5, 0);

    xTaskCreate(webServiceTask, (const char *)"periodicTask", 3*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+2, 0);

	//xTaskCreate(httpTask, (const char *)"httpTask", 3*configMINIMAL_STACK_SIZE, 0, tskIDLE_PRIORITY+2, 0);

	//inicio scheduler
	vTaskStartScheduler();

	while (1) {}
}