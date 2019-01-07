/*
 * LED_7seg.c
 *
 *  Created on: 2018年11月30日
 *      Author: jeremy.hsiao
 */

#include "board.h"
#include "LED_7seg.h"
#include "sw_timer.h"

uint32_t	gpio_mask[LED_GPIO_NO];
uint8_t		/*led_7seg_message[LED_DISPLAY_COL], dp_point,*/ next_refresh_index;

uint8_t const LED_7SEG_GPIO_LUT[] =
{
	LED_7SEG_SEGa_PORT,  LED_7SEG_SEGa_PIN,
	LED_7SEG_SEGb_PORT,  LED_7SEG_SEGb_PIN,
	LED_7SEG_SEGc_PORT,  LED_7SEG_SEGc_PIN,
	LED_7SEG_SEGd_PORT,  LED_7SEG_SEGd_PIN,
	LED_7SEG_SEGe_PORT,  LED_7SEG_SEGe_PIN,
	LED_7SEG_SEGf_PORT,  LED_7SEG_SEGf_PIN,
	LED_7SEG_SEGg_PORT,  LED_7SEG_SEGg_PIN,
	LED_7SEG_SEGdp_PORT, LED_7SEG_SEGdp_PIN,
	LED_7SEG_SEG1_PORT,  LED_7SEG_SEG1_PIN,
	LED_7SEG_SEG2_PORT,  LED_7SEG_SEG2_PIN,
	LED_7SEG_SEG3_PORT,  LED_7SEG_SEG3_PIN,
	LED_7SEG_SEG4_PORT,  LED_7SEG_SEG4_PIN,
};

uint32_t const LED_7SEG_GPIO_IOFUNC_LUT[] =
{
	LED_7SEG_SEGa_IO_FUNC,
	LED_7SEG_SEGb_IO_FUNC,
	LED_7SEG_SEGc_IO_FUNC,
	LED_7SEG_SEGd_IO_FUNC,
	LED_7SEG_SEGe_IO_FUNC,
	LED_7SEG_SEGf_IO_FUNC,
	LED_7SEG_SEGg_IO_FUNC,
	LED_7SEG_SEGdp_IO_FUNC,
	LED_7SEG_SEG1_IO_FUNC,
	LED_7SEG_SEG2_IO_FUNC,
	LED_7SEG_SEG3_IO_FUNC,
	LED_7SEG_SEG4_IO_FUNC,
};


#define LED_character_definition_LUT_width (7)
uint8_t const LED_character_definition_LUT[] =
{
	1,1,1,1,1,1,0,//0
	0,1,1,0,0,0,0,//1
	1,1,0,1,1,0,1,//2
	1,1,1,1,0,0,1,//3
	0,1,1,0,0,1,1,//4
	1,0,1,1,0,1,1,//5
	1,0,1,1,1,1,1,//6
	1,1,1,0,0,0,0,//7
	1,1,1,1,1,1,1,//8
	1,1,1,1,0,1,1,//9
	1,1,1,0,1,1,1,//A
//	0,0,1,1,1,0,0,//u
	0,1,1,1,1,1,0,//U
//	1,0,0,1,1,1,1,//E
//	0,0,0,1,1,0,1,//c
//	0,0,0,1,1,1,0,//L
//	1,1,0,0,1,1,1,//P
//	0,1,1,1,1,0,0,//J
//	0,1,1,1,1,0,1,//d
//	0,0,1,1,1,0,1,//o
//	1,1,1,1,1,0,1,//a
//	0,0,0,0,1,1,0,//l
	0,0,0,0,0,0,0,//' '
};

uint8_t const LED_character_index_LUT[] =
{
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'A',
//	'u',
	'U',
//	'E',
//	'c',
//	'L',
//	'P',
//	'J',
//	'd',
//	'o',
//	'a',
//	'l',
	' ',
};

// This array stores actual GPIO output value calculated during Init_LED_7seg_GPIO()
uint32_t LED_Char_GPIO_Output_Value_LUT[sizeof(LED_character_index_LUT)*LED_GPIO_NO]; // 3 GPIO values

