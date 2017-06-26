#include <Eeprom.h>
#include "modem.h"
#include "string_utils.h"
#include "utils.h"
#include "board.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sapi_uart.h"
#include "sapi_gpio.h"

#include "log.h"

//#define MODEM_DEBUG

#undef TRUE
#undef FALSE

#define MODEM_BUFFER_SIZE 256

#define MODEM_TCP_IN_BUFFER_SIZE 512
#define MODEM_TCP_TIMEOUT seconds(20)

const char * MODEM_AT = "AT\n";
const char * MODEM_SET_FUNCTION = "AT+CFUN=1\n";
const char * MODEM_CHECK_PIN = "AT+CPIN?\n";
const char * MODEM_SET_APN = "AT+CSTT=\"%s\",\"%s\",\"%s\"\n";
const char * MODEM_START_GPRS = "AT+CIICR\n";
const char * MODEM_GET_IP = "AT+CIFSR\n";
const char * MODEM_START_TCP = "AT+CIPSTART=\"TCP\",\"%s\",%d\n";
const char * MODEM_TCP_SEND = "AT+CIPSEND=%d\n";
const char * MODEM_TCP_CLOSE = "AT+CIPCLOSE\n";

const char * MODEM_DISABLE_ECHO = "AT E0\n";

const char * MODEM_OK = "OK";
const char * MODEM_CONNECT_OK = "CONNECT OK";
const char * MODEM_SEND_OK = "SEND OK";
const char * MODEM_ERROR = "ERROR";
const char * MODEM_PIN_READY = "+CPIN: READY";
const char * MODEM_TCP_CLOSED = "CLOSED";

const char MODEM_DATA = '>';

char modem_buffer[MODEM_BUFFER_SIZE];
uint32_t modem_buffer_index = 0;

char tcp_in_buffer[MODEM_TCP_IN_BUFFER_SIZE];
uint32_t tcp_in_buffer_index = 0;

struct ModemStatus modemStatus;

char apn[MODEM_APN_MAX_SIZE];
char username[MODEM_USERNAME_MAX_SIZE];
char password[MODEM_PASSWORD_MAX_SIZE];

xSemaphoreHandle resetSemaphore;

void resetDataModem(){
    xSemaphoreGive(resetSemaphore);
}

static const char * modemWaitForChar(uint32_t deadline){

    uint32_t now = xTaskGetTickCount();

    char modemChar;

    if(xQueueReceive(
                  inputUart232Queue,
                  &modemChar,
                  deadline > now ? deadline - now : 0
          ) != errQUEUE_EMPTY) {

        modem_buffer[0] = modemChar;
        modem_buffer[1] = '\0';

        return modem_buffer;
    }

    //timeout
    return NULL;

}


static const char * modemWaitForLine(uint32_t deadline){

    char modemChar;

    uint32_t now = xTaskGetTickCount();

    while(xQueueReceive(
                  inputUart232Queue,
                  &modemChar,
                  deadline > now ? deadline - now : 0
          ) != errQUEUE_EMPTY) {

        switch(modemChar) {
            case '\n':
            case '\r':

                modem_buffer[modem_buffer_index] = '\0';
                modem_buffer_index = 0;

                return modem_buffer;

            default:

                //guardo y reinicio el buffer en caso de overflow
                modem_buffer[modem_buffer_index] = modemChar;
                modem_buffer_index++;
                if(modem_buffer_index >= MODEM_BUFFER_SIZE){
                    modem_buffer_index = 0;
                }

        }

    }

    //timeout
    return NULL;

}

static Bool modemWaitForIp(uint32_t timeout){

    uint32_t deadline = xTaskGetTickCount() + timeout;

    const char * line;

    while((line = modemWaitForLine(deadline)) != NULL) {

#ifdef MODEM_DEBUG
        if(stringLength(line) > 0){
            usbUartFormat(line);
            usbUartFormat("\n");
        }
#endif

        if(isDigit(line[0])) {
            stringCopyMax(line, modemStatus.ip, MODEM_IP_BUFFER-1);
            return TRUE;
        }

        if(stringEquals(line, MODEM_ERROR)) {
            return FALSE;
        }

    }

    return FALSE;

}

