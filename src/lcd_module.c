///*
// * lcd_module.c
// *
// *  Created on: 2018年12月3日
// *      Author: jeremy.hsiao
// */
//
#include "chip.h"
#include "lcd_module.h"
#include "sw_timer.h"
#include "uart_0_rb.h"
#include "string.h"

uint8_t		lcd_module_display_content[MAX_LCD_CONTENT_PAGE][LCM_DISPLAY_ROW][LCM_DISPLAY_COL];
uint8_t 	lcd_module_display_enable[MAX_LCD_CONTENT_PAGE];
uint32_t	lcd_module_auto_switch_timer;
static uint8_t		lcm_current_page, lcm_current_row, lcm_current_col;

static void inline Delay125ns(void)
{
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
}

static void inline Add_LCM_Delay_Tick(uint32_t delay_us)
{
	SW_delay_cnt += ((delay_us*(SYSTICK_PER_SECOND/1000))/1000);
	SW_delay_timeout=false;
}
static void inline Wait_until_No_More_Delay_Tick(void)
{
	while(SW_delay_timeout==false);
}

//void DelayMS(uint32_t delayms)
//{
//	SW_delay_cnt = (delayms*(SYSTICK_PER_SECOND/1000));
// 	while(SW_delay_cnt!=0);
//}

uint8_t const LCD_LCM_GPIO_LUT[] =
{
	LCD_DB4_7_PORT,  DB4_PIN,
	LCD_DB4_7_PORT,  DB5_PIN,
	LCD_DB4_7_PORT,  DB6_PIN,
	LCD_DB4_7_PORT,  DB7_PIN,
	LCD_GPIO_RW_PORT,  LCD_GPIO_RW_PIN,
	LCD_GPIO_RS_PORT,  LCD_GPIO_RS_PIN,
	LCD_GPIO_EN_PORT,  LCD_GPIO_EN_PIN,
};

uint8_t const LCD_LCM_GPIO_IOFUNC_LUT[] =
{
	LCD_DB4_PIN_IO_FUNC,
	LCD_DB5_PIN_IO_FUNC,
	LCD_DB6_PIN_IO_FUNC,
	LCD_DB7_PIN_IO_FUNC,
	LCD_GPIO_RW_IO_FUNC,
	LCD_GPIO_RS_IO_FUNC,
	LCD_GPIO_EN_IO_FUNC,
};

void lcd_module_db_gpio_as_output(void)
{
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LCD_DB4_7_PORT, DB4_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LCD_DB4_7_PORT, DB5_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LCD_DB4_7_PORT, DB6_PIN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LCD_DB4_7_PORT, DB7_PIN);
}

void lcd_module_db_gpio_as_input(void)
{
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, LCD_DB4_7_PORT, DB4_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, LCD_DB4_7_PORT, DB5_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, LCD_DB4_7_PORT, DB6_PIN);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, LCD_DB4_7_PORT, DB7_PIN);
}

void lcd_gpio_init(void)
{
	uint8_t const 	*prt_gpio_lut, *prt_gpio_iofunc_lut;

	prt_gpio_lut = LCD_LCM_GPIO_LUT;
	prt_gpio_iofunc_lut = LCD_LCM_GPIO_IOFUNC_LUT;
	while (prt_gpio_iofunc_lut<LCD_LCM_GPIO_IOFUNC_LUT+sizeof(LCD_LCM_GPIO_IOFUNC_LUT))
	{
		uint8_t	port_no, pin_no;

		port_no = *prt_gpio_lut;
		prt_gpio_lut++;
		pin_no = *prt_gpio_lut;
		prt_gpio_lut++;

		// Set as output
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port_no, pin_no);
		Chip_GPIO_SetPortOutLow(LPC_GPIO, port_no, pin_no);
		// Set as gpio without pull-up/down/open-drain.
		Chip_IOCON_PinMuxSet(LPC_IOCON, port_no, pin_no, (*prt_gpio_iofunc_lut | IOCON_MODE_INACT ));
		prt_gpio_iofunc_lut++;
	}
}

uint32_t PACK_LCD_4BITS(uint8_t high_nibble)
{
	uint32_t	ret_value = 0;

	if(high_nibble&0x80)
	{
		ret_value |= (1L<<DB7_PIN);
	}

	high_nibble<<=1;
	if(high_nibble&0x80)
	{
		ret_value |= (1L<<DB6_PIN);
	}

	high_nibble<<=1;
	if(high_nibble&0x80)
	{
		ret_value |= (1L<<DB5_PIN);
	}

	high_nibble<<=1;
	if(high_nibble&0x80)
	{
		ret_value |= (1L<<DB4_PIN);
	}

	return ret_value;
}