static inline void LED_Char_GPIO_Output_Value_LUT_Generator(void)
{
	uint32_t 		out_port[LED_GPIO_NO], *ptr_out_port_value;
	uint8_t const 	*ptr_char_def_lut;
	uint8_t			temp_char_index;

	ptr_char_def_lut = LED_character_definition_LUT;
	ptr_out_port_value = LED_Char_GPIO_Output_Value_LUT;

	for(temp_char_index=0;temp_char_index<sizeof(LED_character_index_LUT);temp_char_index++)
	{
		// Init port data
		out_port[0] = out_port[1] = out_port[2] = 0;

		// Please note that first 7 element of LED_7SEG_GPIO_LUT is LED_a~g
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGa_PORT] |= 1L<<(LED_7SEG_SEGa_PIN);}
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGb_PORT] |= 1L<<(LED_7SEG_SEGb_PIN);}
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGc_PORT] |= 1L<<(LED_7SEG_SEGc_PIN);}
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGd_PORT] |= 1L<<(LED_7SEG_SEGd_PIN);}
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGe_PORT] |= 1L<<(LED_7SEG_SEGe_PIN);}
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGf_PORT] |= 1L<<(LED_7SEG_SEGf_PIN);}
		if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGg_PORT] |= 1L<<(LED_7SEG_SEGg_PIN);}

		*ptr_out_port_value++ = out_port[0];
		*ptr_out_port_value++ = out_port[1];
		*ptr_out_port_value++ = out_port[2];
	}
}

void Init_LED_7seg_GPIO(void)
{
	uint8_t const 	*prt_7seg_gpio_lut;
	uint32_t const 	*prt_7seg_gpio_iofunc_lut;

	// Init variables
	next_refresh_index=0;
	gpio_mask[0]=gpio_mask[1]=gpio_mask[2]= ~(0);			// 0 means bits to be written
	//led_7seg_message[0]=led_7seg_message[1]=led_7seg_message[2]=led_7seg_message[3]=dp_point=0;
	led_7SEG_display_enable[LED_VOLTAGE_PAGE] = led_7SEG_display_enable[LED_CURRENT_PAGE] = 1;
	Update_LED_7SEG_Message_Buffer(LED_VOLTAGE_PAGE,(uint8_t*)"000U",1);
	Update_LED_7SEG_Message_Buffer(LED_CURRENT_PAGE,(uint8_t*)"000A",1);

	// Init all LED 7SEG IO port by using table LED_7SEG_GPIO_LUT & LED_7SEG_GPIO_IOFUNC_LUT
	prt_7seg_gpio_lut = LED_7SEG_GPIO_LUT;
	prt_7seg_gpio_iofunc_lut = LED_7SEG_GPIO_IOFUNC_LUT;
	while (prt_7seg_gpio_iofunc_lut<LED_7SEG_GPIO_IOFUNC_LUT+(sizeof(LED_7SEG_GPIO_IOFUNC_LUT)/sizeof(uint32_t)))
	{
		uint8_t	port_no, pin_no;

		port_no = *prt_7seg_gpio_lut;
		prt_7seg_gpio_lut++;
		pin_no = *prt_7seg_gpio_lut;
		prt_7seg_gpio_lut++;

		// Set as output
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port_no, pin_no);
		gpio_mask[port_no] &= ~(1L<<(pin_no));

		// Set as gpio
		Chip_IOCON_PinMuxSet(LPC_IOCON, port_no, pin_no, (*prt_7seg_gpio_iofunc_lut));
		prt_7seg_gpio_iofunc_lut++;
	}

    // output to gpio with mask (note that 0 means that bit is to be written/read
	LPC_GPIO->MASK[0] = gpio_mask[0];
	LPC_GPIO->MASK[1] = gpio_mask[1];
	LPC_GPIO->MASK[2] = gpio_mask[2];
	LPC_GPIO->MPIN[0] = LPC_GPIO->MPIN[1] = LPC_GPIO->MPIN[2] = 0;

	LED_Char_GPIO_Output_Value_LUT_Generator();
}

