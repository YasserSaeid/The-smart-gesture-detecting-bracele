#include <msp430.h>
#include "driverlib.h"
#include "uart.h"
#define BAUD_RATE	9600

void uart_init() {

	//Init uart on Port 5
	//RX --> Pin 53
	//TX --> Pin 54
	GPIO_setAsPeripheralModuleFunctionInputPin(
	GPIO_PORT_P5,
	GPIO_PIN6 + GPIO_PIN7);

	//Baudrate = 115200, clock freq = 6 MHz
	//UCBRx = 52, UCBRFx = 0, UCBRSx = 1, UCOS16 = 0
	USCI_A_UART_initParam param = {0};
	param.selectClockSource = USCI_A_UART_CLOCKSOURCE_SMCLK;
	param.clockPrescalar = 52;
	param.firstModReg = 0;
	param.secondModReg = 1;
	param.parity = USCI_A_UART_NO_PARITY;
	param.msborLsbFirst = USCI_A_UART_LSB_FIRST;
	param.numberofStopBits = USCI_A_UART_ONE_STOP_BIT;
	param.uartMode = USCI_A_UART_MODE;
	param.overSampling = USCI_A_UART_LOW_FREQUENCY_BAUDRATE_GENERATION;

	if (STATUS_FAIL == USCI_A_UART_init(USCI_A1_BASE, &param)) {
		return;
	}

	//Enable UART module for operation
	USCI_A_UART_enable(USCI_A1_BASE);
}

void uart_sendChar(const char *c) {
	uart_sendString(c);
}

void uart_sendString(const char *string) {
	while (*string) {
		uart_sendData(*string++);
	}
}

void uart_sendData(uint8_t data){
	USCI_A_UART_transmitData(USCI_A1_BASE, data);
}


/* reverse:  reverse string s in place */
void tools_reverse(char s[])
{
	int16_t i, j;
	char c;

	for (i = 0, j = strlen(s)-1; i<j; i++, j--)
	{
		c = s[i];
		s[i] = s[j];
		s[j] = c;
	}
}

/* itoa:  convert n to characters in s */
void tools_itoa(int16_t n, char s[])
{
	int16_t i, sign;

	if ((sign = n) < 0)  /* record sign */
		n = -n;          /* make n positive */
	
	i = 0;
	
	do
	{       /* generate digits in reverse order */
		s[i++] = n % 10 + '0';   /* get next digit */
	} while ((n /= 10) > 0);     /* delete it */
	
	if (sign < 0)
		s[i++] = '-';

	s[i] = '\0';
	tools_reverse(s);
}


void uart_sendSensorData(int16_t accData[], int16_t gyrData[])
{
	char result_char[10];
	int i;

	// Send Accelerometer Data
	for( i = 0 ; i < 3; i++)
	{
		tools_itoa(accData[i], result_char);
		uart_sendString(result_char);
		uart_sendString(";");
	}

	// Send Gyro Data
	for( i = 0 ; i < 2; i++)
	{
		tools_itoa(gyrData[i], result_char);
		uart_sendString(result_char);
		uart_sendString(";");
	}
	tools_itoa(gyrData[2], result_char);
	uart_sendString(result_char);
	uart_sendString("\n\r");
}
