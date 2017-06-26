
#include <Eeprom.h>
#include "sapi.h"
#include "print.h"
#include "log.h"

#include "string_utils.h"

#include "wind_speed_sensor.h"
#include "wind_direction_sensor.h"
#include "rain_sensor.h"

#include "uart.h"

#undef TRUE
#undef FALSE

const char * CMD_PING = "PING";
const char * CMD_TIME = "TIME=";
const char * CMD_AT = "AT";
const char * CMD_RT_WIND_SPEED = "RTWS=?";
const char * CMD_RT_WIND_DIRECTION = "RTWD=?";
const char * CMD_RT_RAIN = "RTR=?";
const char * CMD_VERBOSE = "VERBOSE=";

const char * CMD_APN = "APN=";
const char * CMD_APN_USERNAME = "APNUSER=";
const char * CMD_APN_PASSWORD = "APNPASS=";

const char * CMD_SERVER_URL = "SERVERURL=";
const char * CMD_SERVER_PATH = "SERVERPATH=";
const char * CMD_SERVER_PORT = "SERVERPORT=";

const char * CMD_STATION_ID = "STATIONID=";

const char * CMD_ENDLINE = "\n";

const char * CMD_REQUEST = "?";
const char * CMD_ARGUMENT_SEP = ",";

const char * RESPONSE_OK = "OK";
const char * RESPONSE_ERROR = "ERROR";
const char * RESPONSE_WIND_SPEED = "RTWS=%d.%d";
const char * RESPONSE_WIND_DIRECTION = "RTWD=%d.%d";
const char * RESPONSE_RAIN = "RTR=%d.%d";

#define CMD_BUFFER_SIZE 256

char cmd_buffer[CMD_BUFFER_SIZE];
int cmd_buffer_index = 0;



static const char * nextArgument(const char * cmd){
    return stringBeginsWith(cmd, CMD_ARGUMENT_SEP);
}

static void printRtc(){

    rtc_t rtc;
    rtcRead(&rtc);
    logResponse("%d-%d-%d %d:%dh %ds\r\n", rtc.year, rtc.month, rtc.mday, rtc.hour, rtc.min, rtc.sec);

}

static void parseDate(const char * args){

    int32_t parsed;

    if((args = parseInt(args, &parsed)) == NULL ||
       parsed == PARSE_ERROR) {
        logResponse(RESPONSE_ERROR);
        return;
    }

    rtc_t rtc;

    rtc.year = (uint16_t) parsed;

    if((args = nextArgument(args)) == NULL ||
       (args = parseInt(args, &parsed)) == NULL ||
       parsed == PARSE_ERROR) {
        logResponse(RESPONSE_ERROR);
        return;
    }

    rtc.month = (uint8_t) parsed;

    if((args = nextArgument(args)) == NULL ||
       (args = parseInt(args, &parsed)) == NULL ||
       parsed == PARSE_ERROR) {
        logResponse(RESPONSE_ERROR);
        return;
    }

    rtc.mday = (uint8_t) parsed;

    if((args = nextArgument(args)) == NULL ||
       (args = parseInt(args, &parsed)) == NULL ||
       parsed == PARSE_ERROR) {
        logResponse(RESPONSE_ERROR);
        return;
    }

    rtc.hour = (uint8_t) parsed;

    if((args = nextArgument(args)) == NULL ||
       (args = parseInt(args, &parsed)) == NULL ||
       parsed == PARSE_ERROR) {
        logResponse(RESPONSE_ERROR);
        return;
    }

    rtc.min = (uint8_t) parsed;

    if((args = nextArgument(args)) == NULL ||
       parseInt(args, &parsed) == NULL ||
       parsed == PARSE_ERROR) {
        logResponse(RESPONSE_ERROR);
        return;
    }

    rtc.sec = (uint8_t) parsed;

    rtcWrite(&rtc);

    logResponse(RESPONSE_OK);
}