void Update_LED_7SEG_Message_Buffer(uint8_t page, uint8_t *msg, uint8_t new_dp_point)
{
	uint8_t	temp_msg_index, temp_char_index, temp_char;

	temp_msg_index = LED_DISPLAY_COL;
	while(temp_msg_index-->0)
	{
		temp_char = msg[temp_msg_index];
		led_7SEG_display_content[page][temp_msg_index] = sizeof(LED_character_index_LUT) - 1; // default char is ' ' if not found
		for(temp_char_index=0;temp_char_index<sizeof(LED_character_index_LUT);temp_char_index++)
		{
			if(temp_char==LED_character_index_LUT[temp_char_index])
			{
				led_7SEG_display_content[page][temp_msg_index] = temp_char_index;
				break;
			}
		}
	}
	if(new_dp_point<=LED_DISPLAY_COL)
	{
		led_7SEG_display_dp[page] = new_dp_point;
	}
	else
	{
		led_7SEG_display_dp[page]  = 0;
	}
}

bool LED_7SEG_ForceToSpecificPage(uint8_t page)
{
	bool 			bRet;

	if(page>=LED_DISPLAY_PAGE)
	{
			page = 0;
	}
	if(led_7SEG_display_enable[page]!=0x0)
	{
		next_refresh_index = (page * LED_DISPLAY_COL);
		bRet = true;
	}
	else
	{
		bRet = false;
	}

	return bRet;
}

bool LED_7SEG_GoToNextVisiblePage(void)
{
	uint8_t			page;
	uint8_t 		temp;
	bool 			bRet;
	// Get next page
	page =  (next_refresh_index / LED_DISPLAY_COL);
	bRet = false;
	temp = LED_DISPLAY_PAGE;
	do
	{
		if(++page>=LED_DISPLAY_PAGE)
		{
			page = 0;
		}
		if(led_7SEG_display_enable[page]!=0x0)
		{
			next_refresh_index = (page * LED_DISPLAY_COL);
			bRet = true;
		}
	}
	while((bRet==false)&&(--temp>0));

	return bRet;
}

uint8_t led_7SEG_display_content[LED_DISPLAY_PAGE][LED_DISPLAY_COL];
uint8_t led_7SEG_display_enable[LED_DISPLAY_PAGE];
uint8_t led_7SEG_display_dp[LED_DISPLAY_PAGE];			// 1-4, 0 means no dp