void lcm_write_4bit(uint8_t high_nibble, bool rs_high)
{
	Chip_GPIO_SetPortMask(LPC_GPIO, LCD_DB4_7_PORT, ~HIGH_NIBBLE_MASK);
	Chip_GPIO_SetMaskedPortValue(LPC_GPIO, LCD_DB4_7_PORT, PACK_LCD_4BITS(high_nibble&0xf0));
//DelayMS(1);

	if(rs_high==false)
	{
		LCM_RS_0;
	}
	else
	{
		LCM_RS_1;
	}
	LCM_RW_0;
	Delay125ns(); // DelayMS(1);
	LCM_EN_1;
	Delay125ns(); Delay125ns(); Delay125ns(); // DelayMS(1);
	LCM_EN_0;
	Delay125ns(); // DelayMS(1); // DelayMS(10);
}

void lcm_write_cmd(uint8_t c)
{
#ifdef WRITE_4BITS
	lcm_write_4bit((c&0xf0), false);			// RS is low
	lcm_write_4bit(((c<<4)&0xf0), false);		// RS is low
#endif // #ifdef WRITE_4BITS
}

void lcm_write_ram_data(uint8_t c)
{
	Wait_until_No_More_Delay_Tick();

#ifdef WRITE_4BITS
	lcm_write_4bit((c&0xf0), true);				// RS is high
	lcm_write_4bit(((c<<4)&0xf0), true);		// RS is high
#endif // #ifdef WRITE_4BITS

	Add_LCM_Delay_Tick(SHORTER_DELAY_US);

}

uint8_t lcm_read_4bit_wo_setting_gpio_input(bool rs_high)
{
	uint32_t	gpio_value;
	uint8_t		return_value = 0;

	if(rs_high==false)
	{
		LCM_RS_0;
	}
	else
	{
		LCM_RS_1;
	}
	LCM_RW_1;
	Delay125ns(); // DelayMS(1);
	LCM_EN_1;
	Delay125ns(); Delay125ns(); Delay125ns(); // DelayMS(1); // DelayMS(10);
	Chip_GPIO_SetPortMask(LPC_GPIO, LCD_DB4_7_PORT, ~HIGH_NIBBLE_MASK);
	gpio_value = Chip_GPIO_GetMaskedPortValue(LPC_GPIO, LCD_DB4_7_PORT);
	LCM_EN_0;
	Delay125ns(); // DelayMS(1); // DelayMS(10);

	// here is also delay
	if(gpio_value&(1L<<DB7_PIN))
	{
		return_value |= 0x80;
	}
	if(gpio_value&(1L<<DB6_PIN))
	{
		return_value |= 0x40;
	}
	if(gpio_value&(1L<<DB5_PIN))
	{
		return_value |= 0x20;
	}
	if(gpio_value&(1L<<DB4_PIN))
	{
		return_value |= 0x10;
	}

	return return_value;
}

uint8_t lcd_read_busy_and_address(void)
{
	uint8_t		read_value, temp_nibble;

    lcd_module_db_gpio_as_input();

#ifdef WRITE_4BITS
    read_value = lcm_read_4bit_wo_setting_gpio_input(false);		// RS low
    temp_nibble = lcm_read_4bit_wo_setting_gpio_input(false);		// RS low
    read_value |= ((temp_nibble>>4)&0x0f);
#endif
	lcd_module_db_gpio_as_output();

	return read_value;
}

bool wait_for_not_busy(uint8_t retry)
{
	uint8_t	retry_cnt = retry;
	bool	b_ret = false;

	do
	{
		Wait_until_No_More_Delay_Tick();
		if((lcd_read_busy_and_address()&0x80)==0x00)
		{
			b_ret = true;
			break;
		}
		else
		{
			Add_LCM_Delay_Tick(SHORTER_DELAY_US);
		}
	}
	while(retry_cnt-->0);

	return b_ret;
}

uint8_t lcd_read_data_from_RAM(void)
{
	uint8_t		read_value, temp_nibble;

	Wait_until_No_More_Delay_Tick();

	lcd_module_db_gpio_as_input();

#ifdef WRITE_4BITS
    read_value = lcm_read_4bit_wo_setting_gpio_input(true);		// RS low
    temp_nibble = lcm_read_4bit_wo_setting_gpio_input(true);	// RS low
    read_value |= ((temp_nibble>>4)&0x0f);
#endif
	lcd_module_db_gpio_as_output();

	Add_LCM_Delay_Tick(SHORTER_DELAY_US);

	return read_value;
}

