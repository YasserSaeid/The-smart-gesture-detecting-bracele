#include "sensor.h"

int8_t new_data = 0;
uint16_t Druck[11] = {0};				// 12 bit ADC values
int16_t Accel[3] = {0}, Gyro[3] = {0};	// Accel and Gyro values
int16_t GYRO_OFFSET[3] = {0};			// Gyro Offset

void sensor_init(){

	// reset sensor
	I2C_write_byte_usci_b2(MPU6050_RA_PWR_MGMT_1, 0x00);

	// sensor settings
	// sensor test
	if(I2C_read_byte_usci_b2(MPU6050_RA_WHO_AM_I) == 0x68)
	{
		printf("i2c sensor read test passed.\n\r");
		uart_sendString("i2c sensor read test passed.\n\r");
	}

	// GYRO_CONFIG
	// set sensor to +-250°  0b00000000
	//				 +-500°  0b00001000
	//				 +-1000° 0b00010000
	//				 +-2000° 0b00011000
	I2C_write_byte_usci_b2(MPU6050_RA_GYRO_CONFIG, 0b00001000);
	if(I2C_read_byte_usci_b2(MPU6050_RA_GYRO_CONFIG) == 0b00001000)
	{
		printf("MPU6050_RA_GYRO_CONFIG set to 0b00001000 (+-500 degree)\n\r");
		uart_sendString("MPU6050_RA_GYRO_CONFIG set to 0b00001000 (+-500 degree)\n\r");
	}

	// ACCEL_CONFIG
	// set sensor to +-16g 0b00011000 <-
	// set sensor to +- 8g 0b00010000 <-
	//				 +- 4g 0b00001000
	//				 +- 2g 0b00000000
	I2C_write_byte_usci_b2(MPU6050_RA_ACCEL_CONFIG, 0b00011000);
	if(I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_CONFIG) == 0b00011000)
	{
		printf("MPU6050_RA_ACCEL_CONFIG set to 0b00011000 (+-16g)\n\r");
		uart_sendString("MPU6050_RA_ACCEL_CONFIG set to 0b00011000 (+-16g)\n\r");
	}

	// set sensor DLPF_CFG
	//   5Hz 0b00000110
	//  10Hz 0b00000101
	//  21Hz 0b00000100
	//  44Hz 0b00000011
	//  94Hz 0b00000010
	// 184Hz 0b00000001
	// 260Hz 0b00000000
	I2C_write_byte_usci_b2(MPU6050_RA_CONFIG, 0b00000011);
	if(I2C_read_byte_usci_b2(MPU6050_RA_CONFIG) == 0b00000011)
	{
		printf("MPU6050_RA_CONFIG set to 0b00000011 (digital lowpass filter: 44Hz)\n\r");
		uart_sendString("MPU6050_RA_CONFIG set to 0b00000011 (digital lowpass filter: 44Hz)\n\r");
	}

	int16_t i, j, AccelData[3], GyroData[3], GyroData_sum[3] = {0};

	for( i = 0 ; i < 1024; i++)
	{
		sensor_getData(&AccelData[0], &GyroData[0], &Druck[0]);

		for( j = 0 ; j < 3; j++)
		{
			GyroData_sum[j] += GyroData[j];
		}
	}

	for( j = 0 ; j < 3; j++)
	{
		GYRO_OFFSET[j] = GyroData_sum[j] / 1024;
		//GYRO_OFFSET[j] >>= 10;
	}
}