static Bool modemWaitForCharResponse(char c, uint32_t timeout){

    uint32_t deadline = xTaskGetTickCount() + timeout;

    const char * pc;

    while((pc = modemWaitForChar(deadline)) != NULL) {

#ifdef MODEM_DEBUG
        if(stringLength(pc) > 0){
            usbUartFormat(pc);
            usbUartFormat("\n");
        }
#endif

        if(*pc == c) {
            return TRUE;
        }

    }

    return FALSE;

}

static Bool modemWaitForResponse(const char* response, uint32_t timeout){

    uint32_t deadline = xTaskGetTickCount() + timeout;

    const char * line;

    while((line = modemWaitForLine(deadline)) != NULL) {

#ifdef MODEM_DEBUG
        if(stringLength(line) > 0){
            usbUartFormat(line);
            usbUartFormat("\n");
        }
#endif

        if(stringEquals(line, response)) {
            return TRUE;
        }

        if(stringEquals(line, MODEM_ERROR)) {
            return FALSE;
        }

    }

    return FALSE;

}

static Bool resetModem() {

    modemStatus.initialized = FALSE;
    modemStatus.simReady = FALSE;
    modemStatus.joined = FALSE;
    modemStatus.ip[0] = '\0';

	//Reset
	gpioConfig(GPIO5, GPIO_OUTPUT);

    logInfo("MODEM INITIALIZING");

	gpioWrite(GPIO5, OFF);
    vTaskDelay(seconds(2));
    gpioWrite(GPIO5, ON);

    vTaskDelay(seconds(10));

    modemUartFormat(MODEM_AT);

    if (!modemWaitForResponse(MODEM_OK, seconds(1))) {
        logError("MODEM INIT AT");
        return FALSE;
    }
/*
    modem_uart(MODEM_DISABLE_ECHO);
    if (!modemWaitForResponse(MODEM_OK, seconds(10))) {
        pc_uart("MODEM INIT ERROR ECHO\n");
        return;
    }
*/
    modemUartFormat(MODEM_SET_FUNCTION);

    if (!modemWaitForResponse(MODEM_OK, seconds(1))) {
        logError("MODEM INIT FUN");
        return FALSE;
    }

    modemStatus.initialized = TRUE;
    logInfo("MODEM INIT SUCCESS");

    return TRUE;
}

Bool checkSim(){

    modemUartFormat(MODEM_CHECK_PIN);
    modemStatus.simReady = modemWaitForResponse(MODEM_PIN_READY, seconds(10));

    if(modemStatus.simReady) {
        modemWaitForResponse(MODEM_OK, seconds(10));
    }

    return modemStatus.simReady;
}

Bool joinNetwork(const char * apn, const char * username, const char * password){

    modemUartFormat(MODEM_SET_APN, apn, username, password);

    if(!modemWaitForResponse(MODEM_OK, seconds(2))) {
        return modemStatus.joined = FALSE;
    }

    modemUartFormat(MODEM_START_GPRS);

    if(!modemWaitForResponse(MODEM_OK, seconds(30))) {
        return modemStatus.joined = FALSE;
    }

    modemUartFormat(MODEM_GET_IP);

    if (!modemWaitForIp(seconds(10))) {
        return modemStatus.joined = FALSE;
    }

    return modemStatus.joined = TRUE;

}

Bool startTcpConnection(const char * url, uint32_t port){

    modemUartFormat(MODEM_START_TCP, url, (int) port);
    if(!modemWaitForResponse(MODEM_OK, seconds(10))) {
        return FALSE;
    }

    if(!modemWaitForResponse(MODEM_CONNECT_OK, seconds(30))) {
        return FALSE;
    }

    return TRUE;
}

