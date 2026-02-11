#include <LPC21xx.H>
#include <stdio.h>
#include "header.h"

void lcd_init(void)
{
	IODIR1 = 0XFE << 16;
	lcd_cmd(0X02);
	lcd_cmd(0X03);
	lcd_cmd(0X28);
	lcd_cmd(0X0E);
	lcd_cmd(0X01);
	lcd_cmd(0X0C);
}

void lcd_data(u8 data)
{
	u32 temp;
	//HIGHER NIBBLE LOGIC
	IOCLR1 = 0XFE << 16;
	temp = ((data & 0XF0) << 16);
	IOSET1 = temp;
	IOSET1 = 1 << 17;
	IOCLR1 = 1 << 18;
	IOSET1 = 1 << 19;
	delay_ms(2);
	IOCLR1 = 1 << 19;
	
	//LOWER NIBBLE LOGIC
	IOCLR1 = 0XFE << 16;
	temp = ((data & 0X0F) << 20);
	IOSET1 = temp;
	IOSET1 = 1 << 17;
	IOCLR1 = 1 << 18;
	IOSET1 = 1 << 19;
	delay_ms(2);
	IOCLR1 = 1 << 19;
}

void lcd_cmd(u8 cmd)
{
	u32 temp;
	//HIGHER NIBBLE LOGIC
	IOCLR1 = 0XFE << 16;
	temp = ((cmd & 0XF0) << 16);
	IOSET1 = temp;
	IOCLR1 = 1 << 17;
	IOCLR1 = 1 << 18;
	IOSET1 = 1 << 19;
	delay_ms(2);
	IOCLR1 = 1 << 19;
	
	//LOWER NIBBLE LOGIC
	IOCLR1 = 0XFE << 16;
	temp = ((cmd & 0X0F) << 20);
	IOSET1 = temp;
	IOCLR1 = 1 << 17;
	IOCLR1 = 1 << 18;
	IOSET1 = 1 << 19;
	delay_ms(2);
	IOCLR1 = 1 << 19;
}

void lcd_string(char *s)
{
	while(*s != 0)
	{
		lcd_data(*s);
		s++;
	}
}

void lcd_hex(s32 n)
{
    char buf[5];
    sprintf(buf, "%02X", n);
    lcd_string(buf);
}