/* getData: get sensor data*/
void sensor_getData(int16_t accData[], int16_t gyrData[], uint16_t druckData[])
{
	// disable global interrupts
	__disable_interrupt();

	// get DATA needs to be written into Interrupttimer
	ADC12CTL0 |= ADC12SC;    // Starten der Umwandlung

	// Accelerometer
	uint8_t acc_low_x   = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_XOUT_L);
	uint8_t acc_high_x  = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_XOUT_H);
	accData[0] 		    = (int16_t) (acc_high_x  << 8 | acc_low_x);	// 2x 8 Bit -> 16 Bit

	uint8_t acc_low_y   = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_YOUT_L);
	uint8_t acc_high_y  = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_YOUT_H);
	accData[1] 		   	= (int16_t) (acc_high_y  << 8 | acc_low_y);	// 2x 8 Bit -> 16 Bit

	uint8_t acc_low_z   = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_ZOUT_L);
	uint8_t acc_high_z  = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_ACCEL_ZOUT_H);
	accData[2] 		   	= (int16_t) (acc_high_z  << 8 | acc_low_z);	// 2x 8 Bit -> 16 Bit

	// Gyrosensor
	uint8_t gyro_low_x   = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_GYRO_XOUT_L);
	uint8_t gyro_high_x  = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_GYRO_XOUT_H);
	gyrData[0] 			 = (int16_t) ((gyro_high_x << 8 | gyro_low_x) - GYRO_OFFSET[0]);	// 2x 8 Bit -> 16 Bit

	uint8_t gyro_low_y   = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_GYRO_YOUT_L);
	uint8_t gyro_high_y  = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_GYRO_YOUT_H);
	gyrData[1] 			 = (int16_t) ((gyro_high_y << 8 | gyro_low_y) - GYRO_OFFSET[1]);	// 2x 8 Bit -> 16 Bit

	uint8_t gyro_low_z   = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_GYRO_ZOUT_L);
	uint8_t gyro_high_z  = (uint8_t) I2C_read_byte_usci_b2(MPU6050_RA_GYRO_ZOUT_H);
	gyrData[2] 			 = (int16_t) ((gyro_high_z << 8 | gyro_low_z) - GYRO_OFFSET[2]);	// 2x 8 Bit -> 16 Bit

   	// enable global interrupts
	__enable_interrupt();

}


// Timer A0 Interrupt service routine
#pragma vector = TIMER1_A0_VECTOR
__interrupt void TIMER1_A0_ISR(void)
{
	P1OUT ^= 0x01;                            // Toggle P1.0
	new_data = 1;							  // Flag for new data
	// LPM0_EXIT; 								   Exit Low Power Mode 0

	// Add Offset to CCR0, wegen continuous mode
	TA1CCR0 += 328;
	// 16384 ->   1Hz Samplerate
	//   655 ->  50Hz Samplerate
	//   328 -> 100Hz Samplerate
}


// ADC12 Interrupt Service Routine
#pragma vector = ADC12_VECTOR               // Vektor fuer die Interrupt Vektor Routine (nachzulesen in MSP430 Optimizing C/C++ Compiler v 4.1 User's Guide)
__interrupt void ADC12_ISR(void)
{
  switch(__even_in_range(ADC12IV,34))		// Erzeugung der Switch-Anweisung fuer ADC12 Interrupt Vektor Routine
  {
  case  0: break;     		// Vektor  0:  No interrupt
  case  2: break;         	// Vektor  2:  ADC overflow
  case  4: break;          	// Vektor  4:  ADC timing overflow
  case  6:               	// Vektor  6:  ADC12IFG0
	  Druck[0]=ADC12MEM0;
	  break;
  case  8:					// Vektor  8:  ADC12IFG1
	  Druck[1]=ADC12MEM1;
	  break;
  case 10:              	// Vektor 10:  ADC12IFG2
	  Druck[2]=ADC12MEM2;
	  break;
  case 12:              	// Vektor 12:  ADC12IFG3
	  Druck[3]=ADC12MEM3;
	  break;
  case 14:                	// Vektor 14:  ADC12IFG4
	  Druck[4]=ADC12MEM4;
	  break;
  case 16:					// Vektor 16:  ADC12IFG5
	  Druck[5]=ADC12MEM5;
	  break;                           		
  case 18:					// Vektor 18:  ADC12IFG6
	  Druck[6]=ADC12MEM6;
	  break;                           
  case 20:					// Vektor 20:  ADC12IFG7
	  Druck[7]=ADC12MEM7;
	  break;                           
  case 22:					// Vektor 22:  ADC12IFG8
	  Druck[8]=ADC12MEM8;
	  break;                           
  case 24:					// Vektor 24:  ADC12IFG9
	  Druck[9]=ADC12MEM9;
	  break;                           
  case 26:					// Vektor 26:  ADC12IFG10
	  Druck[10]=ADC12MEM10;
	  break;                           
  case 28: break;        	// Vektor 28:  ADC12IFG11
  case 30: break;        	// Vektor 30:  ADC12IFG12
  case 32: break;          	// Vektor 32:  ADC12IFG13
  case 34: break;      		// Vektor 34:  ADC12IFG14
  default: break;
  }
}