void lcm_clear_display(void)
{
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(0x01);        // Clear Display
	Add_LCM_Delay_Tick(LONGER_DELAY_US);
}

void lcm_return_home(void)
{
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(0x02);        // Clear Display
	Add_LCM_Delay_Tick(LONGER_DELAY_US);
}

void lcm_entry_mode(bool ID, bool SH)
{
	uint8_t	out_data = 0x04;

	if(ID) out_data|= 0x02;
	if(SH) out_data|= 0x01;
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(out_data);
	Add_LCM_Delay_Tick(SHORTER_DELAY_US);
}

void lcm_display_on_off_control(bool Disply, bool Cursor, bool Blinking)
{
	uint8_t	out_data = 0x08;

	if(Disply) out_data|= 0x04;
	if(Cursor) out_data|= 0x02;
	if(Blinking) out_data|= 0x01;
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(out_data);
	Add_LCM_Delay_Tick(SHORTER_DELAY_US);
}

void lcm_cursor_display_shift(bool SC, bool RL)
{
	uint8_t	out_data = 0x10;

	if(SC) out_data|= 0x08;
	if(RL) out_data|= 0x04;
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(out_data);
	Add_LCM_Delay_Tick(SHORTER_DELAY_US);
}

void lcm_initialize_to_4_bit_mode()
{
	Add_LCM_Delay_Tick(LONGER_DELAY_US);
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(0x33);    // force back to 8-bit modes to make sure initial stage
	Add_LCM_Delay_Tick(LONGER_DELAY_US);
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(0x02);    // send 2x 0x20 (for 4-bit) to make sure entering 4-bit mode
	Add_LCM_Delay_Tick(LONGER_DELAY_US);
	Wait_until_No_More_Delay_Tick();
	lcm_write_cmd(0x28);    // Setup 4 bit mode, 2 lines, 5x8
	Add_LCM_Delay_Tick(LONGER_DELAY_US);
}

void lcm_puts(uint8_t *s)
{
 	while(*s)
 	{
 		lcm_write_ram_data(*s++);
 	}
}
void lcm_putch(uint8_t c)
{
 	lcm_write_ram_data( c );
}

void lcm_goto(uint8_t pos, uint8_t line)		// pos / line
{
	uint8_t	LineTmp;

	Wait_until_No_More_Delay_Tick();

	if( line <2 )
 	{
 		LineTmp = ((( line * 40) + pos) & 0x7F);
 		lcm_write_cmd(0x80+LineTmp);
 	}
 	else if( line ==2 )
 	{
 		LineTmp = ( pos  & 0x7F);
 		lcm_write_cmd(0x94+LineTmp);
 	}
 	else
 	{
 		LineTmp = ( pos  & 0x7F);
 		lcm_write_cmd(0xD4+LineTmp);
 	}

	Add_LCM_Delay_Tick(SHORTER_DELAY_US);
}

void lcm_init(void)
{
	lcd_gpio_init();	// All port are configured as output, set as low

#ifdef WRITE_4BITS
	lcm_initialize_to_4_bit_mode();
#else
	// lcm_write_cmd(0x38);	// Function set	8 bits
#endif

	lcm_clear_display();
	lcm_return_home();
	//lcm_entry_mode(true,false);		// I/D high, SH low
	lcm_display_on_off_control(true,false,false);       // Display ON, Cursor off, Cursor Blink off
	//lcm_cursor_display_shift(true,true);        		// S/C high & R/L high
}

void lcm_auto_display_page_visible(uint8_t page, uint8_t visible)
{
	if(page<MAX_LCD_CONTENT_PAGE)
	{
		lcd_module_display_enable[page] = visible;
	}
}

// return if cursor reaches end of row/col and move back to (0,0)
bool lcm_move_to_next_pos(void)
{
	if(++lcm_current_col<LCM_DISPLAY_COL)
	{
		return false;
	}

	lcm_current_col = 0;
	if(++lcm_current_row<LCM_DISPLAY_ROW)
	{
		wait_for_not_busy(15);
		lcm_goto(0,lcm_current_row);
		return false;
	}

	// end of row/col has been reached.
	lcm_current_row=0;
	wait_for_not_busy(15);
	lcm_goto(0,0);
	return true;
}

void lcm_auto_display_clear_all_page(void)
{
	memset((void *)lcd_module_display_content, ' ', sizeof(lcd_module_display_content));
}

void lcm_auto_disable_all_page(void)
{
	memset((void *)lcd_module_display_enable, 0x00, sizeof(lcd_module_display_enable));
}

