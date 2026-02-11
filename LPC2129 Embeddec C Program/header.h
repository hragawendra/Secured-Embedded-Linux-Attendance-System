#ifndef __HEADER_H__
#define __HEADER_H__

typedef unsigned int u32;
typedef signed int s32;
typedef unsigned char u8;
typedef signed char s8;

/* Timer */
extern void delay_sec(u32 sec);
extern void delay_ms(u32 ms);

/* LCD */
extern void lcd_init(void);
extern void lcd_data(u8 data);
extern void lcd_cmd(u8 cmd);
extern void lcd_string(char *s);
extern void lcd_hex(s32 n);

/* UATR0 */
extern void uart0_init(u32 baud);
extern void uart0_tx(u8 data);
extern u8 uart0_rx(void);
extern void uart0_tx_string(char *s);;
extern void uart0_tx_hex(s32 n);

/* UATR1 */
extern void uart1_init(u32 baud);
extern void uart1_tx(u8 data);
extern u8 uart1_rx(void);
extern void uart1_tx_string(char *s);
extern void uart1_tx_hex(s32 n);

/* I2C */
extern void i2c_init(void);
extern void i2c_send(u8 sa, u8 mr, u8 data);
extern u8 i2c_receive(u8 sa, u8 mr);
extern u8 i2c_detect(u8 sa);

/* RTC */
typedef struct RTC_DATA
{
	u8 h;
	u8 m;
	u8 s;
	u8 dow;
	u8 date;
	u8 month;
	u8 year;
}RTC;
extern void rtc_set_time(u8 h,u8 m,u8 s);
extern void rtc_set_date(u8 date,u8 month,u8 year,u8 dow);
extern void rtc_receive_time(void);
extern void rtc_receive_date(void);
extern char *day_name(u8 day);

/* UART0 Interrupts */
extern void config_uart0_intr(void);
extern void UART0_isr(void) __irq;

/* UART1 Interrupts */
extern void config_uart1_intr(void);
extern void UART1_isr(void) __irq;

/* RTC Global variables */
extern s8 am_pm_flag;

/* UART Global variables */
extern u8 intr_flag;
extern u8 rx_state; // 0 = Wait for Card, 1 = Wait for Linux Response
extern u8 response_flag; // Flag set when response is ready
extern u8 auth_flag; // Flag for Linux response
extern char id[13];

#endif
