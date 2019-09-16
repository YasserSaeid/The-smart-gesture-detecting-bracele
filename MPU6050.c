#include "MPU6050.h"

char data;

char I2C_read_byte_usci_b2(char address)
{
	data = I2C_read_usci_b2(MPU9150_ADDRESS, address);
//	printf(data);
	return data;
}

void I2C_write_byte_usci_b2(char address, char byte)
{
	/* send there 3 bytes via I2C to the AFE 4404, if it takes to long ErrorLED is turned on */
	if(I2C_write_usci_b2(MPU9150_ADDRESS, address, byte))
		P1OUT |= BIT1;
}

void write_char_usci_b2(char ch)
{
	/* wait until USCI_B2 module is ready */
	/* BIT UCTXIFG in Interrupt Flagregister UCB2IFG should be 0 */
 	while(!(UCB2IFG & UCTXIFG));

	/* write character to transmit buffer */
	UCB2TXBUF = ch;
}

char receive_char_usci_b2(void)
{
	char rcvByte;

	/* wait until USCI_B2 module is ready */
	/* BIT UCRXIFG in Interrupt Flagregister UCB2IFG should be 0 */

	while(!(UCB2IFG & UCRXIFG));
	rcvByte = UCB2RXBUF;
	return rcvByte;
}

void write_char_usci_b1(char ch)
{
	/* wait until USCI_B1 module is ready */
	/* BIT UCTXIFG in Interrupt Flagregister UCB1IFG should be 0 */
 	while(!(UCB1IFG & UCTXIFG));

	/* write character to transmit buffer */
	UCB1TXBUF = ch;
}

/**
 * \brief	send data via I2C (USCIB2)
 * \author	Frowin Buballa
 * \date	07.04.16
 *
 */
char I2C_write_usci_b2(char slaveAdd, char address, char data)
{
	unsigned long loop_counter = 0;
	UCB2I2CSA = slaveAdd;

	/* wait until bus is ready */
	while((UCB2STAT & UCBBUSY) && (loop_counter < 16000000)) 		//1,25s
		loop_counter++;

	if(loop_counter == 16000000)
		return 1;

	loop_counter = 0;

	/* declare master as Transmitter and transmit START condition */
	UCB2CTL1 |= UCTR;
	UCB2CTL1 |= UCTXSTT;

	/* send register address via I2C bus */
	write_char_usci_b2(address);

	/* wait until START condition got send */
	while((UCB2CTL1 & UCTXSTT) && (loop_counter < 16000000))
		loop_counter++;

	if(loop_counter == 16000000)
		return 1;

	loop_counter = 0;

	/* write data to register address via I2C bus */
		write_char_usci_b2(data);

		write_char_usci_b2(data);

//		write_char_usci_b2(data);
	/* send STOP condition */
	UCB2CTL1 |= UCTXSTP;

	/* check if STOP condition got send */
	while(UCB2CTL1 & UCTXSTP);

	return 0;
}

/**
 * \brief	receive data via I2C (USCIB2)
 * \author	Frowin Buballa
 * \date	07.04.16
 *
 * UCB2I2CSA needs to be set
 */

char I2C_read_usci_b2(char slaveAdd, char address)
{
	char data;

	UCB2I2CSA = slaveAdd;

	/* wait until bus is ready */
	while(UCB2STAT & UCBBUSY);

	/* declare master as Transmitter and transmit START condition */
	UCB2CTL1 |= UCTR;
	UCB2CTL1 |= UCTXSTT;

	/* send register address via I2C bus */
	write_char_usci_b2(address);

	/* wait until START condition got send */
	while(UCB2CTL1 & UCTXSTT);

	/* declare master as Receiver and transmit START condition */
	UCB2CTL1 &= ~UCTR;
	UCB2CTL1 |= UCTXSTT;

	/* wait until START condition got send */
	while(UCB2CTL1 & UCTXSTT);

	/* read data via I2C */
//	for(i = 0; i < (ByteCounter - 1); i++)
//	{
//		 data[i] = receive_char_usci_b2();
//	}
	/* prepare to send  STOP condition */
		UCB2CTL1 |= UCTXSTP;

	/* receive last byte */
	data = receive_char_usci_b2();

	/* prepare to send  STOP condition */
//	UCB2CTL1 |= UCTXSTP;

	/* check if STOP condition got send */
	while(UCB2CTL1 & UCTXSTP);

	return data;
}
