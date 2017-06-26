#ifndef EOLO_UART_H
#define EOLO_UART_H

#include "board.h"

#include "sapi_uart.h"

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"

extern xQueueHandle inputUartUsbQueue;
extern xQueueHandle outputUartUsbQueue;

extern xQueueHandle inputUart232Queue;
extern xQueueHandle outputUart232Queue;

BaseType_t uartUsbSendByte(char byte);
BaseType_t uart232SendByte(char byte);

Bool uartUsbSendBytes(const char * bytes, uint32_t size);
Bool uart232SendBytes(const char * bytes, uint32_t size);

void initUart();
void initUartTasks(uint32_t priority);

void uart232InputTask(void * a);
void uart232OutputTask(void * a);
void uart232Callback(const uartMap_t uart, uint8_t data);

void uartUsbInputTask(void * a);
void uartUsbOutputTask(void * a);
void uartUsbCallback(const uartMap_t uart, uint8_t data);


#endif //EOLO_UART_H
