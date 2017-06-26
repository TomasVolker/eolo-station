#include "utils.h"
#include "uart.h"

#define QUEUE_IN_SIZE 512
#define QUEUE_OUT_SIZE 256

#undef TRUE
#undef FALSE

xQueueHandle inputUartUsbQueue;
xQueueHandle outputUartUsbQueue;

xQueueHandle inputUart232Queue;
xQueueHandle outputUart232Queue;

void initUart() {

    //Debug UART
    inputUartUsbQueue = xQueueCreate(QUEUE_IN_SIZE, sizeof(char));
    outputUartUsbQueue = xQueueCreate(QUEUE_OUT_SIZE, sizeof(char));

    uartConfigCb(UART_USB, uartUsbCallback);
    uartConfig(UART_USB, 115200);

    //MODEM UART
    inputUart232Queue = xQueueCreate(QUEUE_IN_SIZE, sizeof(char));
    outputUart232Queue = xQueueCreate(QUEUE_OUT_SIZE, sizeof(char));

    uartConfigCb(UART_232, uart232Callback);
    uartConfig(UART_232, 57600);

}

void initUartTasks(uint32_t priority){
    xTaskCreate(uartUsbOutputTask, (const char *)"uartUsbOutputTask", configMINIMAL_STACK_SIZE, 0, priority, 0);
    xTaskCreate(uart232OutputTask, (const char *)"uart232OutputTask", configMINIMAL_STACK_SIZE, 0, priority, 0);
}

BaseType_t uartUsbSendByte(char byte) {
    return xQueueSendToBack(outputUartUsbQueue, ( void * ) &byte, seconds(1));
}

Bool uartUsbSendBytes(const char * bytes, uint32_t size) {

    Bool result = TRUE;

    int i;

    for (i = 0; i < size; ++i) {
        if(uartUsbSendByte(bytes[i]) == errQUEUE_FULL) {
            result = FALSE;
        }
    }

    return result;
}

BaseType_t uart232SendByte(char byte) {
    return xQueueSendToBack(outputUart232Queue, ( void * ) &byte, seconds(1));
}

Bool uart232SendBytes(const char * bytes, uint32_t size) {

    Bool result = TRUE;

    int i;

    for (i = 0; i < size; ++i) {
        if(uart232SendByte(bytes[i]) == errQUEUE_FULL) {
            result = FALSE;
        }
    }

    return result;

}


void uart232OutputTask(void * a){

    char sendByte;

    while(1) {

        if(xQueueReceive(outputUart232Queue, &sendByte, portMAX_DELAY) != errQUEUE_EMPTY){
            uartWriteByte(UART_232, sendByte);
        }

    }

}

void uartUsbOutputTask(void * a){

    char sendByte;

    while(1) {

        if(xQueueReceive(outputUartUsbQueue, &sendByte, portMAX_DELAY) != errQUEUE_EMPTY){
            uartWriteByte(UART_USB, sendByte);
        }

    }

}

void uartUsbCallback(const uartMap_t uart, uint8_t data){

    portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

    xQueueSendToBackFromISR(inputUartUsbQueue, &data, &higherPriorityTaskWoken);

    portEND_SWITCHING_ISR(higherPriorityTaskWoken);
}

void uart232Callback(const uartMap_t uart, uint8_t data){

    portBASE_TYPE higherPriorityTaskWoken = pdFALSE;

    xQueueSendToBackFromISR(inputUart232Queue, &data, &higherPriorityTaskWoken);

    portEND_SWITCHING_ISR(higherPriorityTaskWoken);
}

