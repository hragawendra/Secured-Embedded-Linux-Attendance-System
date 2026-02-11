#include <LPC21xx.H>
#include <stdio.h>
#include "header.h"

#define LED (7 << 17)
#define BUZZER (1 << 21)

RTC rtc_data;

int main()
{
	s32 timeout = 0;
	char buff[50];

	lcd_init();
	i2c_init();
	uart0_init(9600);
	uart1_init(9600);
	config_uart0_intr();
	config_uart1_intr();

	rtc_set_time(0X49, 0X00, 0X00);
	rtc_set_date(0X10, 0X02, 0X26, 0X07);

	lcd_string("System Init...");
	delay_ms(200);
	lcd_cmd(0X01);

	rx_state = 0; 

	while(1)
	{
		if(rx_state == 0)
		{
			rtc_receive_time();
			rtc_receive_date();
			lcd_cmd(0X0C);
			lcd_cmd(0X80);
			lcd_hex(rtc_data.h);
			lcd_data(':');
			lcd_hex(rtc_data.m);
			lcd_data(':');
			lcd_hex(rtc_data.s);
			if(am_pm_flag == 0)
			{
				lcd_string(" AM");
			}
			else if(am_pm_flag == 1)
			{
				lcd_string(" PM");
			}
			lcd_cmd(0XC0);
			lcd_string("Scan your ID   ");
		}

		// EVENT 1: CARD SCANNED
		if(intr_flag == 1)
		{
			rtc_receive_time();
			rtc_receive_date();
			lcd_cmd(0X01);
			lcd_cmd(0X80);
			lcd_string("Processing...");

			// 1. Send Data to Linux
			sprintf(buff,"%s,%02X/%02X/%02X,%s,%02X:%02X:%02X,%s\n", id, rtc_data.date, rtc_data.month, rtc_data.year, day_name(rtc_data.dow), rtc_data.h, rtc_data.m, rtc_data.s, (am_pm_flag == 1) ? "PM" : "AM");
			uart0_tx_string(buff);

			// 2. Switch ISR to "Response Mode"
			intr_flag = 0;      // Clear card flag
			response_flag = 0;  // Clear response flag
			rx_state = 1;       // Tell ISR to put next data into rx_response
			timeout = 0;        // Reset timeout counter
		}

		// EVENT 2: WAITING FOR LINUX RESPONSE
		if(rx_state == 1)
		{
			lcd_cmd(0X01);
			lcd_cmd(0X80);
			lcd_string("Checking DB... "); // Display "Wait" animation or just keep time running
			delay_ms(10); // Simple Timeout Logic (Approx 1 seconds) Each loop iteration takes some ms, so we count up
			timeout++;
			
			if(timeout > 100) // ~1 Seconds timeout just to see output in proteus use 100 for 1 sec
			{
				rx_state = 0; // Force back to Card mode
				lcd_cmd(0X01);
				lcd_cmd(0X80);
				lcd_string("Server Timeout");
				delay_ms(500);
				lcd_cmd(0X01);
			}
		}

		// EVENT 3: RESPONSE RECEIVED FROM LINUX
		if(response_flag == 1)
		{
			if(auth_flag == '1') // Compare auth_flag
			{
				lcd_cmd(0X01);
				lcd_cmd(0X80);
				lcd_string("Access Granted");
				delay_ms(500);
			}
			else if(auth_flag == '0')
			{
				// Blink LEDs
				IODIR0 = LED;
				IOSET0 = LED; 
				lcd_cmd(0X01);
				IOCLR0 = LED;
				lcd_cmd(0X80);
				lcd_string("Access Denied");
				lcd_cmd(0XC0);
				lcd_string("Invalid ID");
				delay_ms(500);
				IOSET0 = LED;
			}
			else if(auth_flag == 'M')
			{
				lcd_cmd(0X01);
				lcd_cmd(0X80);
				lcd_string("Master ID Tapped");
				lcd_cmd(0XC0);
				lcd_string("Master Mode EN");
				delay_ms(500);
			}
			else if(auth_flag == 'A')
			{
				lcd_cmd(0X01);
				lcd_cmd(0X80);
				lcd_string("Master Mode");
				lcd_cmd(0XC0);
				lcd_string("Adding New EMP");
				delay_ms(500);
			}
			else if(auth_flag == 'R')
			{
				lcd_cmd(0X01);
				lcd_cmd(0X80);
				lcd_string("Master Mode");
				lcd_cmd(0XC0);
				lcd_string("Removing EMP");
				delay_ms(500);
			}
			else if(auth_flag == '!')
			{
				IODIR0 = BUZZER;
				lcd_cmd(0X01);
				IOSET0 = BUZZER;
				lcd_cmd(0X80);
				lcd_string("Sytem Locked");
				lcd_cmd(0XC0);
				lcd_string("Contact Admin");
				delay_ms(500);
				IOCLR0 = BUZZER;
			}
			
			response_flag = 0;
			rx_state = 0; // Ensure we are back to card mode
			lcd_cmd(0X01);
		}
	}
}
