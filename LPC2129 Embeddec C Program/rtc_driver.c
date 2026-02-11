#include <LPC21xx.H>
#include "header.h"

s8 am_pm_flag;
extern RTC rtc_data;

void rtc_set_time(u8 h,u8 m,u8 s) 
{
	i2c_send(0xD0, 0x0, s); //set secs
	i2c_send(0xD0, 0x1, m); //set mins
	i2c_send(0xD0, 0x2, h); //set hrs
}

void rtc_set_date(u8 date,u8 month,u8 year,u8 dow)
{
	i2c_send(0xD0, 0x3, dow); //set day
	i2c_send(0xD0, 0x4, date); //set date
	i2c_send(0xD0, 0x5, month); //set month
	i2c_send(0xD0, 0x6, year); //set year
}

void rtc_receive_time(void)
{
	rtc_data.s = i2c_receive(0xD1,0x0); //read secs
	rtc_data.m = i2c_receive(0xD1,0x1); //read mins
	rtc_data.h = i2c_receive(0xD1,0x2); //read hrs
	if(((rtc_data.h) >> 6) & 1) 
	{
		if(((rtc_data.h) >> 5) & 1)
		{
			am_pm_flag = 1;
		}
		else
		{
			am_pm_flag = 0;
		}
		rtc_data.h = rtc_data.h & 0X1F;
	}
	else
	{
		am_pm_flag = -1;
	}
}

void rtc_receive_date(void)
{
	rtc_data.dow = i2c_receive(0xD1,0x3); // read day
	rtc_data.date = i2c_receive(0xD1,0x4); //read date
	rtc_data.month = i2c_receive(0xD1,0x5); //read month
	rtc_data.year = i2c_receive(0xD1,0x6); //read year
}

char *day_name(u8 day)
{
	char *dow[] = {"", "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};
	return (day >= 1 && day <= 7) ? dow[day] : "ERR";
}
