#include <LPC21xx.H>
#include "header.h"

#define THRE1 ((U1LSR >> 5) & 1)
#define RDR1 (U1LSR & 1)

void uart1_init(u32 baud)
{
	s32 pclk,result = 0;
	unsigned int a[] = {15, 60, 30};
	pclk = a[VPBDIV] * 1000000;
	result = pclk / (16 * baud);
	PINSEL0 |= 0X5 << 16;
	U1LCR = 0X83;
	U1DLL = result & 0XFF;
	U1DLM = (result >> 8) & 0XFF;
	U1LCR = 0X03;
}

void uart1_tx(u8 data)
{
	U1THR = data;
	while(THRE1 == 0);
}

u8 uart1_rx(void)
{
	while(RDR1 == 0);
	return U1RBR;
}

void uart1_tx_string(char *s)
{
	while(*s != 0)
	{
		uart1_tx(*s);
		s++;
	}
}
