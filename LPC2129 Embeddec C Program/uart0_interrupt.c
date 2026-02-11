#include <LPC21xx.H>
#include "header.h"

u8 response_flag = 0; // Flag set when response is ready
u8 auth_flag; // Buffer for Linux response

void UART0_isr(void) __irq
{
	s32 v = U0IIR;
 	v &= 0x0E;
	
 	if(v == 4)
	{
		if(rx_state == 1) // STATE 1: Reading Response from Linux
		{
			auth_flag = U0RBR;
			//U0THR = auth_flag;
			response_flag = 1; // Tell main() response arrived
			rx_state = 0; // Go back to listening for cards automatically
			VICIntEnClr = 1 << 6;
		}
	}
	VICVectAddr = 0;
}

void config_uart0_intr(void)
{
	VICVectCntl1 = 6 | (1 << 5);
	VICVectAddr1 = (u32)UART0_isr;
	U0IER = 1; // Enable RBR Interrupt
}