static void processCmd(const char * cmd) {

    if(isEmpty(cmd)){
        return;
    }
/*
    if (stringBeginsWith(cmd, CMD_AT)) {
        modemUartFormat(cmd);
        modemUartFormat(CMD_ENDLINE);
        return;
    }
*/
    if (stringEquals(cmd, CMD_PING)) {

        logResponse(RESPONSE_OK);

        return;
    }

    const char * aux;

    if ((aux = stringBeginsWith(cmd, CMD_TIME))) {

        if(stringEquals(aux, CMD_REQUEST)) {
            printRtc();
        } else {
            parseDate(aux);
        }

        return;

    }

    if (stringEquals(cmd, CMD_RT_WIND_SPEED)) {
        float value = getRealTimeWindSpeed();
        logResponse(RESPONSE_WIND_SPEED, i(value), d(value, 2));
        return;
    }

    if (stringEquals(cmd, CMD_RT_WIND_DIRECTION)) {
        float value = getRealTimeWindDirection();
        logResponse(RESPONSE_WIND_DIRECTION, i(value), d(value, 1));
        return;
    }

    if (stringEquals(cmd, CMD_RT_RAIN)) {
        float value = getRealTimeRain();
        logResponse(RESPONSE_RAIN, i(value), d(value, 2));
        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_VERBOSE))) {

        switch (*aux) {
            case '0':
                setLogVerbose(FALSE);
                logResponse(RESPONSE_OK);
                break;
            case '1':
                setLogVerbose(TRUE);
                logResponse(RESPONSE_OK);
                break;
            default:
                logResponse(RESPONSE_ERROR);
        }

        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_APN))) {
        Board_EEPROM_writeData(EEPROM_ADDRESS_APN, (uint8_t *) aux, MODEM_APN_MAX_SIZE);
        logResponse(RESPONSE_OK);
        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_APN_USERNAME))) {
        Board_EEPROM_writeData(EEPROM_ADDRESS_APN_USERNAME, (uint8_t *) aux, MODEM_USERNAME_MAX_SIZE);
        logResponse(RESPONSE_OK);
        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_APN_PASSWORD))) {
        Board_EEPROM_writeData(EEPROM_ADDRESS_APN_PASSWORD, (uint8_t *) aux, MODEM_PASSWORD_MAX_SIZE);
        logResponse(RESPONSE_OK);
        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_SERVER_URL))) {
        Board_EEPROM_writeData(EEPROM_ADDRESS_SERVER_URL, (uint8_t *) aux, SERVER_URL_MAX_SIZE);
        logResponse(RESPONSE_OK);
        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_SERVER_PATH))) {
        Board_EEPROM_writeData(EEPROM_ADDRESS_SERVER_PATH, (uint8_t *) aux, SERVER_PATH_MAX_SIZE);
        logResponse(RESPONSE_OK);
        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_SERVER_PORT))) {
        int32_t parsed;
        parseInt(aux, &parsed);
        if (parsed != PARSE_ERROR) {
            Board_EEPROM_writeData(EEPROM_ADDRESS_SERVER_PORT, (uint8_t *) &parsed, 4);
            logResponse(RESPONSE_OK);
        } else {
            logResponse(RESPONSE_ERROR);
        }

        return;
    }

    if ((aux = stringBeginsWith(cmd, CMD_STATION_ID))) {
        int32_t parsed;
        parseInt(aux, &parsed);
        if (parsed != PARSE_ERROR) {
            Board_EEPROM_writeData(EEPROM_ADDRESS_STATION_ID, (uint8_t *) &parsed, 4);
            logResponse(RESPONSE_OK);
        } else {
            logResponse(RESPONSE_ERROR);
        }
        return;
    }

    logResponse(RESPONSE_ERROR);

}

void cmdManagerTask(void * a){

    setLogVerbose(TRUE);

    logInfo("CMD INIT");

	char cmdChar;

    while(1){

        xQueueReceive(inputUartUsbQueue, &cmdChar, portMAX_DELAY);

        switch(cmdChar) {
            case '\n':
            case '\r':

                cmd_buffer[cmd_buffer_index] = '\0';
                cmd_buffer_index = 0;

                processCmd(cmd_buffer);
                break;

            default:

                //guardo y reinicio el buffer en caso de overflow
                cmd_buffer[cmd_buffer_index] = cmdChar;
                cmd_buffer_index++;
                if(cmd_buffer_index >= CMD_BUFFER_SIZE){
                    cmd_buffer_index = 0;
                }

        }

    }

}

