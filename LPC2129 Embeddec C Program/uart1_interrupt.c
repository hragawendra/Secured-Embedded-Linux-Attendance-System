#include <LPC21xx.H>
#include "header.h"

char id[13];
u8 intr_flag = 0; //UART1 intr_flag
u8 rx_state = 0; // 0 = Card Mode, 1 = Linux Mode

void UART1_isr(void) __irq
{
	static int i = 0; // Index for id buffer
	s32 v = U1IIR;
 	v &= 0x0E;
	
 	if(v == 4)
	{
		if(rx_state == 0) // STATE 0: Reading RFID Card ID
		{
			id[i] = U1RBR;
			//U1THR = id[i];
			i++;
			if(i == 12)
			{
				id[i] = '\0';
				i = 0;
				intr_flag = 1; // Tell main() a card was scanned
				VICIntEnable = 1 << 6;
			}
		}
	}
	VICVectAddr = 0;
}

void config_uart1_intr(void)
{
	VICVectCntl0 = 7 | (1 << 5);
	VICVectAddr0 = (u32)UART1_isr;
	VICIntEnable = 1 << 7;
	U1IER = 1; // Enable RBR Interrupt
}
