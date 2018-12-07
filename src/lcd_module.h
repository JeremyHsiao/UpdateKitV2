/*
 * lcd_module.h
 *
 *  Created on: 2018年12月5日
 *      Author: jeremy.hsiao
 */

#ifndef LCD_MODULE_H_
#define LCD_MODULE_H_

#define WRITE_4BITS

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

#define LCM_RS_0	Chip_GPIO_SetPinOutLow(LPC_GPIO, LCD_GPIO_RS_PORT, LCD_GPIO_RS_PIN)
#define LCM_RW_0	Chip_GPIO_SetPinOutLow(LPC_GPIO, LCD_GPIO_RW_PORT, LCD_GPIO_RW_PIN)
#define LCM_EN_0	Chip_GPIO_SetPinOutLow(LPC_GPIO, LCD_GPIO_EN_PORT, LCD_GPIO_EN_PIN)

#define LCM_RS_1	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LCD_GPIO_RS_PORT, LCD_GPIO_RS_PIN)
#define LCM_RW_1	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LCD_GPIO_RW_PORT, LCD_GPIO_RW_PIN)
#define LCM_EN_1	Chip_GPIO_SetPinOutHigh(LPC_GPIO, LCD_GPIO_EN_PORT, LCD_GPIO_EN_PIN)

extern void lcm_clear(void);
extern void lcm_puts(uint8_t *s);
extern void lcm_putch(uint8_t c);
extern void lcm_goto(uint8_t pos, uint8_t line);
extern void lcm_init(void);
extern void lcm_clear_display(void);
extern void lcm_return_home(void);
extern void lcm_entry_mode(bool ID, bool SH);

extern void lcm_write_ram_data(uint8_t c);
extern uint8_t lcd_read_busy_and_address(void);
extern uint8_t lcd_read_data_from_RAM(void);

#endif /* LCD_MODULE_H_ */