void refresh_LED_7SEG_periodic_task(void)
{
	uint32_t 		out_port[LED_GPIO_NO], *ptr_port_value;
	//uint8_t const 	*ptr_char_def_lut;
	uint8_t			page, col, display_char;

	// Get current page & current col
	page =  next_refresh_index / LED_DISPLAY_COL;
//	col =  next_refresh_index % LED_DISPLAY_COL;
//	display_char = sizeof(LED_character_index_LUT) - 1; // default char is ' ' if not found,

	//Checking whether current page has been disabled
	if(led_7SEG_display_enable[page]==0)
	{
		if(LED_7SEG_GoToNextVisiblePage())
		{
			page =  next_refresh_index / LED_DISPLAY_COL;
			col =  next_refresh_index % LED_DISPLAY_COL;
			display_char = led_7SEG_display_content[page][col];
		}
		else
		{
			display_char = sizeof(LED_character_index_LUT) - 1; // default char is ' ' if not found,
		}
	}
	else
	{
		col =  next_refresh_index % LED_DISPLAY_COL;
		display_char = led_7SEG_display_content[page][col];
	}

	// Init port data & pointer to LUT
//	out_port[0] = out_port[1] = out_port[2] = 0;
//	ptr_char_def_lut = LED_character_definition_LUT + (display_char*LED_character_definition_LUT_width);
//
//	// Please note that first 7 element of LED_7SEG_GPIO_LUT is LED_a~g
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGa_PORT] |= 1L<<(LED_7SEG_SEGa_PIN);}
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGb_PORT] |= 1L<<(LED_7SEG_SEGb_PIN);}
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGc_PORT] |= 1L<<(LED_7SEG_SEGc_PIN);}
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGd_PORT] |= 1L<<(LED_7SEG_SEGd_PIN);}
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGe_PORT] |= 1L<<(LED_7SEG_SEGe_PIN);}
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGf_PORT] |= 1L<<(LED_7SEG_SEGf_PIN);}
//	if(*ptr_char_def_lut++) {out_port[LED_7SEG_SEGg_PORT] |= 1L<<(LED_7SEG_SEGg_PIN);}
	ptr_port_value = LED_Char_GPIO_Output_Value_LUT+(display_char*LED_GPIO_NO);
	out_port[0] = *ptr_port_value++;
	out_port[1] = *ptr_port_value++;
	out_port[2] = *ptr_port_value++;

	col++; // In processing LED position, column is 1-4 instead of 0-3
	// Please note that next element of LED_7SEG_GPIO_LUT is LED_dp
	if (led_7SEG_display_dp[page]==col)
	{
		out_port[LED_7SEG_SEGdp_PORT] |= 1L<<(LED_7SEG_SEGdp_PIN);
	}

	// Please note that next 4 element of LED_7SEG_GPIO_LUT is LED_1~4
	// The GPIO for the digit to be displayed should be set to low; others is keep as high
	if(1!=col) {out_port[LED_7SEG_SEG1_PORT] |= 1L<<(LED_7SEG_SEG1_PIN);}
	if(2!=col) {out_port[LED_7SEG_SEG2_PORT] |= 1L<<(LED_7SEG_SEG2_PIN);}
	if(3!=col) {out_port[LED_7SEG_SEG3_PORT] |= 1L<<(LED_7SEG_SEG3_PIN);}
	if(4!=col) {out_port[LED_7SEG_SEG4_PORT] |= 1L<<(LED_7SEG_SEG4_PIN);}

	LPC_GPIO->MASK[0] = gpio_mask[0];
	LPC_GPIO->MPIN[0] = out_port[0];
	LPC_GPIO->MASK[1] = gpio_mask[1];
	LPC_GPIO->MPIN[1] = out_port[1];
	LPC_GPIO->MASK[2] = gpio_mask[2];
	LPC_GPIO->MPIN[2] = out_port[2];

	// Check if restart from 1st col of the same page
	// col has been added by 1 before
	if(col>=LED_DISPLAY_COL)
	{
		next_refresh_index = (page * LED_DISPLAY_COL);
	}
	else
	{
		next_refresh_index++;
	}

}

//void LED_7seg_self_test(void)
//{
//	uint32_t 		out_port[LED_GPIO_NO];
//	uint8_t			temp_io_index, *temp_io_data_ptr;
//	uint8_t			test_pin_high_low[12] = { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}; // a b c d e f g dp 1 2 3 4
//
//	out_port[0]=0;
//	out_port[1]=0;
//	out_port[2]=0;
//	temp_io_index = sizeof(test_pin_high_low);
//	temp_io_data_ptr = (uint8_t *) LED_7SEG_GPIO_LUT+(temp_io_index*2);
//	do
//	{
//		temp_io_index--;
//		if(test_pin_high_low[temp_io_index]!=0)
//		{
//			uint8_t		port, pin;
//
//			pin  = *--temp_io_data_ptr;
//			port = *--temp_io_data_ptr;
//			out_port[port] |= (1L<<pin);
//		}
//		else
//		{
//			temp_io_data_ptr-=2;
//		}
//	}
//	while(temp_io_index>0);
//
//	// output to gpio with mask
//	temp_io_index = LED_GPIO_NO;
//	while(temp_io_index-->0)
//	{
//		LPC_GPIO->MASK[temp_io_index] = ~gpio_mask[temp_io_index];
//		LPC_GPIO->MPIN[temp_io_index] = out_port[temp_io_index];
//	}
//	SW_delay_sys_tick_cnt += (SYSTICK_PER_SECOND*1);
//	SW_delay_timeout=false;
//	while(SW_delay_timeout==false);
//}
