/*
 * lcd_module.h
 *
 *  Created on: 2018年12月5日
 *      Author: jeremy.hsiao
 */

#ifndef LCD_MODULE_H_
#define LCD_MODULE_H_

#define WRITE_4BITS

#ifdef _REAL_UPDATEKIT_V2_BOARD_

#define LCD_GPIO_RS_PORT	(0)
#define LCD_GPIO_RS_PIN		(23)
#define	LCD_GPIO_RS_IO_FUNC	(IOCON_FUNC0)
#define LCD_GPIO_RW_PORT	(1)
#define LCD_GPIO_RW_PIN		(21)
#define	LCD_GPIO_RW_IO_FUNC	(IOCON_FUNC0)
#define LCD_GPIO_EN_PORT	(1)
#define LCD_GPIO_EN_PIN		(20)
#define	LCD_GPIO_EN_IO_FUNC	(IOCON_FUNC0)

// Assumed that DB4~7 are at the same GPIO port
#define LCD_DB4_7_PORT		(0)
#define DB4_PIN				(6)
#define	LCD_DB4_PIN_IO_FUNC	(IOCON_FUNC0)
#define DB5_PIN				(7)
#define	LCD_DB5_PIN_IO_FUNC	(IOCON_FUNC0)
#define DB6_PIN				(8)
#define	LCD_DB6_PIN_IO_FUNC	(IOCON_FUNC0)
#define DB7_PIN				(9)
#define	LCD_DB7_PIN_IO_FUNC	(IOCON_FUNC0)
#define HIGH_NIBBLE_MASK	((1L<<DB7_PIN)|(1L<<DB6_PIN)|(1L<<DB5_PIN)|(1L<<DB4_PIN))

#else

#define LCD_GPIO_RS_PORT	(0)
#define LCD_GPIO_RS_PIN		(8)
#define	LCD_GPIO_RS_IO_FUNC	(IOCON_FUNC0)
#define LCD_GPIO_RW_PORT	(0)
#define LCD_GPIO_RW_PIN		(9)
#define	LCD_GPIO_RW_IO_FUNC	(IOCON_FUNC0)
#define LCD_GPIO_EN_PORT	(0)
#define LCD_GPIO_EN_PIN		(2)
#define	LCD_GPIO_EN_IO_FUNC	(IOCON_FUNC0)

// Assumed that DB4~7 are at the same GPIO port
#define LCD_DB4_7_PORT		(1)
#define DB4_PIN				(25)
#define	LCD_DB4_PIN_IO_FUNC	(IOCON_FUNC0)
#define DB5_PIN				(19)
#define	LCD_DB5_PIN_IO_FUNC	(IOCON_FUNC0)
#define DB6_PIN				(24)
#define	LCD_DB6_PIN_IO_FUNC	(IOCON_FUNC0)
#define DB7_PIN				(18)
#define	LCD_DB7_PIN_IO_FUNC	(IOCON_FUNC0)
#define HIGH_NIBBLE_MASK	((1L<<DB7_PIN)|(1L<<DB6_PIN)|(1L<<DB5_PIN)|(1L<<DB4_PIN))

#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

#define LONGER_DELAY_US		1600
#define SHORTER_DELAY_US	400

#define LCM_RS_0	Chip_GPIO_SetPinOutLow(LPC_GPIO, LCD_GPIO_RS_PORT, LCD_GPIO_RS_PIN)
#define LCM_RW_0	Chip_GPIO_SetPinOutLow(LPC_GPIO, LCD_GPIO_RW_PORT, LCD_GPIO_RW_PIN)
#define LCM_EN_0	Chip_GPIO_SetPinOutLow(LPC_GPIO, LCD_GPIO_EN_PORT, LCD_GPIO_EN_PIN)

#define LCM_RS_1	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LCD_GPIO_RS_PORT, LCD_GPIO_RS_PIN)
#define LCM_RW_1	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LCD_GPIO_RW_PORT, LCD_GPIO_RW_PIN)
#define LCM_EN_1	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LCD_GPIO_EN_PORT, LCD_GPIO_EN_PIN)

//extern void lcm_clear(void);
//extern void lcm_puts(uint8_t *s);
//extern void lcm_putch(uint8_t c);
//extern void lcm_goto(uint8_t pos, uint8_t line);
//extern void lcm_clear_display(void);
//extern void lcm_return_home(void);
//extern void lcm_entry_mode(bool ID, bool SH);

extern void lcm_write_ram_data(uint8_t c);
extern uint8_t lcd_read_busy_and_address(void);
extern uint8_t lcd_read_data_from_RAM(void);

extern void lcm_auto_display_init(void);
extern void lcm_auto_display_refresh_task(void);
extern void lcm_force_to_display_page(uint8_t page_no);
#define LCM_AUTO_DISPLAY_SWITCH_PAGE_MS		(5000)

extern void lcm_demo(void);
extern void lcm_content_init(void);
extern void lcm_sw_init(void);
extern void Init_LCD_Module_GPIO(void);

#define	MAX_LCD_CONTENT_PAGE	(4)
#define LCM_DISPLAY_ROW			(2)
#define LCM_DISPLAY_COL			(16)
extern uint8_t lcd_module_display_content[MAX_LCD_CONTENT_PAGE][LCM_DISPLAY_ROW][LCM_DISPLAY_COL];
extern uint8_t lcd_module_display_enable[MAX_LCD_CONTENT_PAGE];

#endif /* LCD_MODULE_H_ */
