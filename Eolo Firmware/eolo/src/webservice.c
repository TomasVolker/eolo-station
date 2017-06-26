
#include <Eeprom.h>
#include "sapi.h"
#include "rtc.h"
#include "utils.h"
#include "log.h"
#include "webservice.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

#include "string_utils.h"
#include "print.h"
#include "rtc.h"
#include "modem.h"

#undef TRUE
#undef FALSE

#define HTTP_BUFFER_SIZE 512
#define JSON_BUFFER_SIZE 124
#define DATE_BUFFER 32

#define RESET_REQUEST_ERRORS 10


const char * HTTP_TEMPLATE = "POST %s HTTP/1.1\r\nHost: eolo-server.herokuapp.com\r\nConnection: Close\r\nContent-Type: application/json\r\nContent-Length: %d\r\n\r\n%s";//
const char * JSON_TEMPLATE = "{\"id\":%d,\"m\":{\"i\":\"%s\",\"ws\":%d.%02d,\"wd\":%d.%02d,\"r\":%d.%02d}}\n";
const char * DATE_TEMPLATE = "%d/%02d/%02d %02d:%02d:%02d";
const char * SERVER_RESPONSE_OK = " {\"r\":\"OK\"}";

/*

char serverUrl[SERVER_URL_MAX_SIZE] = "eolo-server.herokuapp.com";
char serverPath[SERVER_PATH_MAX_SIZE] = "/internal/station";
uint32_t serverPort = 80;
uint32_t stationId = 1;

 */

char serverUrl[SERVER_URL_MAX_SIZE];
char serverPath[SERVER_PATH_MAX_SIZE];
uint32_t serverPort;
uint32_t stationId;

char http_buffer[HTTP_BUFFER_SIZE];
char json_buffer[JSON_BUFFER_SIZE];
char date_buffer[DATE_BUFFER];

/*
POST %s HTTP/1.1
Host: eolo-server.herokuapp.com
Connection: Close
Content-Type: application/json
Content-Length: %d

{"id":%d,"m":{"i":"%s","ws":%d.%02d,"wd":%d.%02d,"r":%d.%02d}}
 */

/*
HTTP/1.1 200 OK
Date: Mon, 27 Jul 2009 12:28:53 GMT
Server: Apache/2.2.14 (Win32)
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT
Content-Length: 88
Content-Type: application/json
Connection: Closed

 {"r":"OK"}
 {"r":"ERROR"}
 */

void clearBuffer(char * buffer, int size){

    int i;
    for (i = 0; i < size; ++i) {
        buffer[i] = '\0';
    }

}

void clearBuffers(){

    clearBuffer(http_buffer, HTTP_BUFFER_SIZE);

    clearBuffer(json_buffer, JSON_BUFFER_SIZE);

    clearBuffer(date_buffer, DATE_BUFFER);

}

void prepareHttpRequest(const char * serverPath, Measurement* measurement){

    clearBuffers();

    rtc_t* time = &(measurement->time);

    sprintf(
            date_buffer,
            DATE_TEMPLATE,
            time->year,
            time->month,
            time->mday,
            time->hour,
            time->min,
            time->sec
    );

    sprintf(
            json_buffer,
            JSON_TEMPLATE,
            stationId,
            date_buffer,
            i(measurement->averageWindSpeed),
            d(measurement->averageWindSpeed, 2),
            i(measurement->averageWindDirection),
            d(measurement->averageWindDirection, 2),
            i(measurement->averageRain),
            d(measurement->averageRain, 2)
    );

    sprintf(http_buffer, HTTP_TEMPLATE, serverPath, stringLength(json_buffer), json_buffer);

}

const char * skipHttpHeader(const char * httpResponse){

    if(httpResponse == NULL || httpResponse[0] == '\0'){
        return NULL;
    }

    uint32_t i = 1;

    char c;
    char prevC = httpResponse[0];

    while((c=httpResponse[i]) != '\0'){

        if(c != '\r'){

            if (prevC == '\n' && c == '\n'){
                return httpResponse+i+1;
            }

            prevC = c;

        }

        i++;
    }

    return NULL;
}

Bool checkIfAnswerSucceeded(const char * responseBody){
    return stringEqualsIgnoreWhiteChars(SERVER_RESPONSE_OK, responseBody);
}

Bool sendMeasurementToServer(Measurement* measurement){

    if(isEmpty(http_buffer)) {
        prepareHttpRequest(serverPath, measurement);
    }

    if(!modemStatus.initialized){
        logError("WS MODEM NOT INIT");
        return FALSE;
    }

    if(!modemStatus.simReady) {
        logError("WS SIM NOT READY");
        return FALSE;
    }

    if(!modemStatus.joined) {
        logError("WS NETWORK NOT JOINED");
        return FALSE;
    }

    if(!startTcpConnection(serverUrl, serverPort)) {
        logError("WS TCP CONNECTION");
        return FALSE;
    }

    vTaskDelay(millis(200));

    if(!tcpSendData(http_buffer, stringLength(http_buffer))) {
        tcpClose();
        logError("WS TCP SEND DATA");
        return FALSE;
    }

    const char* data = tcpReceiveData();

    //logDebug(data);

    data = skipHttpHeader(data);

    if(data == NULL){
        logError("WS HTTP HEADER");
        return FALSE;
    }

    if(!checkIfAnswerSucceeded(data)){
        logError("WS RESPONSE");
        return FALSE;
    }

    clearBuffers();

    logInfo("WS UPLOAD SUCCESS");

    return TRUE;
}

void webServiceTask(void * args){

    clearBuffers();

    Board_EEPROM_readData(EEPROM_ADDRESS_SERVER_URL, (uint8_t *) serverUrl, SERVER_URL_MAX_SIZE);
    Board_EEPROM_readData(EEPROM_ADDRESS_SERVER_PATH, (uint8_t *) serverPath, SERVER_PATH_MAX_SIZE);
    Board_EEPROM_readData(EEPROM_ADDRESS_SERVER_PORT, (uint8_t *) &serverPort, 4);

    Board_EEPROM_readData(EEPROM_ADDRESS_STATION_ID, (uint8_t *) &stationId, 4);

    logInfo("SERVER URL: %s", serverUrl);
    logInfo("SERVER PATH: %s", serverPath);
    logInfo("SERVER PORT: %d", serverPort);

    logInfo("STATION ID: %d", stationId);

    Measurement measurement;

    uint32_t requestErrors;

    while(1) {

        requestErrors = 0;

        if(xQueueReceive(measurementQueue, &measurement, portMAX_DELAY)) {

            while(!sendMeasurementToServer(&measurement)){

                requestErrors++;

                if (requestErrors < RESET_REQUEST_ERRORS) {

                    vTaskDelay(seconds(2)*requestErrors);

                } else {

                    logError("WS MODEM NOT RESPONDING, RESETTING");

                    resetDataModem();
                    vTaskDelay(seconds(30));

                    break;

                }

            }

        }

    }

}


