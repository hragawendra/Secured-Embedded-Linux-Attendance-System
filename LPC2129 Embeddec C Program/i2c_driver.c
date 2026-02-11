#include <LPC21xx.H>
#include "header.h"

#define SI ((I2CONSET >> 3) & 1)

void i2c_init(void)
{
	PINSEL0 |= 0x50;
	I2CONSET = (1 << 6);
	I2SCLH = I2SCLL = 75;
}

void i2c_send(u8 sa, u8 mr, u8 data)
{
	/* step1: Generate START condition */
	I2CONSET = (1 << 5);
	I2CONCLR = (1 << 3);
	while(SI == 0);
	I2CONCLR = (1 << 5);

	/* step2: Send SA+W & check ACK */
	I2DAT = sa;
	I2CONCLR = (1 << 3);
	while(SI == 0);
	if(I2STAT == 0x20)
	{
	 uart0_tx_string("ERROR: SA+W\r\n");
	 goto exit; 
	}

	/* step3: Send memory address & check ACK */
	I2DAT = mr;
	I2CONCLR = (1 << 3);
	while(SI == 0);
	if(I2STAT == 0x30)
	{
	 uart0_tx_string("ERROR: Memory Address\r\n");
	 goto exit; 
	}
	
	/* step4: Send data & check ACK */
	I2DAT = data;
	I2CONCLR = (1 << 3);
	while(SI == 0);
	if(I2STAT == 0x30)
	{
	 uart0_tx_string("ERROR: Data\r\n");
	 goto exit; 
	}
	
	/* step5: Generate STOP condition */
	exit:
			I2CONSET = (1 << 4);
			I2CONCLR = (1 << 3);
}

u8 i2c_receive(u8 sa, u8 mr)
{
	u8 temp;
	/* step1: Generate START condition */
	I2CONSET = (1 << 5);
	I2CONCLR = (1 << 3); 
	while(SI == 0);
	I2CONCLR = (1 << 5);
	
	/* step2: Send SA+W & check ACK */
	I2DAT = sa ^ 1;
	I2CONCLR = (1 << 3);
	while(SI == 0);
	if(I2STAT == 0x20)
	{
	 uart0_tx_string("ERROR: SA+W\r\n");
	 goto exit; 
	}
	
	/* step3: Send memory address & check ACK */
	I2DAT = mr;
	I2CONCLR = (1 << 3);
	while(SI == 0);
	if(I2STAT == 0x30)
	{
	 uart0_tx_string("ERR: Memory Addr\r\n");
	 goto exit; 
	}
	
	/* step4: Generate restart condition */
	I2CONSET = (1 << 5);
	I2CONCLR = (1 << 3); 
	while(SI == 0);
	I2CONCLR = (1 << 5);
	
	/* step5: Send SA+R & check ACK */
	I2DAT = sa;
	I2CONCLR = (1 << 3);
	while(SI == 0);
	if(I2STAT == 0x48)
	{
		uart0_tx_string("ERR: SA+R\r\n");
		goto exit;
	}
		
	/* step6: Read data from slave & send NACK */
	I2CONCLR = (1 << 3);
	while(SI == 0);
	temp = I2DAT;
	
	/* step7: Generate STOP condition */
	exit:
			I2CONSET = (1 << 4);
			I2CONCLR = (1 << 3);
	/* step8: Return received data */
	return temp;
}
