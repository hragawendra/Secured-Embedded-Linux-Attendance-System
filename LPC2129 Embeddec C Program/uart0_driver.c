#include <LPC21xx.H>
#include "header.h"

#define THRE0 ((U0LSR >> 5) & 1)
#define RDR0 (U0LSR & 1)

void uart0_init(u32 baud)
{
	s32 pclk,result = 0;
	unsigned int a[] = {15, 60, 30};
	pclk = a[VPBDIV] * 1000000;
	result = pclk / (16 * baud);
	PINSEL0 |= 0X5;
	U0LCR = 0X83;
	U0DLL = result & 0XFF;
	U0DLM = (result >> 8) & 0XFF;
	U0LCR = 0X03;
}

void uart0_tx(u8 data)
{
	U0THR = data;
	while(THRE0 == 0);
}

u8 uart0_rx(void)
{
	while(RDR0 == 0);
	return U0RBR;
}

void uart0_tx_string(char *s)
{
	while(*s != 0)
	{
		uart0_tx(*s);
		s++;
	}
}