Bool tcpSendData(const char * data, uint32_t size){

    modemUartFormat(MODEM_TCP_SEND, (int) size);

    if(!modemWaitForCharResponse(MODEM_DATA, seconds(10))) {
        return FALSE;
    }

    uart232SendBytes(data, size);

    modemUartFormat("\n");

    if(!modemWaitForResponse(MODEM_SEND_OK, seconds(60))) {
        return FALSE;
    }

    return TRUE;
}

const char* tcpReceiveData(){

    char modemChar;

    Bool tcpBufferOverflow = FALSE;

    tcp_in_buffer[0] = '\0';
    tcp_in_buffer_index = 0;

    while(1){

        if(xQueueReceive(inputUart232Queue, &modemChar, MODEM_TCP_TIMEOUT) != errQUEUE_EMPTY){

            switch(modemChar) {
                case '\0':
                case '\n':
                case '\r':

                    modem_buffer[modem_buffer_index] = '\0';

                    if(stringEquals(modem_buffer, MODEM_TCP_CLOSED)){
                        tcp_in_buffer[tcp_in_buffer_index] = '\0';
                        return tcp_in_buffer;
                    } else {

                        modem_buffer[modem_buffer_index] = modemChar;
                        modem_buffer_index++;

                        if(!tcpBufferOverflow) {

                            if (tcp_in_buffer_index + modem_buffer_index > MODEM_TCP_IN_BUFFER_SIZE-1) {
                                tcpBufferOverflow = TRUE;
                                modem_buffer_index = MODEM_TCP_IN_BUFFER_SIZE-1 - tcp_in_buffer_index;
                            }

                            bytesCopy(modem_buffer, tcp_in_buffer + tcp_in_buffer_index, modem_buffer_index);
                            tcp_in_buffer_index += modem_buffer_index;

                        }

                        modem_buffer_index = 0;
                    }

                    break;

                default:

                    //guardo y reinicio el buffer en caso de overflow
                    modem_buffer[modem_buffer_index] = modemChar;
                    modem_buffer_index++;
                    if(modem_buffer_index >= MODEM_BUFFER_SIZE){

                        if(!tcpBufferOverflow) {

                            if (tcp_in_buffer_index + modem_buffer_index > MODEM_TCP_IN_BUFFER_SIZE-1) {
                                tcpBufferOverflow = TRUE;
                                modem_buffer_index = MODEM_TCP_IN_BUFFER_SIZE-1 - tcp_in_buffer_index;
                            }

                            bytesCopy(modem_buffer, tcp_in_buffer + tcp_in_buffer_index, modem_buffer_index);
                            tcp_in_buffer_index += modem_buffer_index;

                        }

                        modem_buffer_index = 0;
                    }

            }

        } else {
            return tcp_in_buffer;
        }

    }

}

void tcpClose(){
    modemUartFormat(MODEM_TCP_CLOSE);
}

void modemManagerTask(void * a){

    Board_EEPROM_readData(EEPROM_ADDRESS_APN, (uint8_t *) apn, MODEM_APN_MAX_SIZE);
    Board_EEPROM_readData(EEPROM_ADDRESS_APN_USERNAME, (uint8_t *) username, MODEM_USERNAME_MAX_SIZE);
    Board_EEPROM_readData(EEPROM_ADDRESS_APN_PASSWORD, (uint8_t *) password, MODEM_PASSWORD_MAX_SIZE);

    logInfo("APN: %s", apn);
    logInfo("APN USER: %s", username);
    logInfo("APN PASS: %s", password);

    resetSemaphore = xSemaphoreCreateBinary();

    while (1) {

        if(resetModem()) {

            vTaskDelay(seconds(5));

            if(checkSim()) {

                logInfo("SIM OK");

                vTaskDelay(seconds(5));

                if(joinNetwork(apn, username, password)) {

                    logInfo("GPRS JOINED");

                } else {

                    logError("GPRS NOT JOINED");

                    vTaskDelay(seconds(30));

                    continue;
                }

            } else {

                logError("SIM ERROR");

                vTaskDelay(seconds(10));

                continue;

            }

        } else {

            logError("MODEM INIT ERROR");

            continue;

        }

        xSemaphoreTake(resetSemaphore, portMAX_DELAY);

    }

}