void lcm_auto_display_init(void)
{
	lcd_module_auto_switch_timer = 0;
	lcm_current_page = lcm_current_row = lcm_current_col = 0;
	lcm_auto_display_clear_all_page();
	lcm_auto_disable_all_page();
}

void lcm_auto_display_refresh_task(void)
{
	uint8_t	temp;

	// If all pages are disabled, simply return
	for(temp=0; temp < MAX_LCD_CONTENT_PAGE; temp++)
	{
		if(lcd_module_display_enable[temp]!=0x0)
			break;
	}
	if(temp==MAX_LCD_CONTENT_PAGE)
		return;								// If all pages are disabled, simply return

	wait_for_not_busy(15);
	lcm_write_ram_data(lcd_module_display_content[lcm_current_page][lcm_current_row][lcm_current_col]);

	if(lcm_move_to_next_pos()==true)		// end of current page, need to find next visible page
	{
		if(lcd_module_auto_switch_timer_timeout==true)
		{
			lcd_module_auto_switch_timer = SYSTICK_COUNT_VALUE_MS(LCM_AUTO_DISPLAY_SWITCH_PAGE_MS);
			lcd_module_auto_switch_timer_timeout = false;

			// Move to next display-enabled page
			for(temp=0; temp < MAX_LCD_CONTENT_PAGE; temp++)
			{
				if(++lcm_current_page>=MAX_LCD_CONTENT_PAGE)
				{
					lcm_current_page = 0;
				}
				if(lcd_module_display_enable[lcm_current_page]!=0x0)
					break;
			}
		}
	}
}

void lcm_content_init(void)
{
	strcpy((void *)&lcd_module_display_content[0][0][0], "TPV UpdateKit V2");
	strcpy((void *)&lcd_module_display_content[0][1][0], "Elapse: 0000 Sec");
	strcpy((void *)&lcd_module_display_content[1][0][0], "ADC: 1024 / 1024");
	strcpy((void *)&lcd_module_display_content[1][1][0], "PWM Duty:50     ");
	strcpy((void *)&lcd_module_display_content[2][0][0], "Ver:            ");
	strcpy((void *)&lcd_module_display_content[2][1][0], "detecting...    ");
	strcpy((void *)&lcd_module_display_content[3][0][0], "PWR detecting...");
 	strcpy((void *)&lcd_module_display_content[3][1][0], "OK detecting... ");
	memset((void *)lcd_module_display_enable, 0x01, 4);
	//lcd_module_display_enable[3] = 0x00;
}

void lcm_demo(void)
{
	strcpy((void *)&lcd_module_display_content[0][0][0], "TPV Technology  ");
	strcpy((void *)&lcd_module_display_content[0][1][0], "UpdateKit V002  ");
	strcpy((void *)&lcd_module_display_content[1][0][0], "Developed By:   ");
	strcpy((void *)&lcd_module_display_content[1][1][0], "I&D TV-FW-SPP   ");
	strcpy((void *)&lcd_module_display_content[2][0][0], "HW Hsu          ");
	strcpy((void *)&lcd_module_display_content[2][1][0], "Jeremy Hsiao    ");
	strcpy((void *)&lcd_module_display_content[3][0][0], "(C) 2018/12/10  ");
	strcpy((void *)&lcd_module_display_content[3][1][0], "Taipei, Taiwan  ");
	memset((void *)lcd_module_display_enable, 0x01, sizeof(lcd_module_display_enable));
}

void lcm_demo_old(void)
{
	uint8_t 	readback_value, index;
	uint8_t		brand_string[] = "TPV Technology";
	uint8_t		product_string[] = "UpdateKit V002";

	readback_value = lcd_read_busy_and_address();

	lcm_goto(0,0);			// Go back to 0,0 for confirming (pos:0, Line:0)
	lcm_puts(brand_string);
	lcm_goto(0,1);			// Go to Next line (pos:0, Line:1)
    lcm_puts(product_string);

	lcm_goto(0,0);			// Go back to 0,0 for confirming (pos:0, Line:0)
	readback_value = lcd_read_busy_and_address();
	for (index=0; index<sizeof(brand_string-1); index++)
	{
		readback_value = lcd_read_data_from_RAM();
		if (readback_value != brand_string[index])
		{
			OutputHexValue_with_newline(index);
		}
	}
	lcm_goto(0,1);			// Go to Next line (pos:0, Line:1)
	for (index=0; index<sizeof(product_string-1); index++)
	{
		readback_value = lcd_read_data_from_RAM();
		if (readback_value != product_string[index])
		{
			OutputHexValue_with_newline(index);
		}
	}

}
//
//
