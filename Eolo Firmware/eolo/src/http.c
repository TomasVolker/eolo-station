#include "utils.h"
#include "sapi.h"
#include "sapi_gpio.h"

#include "http.h"


void httpTask(void * a) {

	gpioConfig(GPIO5, GPIO_OUTPUT);

	//RESET MODULE
	gpioWrite(GPIO5, OFF);

	vTaskDelay(seconds(2));

	gpioWrite(GPIO5, ON);

	vTaskDelay(seconds(10));

	uartWriteString(UART_232, "AT\n");

	vTaskDelay(seconds(1));

	uartWriteString(UART_232, "AT+CMEE=2\n");

	vTaskDelay(seconds(1));

	uartWriteString(UART_232, "AT+CFUN=1\n");

	vTaskDelay(seconds(4));

	uartWriteString(UART_232, "AT+CPIN?\n");

	vTaskDelay(seconds(1));

	uartWriteString(UART_232, "AT+CSTT=\"internet.movil\",\"internet\",\"internet\"\n");

	vTaskDelay(seconds(1));

	uartWriteString(UART_232, "AT+CIICR\n");

	vTaskDelay(seconds(5));

	uartWriteString(UART_232, "AT+CIFSR\n");

	vTaskDelay(seconds(2));

	uartWriteString(UART_232, "AT+CIPSTART=\"TCP\",\"google.com\",80\n");

	vTaskDelay(seconds(5));

	uartWriteString(UART_232, "AT+CIPSEND=15\n");

	vTaskDelay(seconds(1));

	uartWriteString(UART_232, "GET \n HTTP/1.0\n");

	while(1){
		vTaskDelay(seconds(60));
	}
}
