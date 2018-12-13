/*
 * LED_7seg.c
 *
 *  Created on: 2018年11月30日
 *      Author: jeremy.hsiao
 */

#include "board.h"
#include "LED_7seg.h"
#include "sw_timer.h"

uint32_t	gpio_mask[3];
uint8_t		led_7seg_message[4], dp_point, next_refresh_index;

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

uint8_t const LED_7SEG_GPIO_IOFUNC_LUT[] =
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
	0,0,1,1,1,0,0,//u
	0,1,1,1,1,1,0,//U
	1,0,0,1,1,1,1,//E
	0,0,0,1,1,0,1,//c
	0,0,0,1,1,1,0,//L
	1,1,0,0,1,1,1,//P
	0,1,1,1,1,0,0,//J
	0,1,1,1,1,0,1,//d
	0,0,1,1,1,0,1,//o
	1,1,1,1,1,0,1,//a
	0,0,0,0,1,1,0,//l
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
	'u',
	'U',
	'E',
	'c',
	'L',
	'P',
	'J',
	'd',
	'o',
	'a',
	'l',
	' ',
};

void Init_LED_7seg_GPIO(void)
{
	uint8_t const 	*prt_7seg_gpio_lut, *prt_7seg_gpio_iofunc_lut;

	// Init variables
	next_refresh_index=0;
	gpio_mask[0]=gpio_mask[1]=gpio_mask[2]=0;
	led_7seg_message[0]=led_7seg_message[1]=led_7seg_message[2]=led_7seg_message[3]=dp_point=0;

	// Init all LED 7SEG IO port by using table LED_7SEG_GPIO_LUT & LED_7SEG_GPIO_IOFUNC_LUT
	prt_7seg_gpio_lut = LED_7SEG_GPIO_LUT;
	prt_7seg_gpio_iofunc_lut = LED_7SEG_GPIO_IOFUNC_LUT;
	while (prt_7seg_gpio_iofunc_lut<LED_7SEG_GPIO_IOFUNC_LUT+sizeof(LED_7SEG_GPIO_IOFUNC_LUT))
	{
		uint8_t	port_no, pin_no;

		port_no = *prt_7seg_gpio_lut;
		prt_7seg_gpio_lut++;
		pin_no = *prt_7seg_gpio_lut;
		prt_7seg_gpio_lut++;

		// Set as output
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port_no, pin_no);
		gpio_mask[port_no] |= 1L<<(pin_no);

		// Set as gpio
		Chip_IOCON_PinMuxSet(LPC_IOCON, port_no, pin_no, (*prt_7seg_gpio_iofunc_lut));
		prt_7seg_gpio_iofunc_lut++;
	}

    // output to gpio with mask
	LPC_GPIO->MASK[0] = ~gpio_mask[0];
	LPC_GPIO->MPIN[0] = 0;
	LPC_GPIO->MASK[1] = ~gpio_mask[1];
	LPC_GPIO->MPIN[1] = 0;
	LPC_GPIO->MASK[2] = ~gpio_mask[2];
	LPC_GPIO->MPIN[2] = 0;
}

void Update_LED_7SEG_Message_Buffer(uint8_t *msg, uint8_t new_dp_point)
{
	uint8_t	temp_msg_index, temp_char_index, temp_char;

	temp_msg_index = 4;
	while(temp_msg_index-->0)
	{
		temp_char = msg[temp_msg_index];
		led_7seg_message[temp_msg_index] = sizeof(LED_character_index_LUT) - 1; // default char is ' ' if not found
		for(temp_char_index=0;temp_char_index<sizeof(LED_character_index_LUT);temp_char_index++)
		{
			if(temp_char==LED_character_index_LUT[temp_char_index])
			{
				led_7seg_message[temp_msg_index] = temp_char_index;
			}
		}
	}
	if(new_dp_point<=4)
	{
		dp_point = new_dp_point;
	}
	else
	{
		dp_point = 0;
	}
}

void refresh_LED_7SEG_periodic_task(void)
{
	uint32_t 		out_port[3];
	uint8_t			temp_index;
	uint8_t const 	*ptr_char_def_lut, *prt_7seg_gpio_lut;

	// Go to next index
	next_refresh_index--;
	if((next_refresh_index==0)||(next_refresh_index>4))
	{
		next_refresh_index = 4;
	}

	// Init port data & pointer to LUT
	out_port[0] = out_port[1] = out_port[2] = 0;
	ptr_char_def_lut = LED_character_definition_LUT + (led_7seg_message[next_refresh_index-1]*LED_character_definition_LUT_width);
	prt_7seg_gpio_lut = LED_7SEG_GPIO_LUT;

	// Please note that first 7 element of LED_7SEG_GPIO_LUT is LED_a~g
	for(temp_index=0;temp_index<7;temp_index++)
	{
		if(*ptr_char_def_lut++)
		{
			out_port[*prt_7seg_gpio_lut] |= 1L<<(*(prt_7seg_gpio_lut+1));
		}
		prt_7seg_gpio_lut+=2;
	}

	// Please note that next element of LED_7SEG_GPIO_LUT is LED_dp
	if (dp_point==next_refresh_index)
	{
		out_port[*prt_7seg_gpio_lut] |= 1L<<(*(prt_7seg_gpio_lut+1));
	}
	prt_7seg_gpio_lut+=2;

	// Please note that next 4 element of LED_7SEG_GPIO_LUT is LED_1~4
	// The GPIO for the digit to be displayed should be set to low; others is keep as high
	for(temp_index=1; temp_index<=4; temp_index++)
	{
		if(temp_index!=next_refresh_index)
		{
			out_port[*prt_7seg_gpio_lut] |= 1L<<(*(prt_7seg_gpio_lut+1));
		}
		prt_7seg_gpio_lut+=2;
	}

	// output to gpio with mask
	temp_index = 3;
	while(temp_index-->0)
	{
		LPC_GPIO->MASK[temp_index] = ~gpio_mask[temp_index];
		LPC_GPIO->MPIN[temp_index] = out_port[temp_index];
	}
}

void LED_7seg_self_test(void)
{
	uint32_t 		out_port[3];
	uint8_t			temp_io_index, *temp_io_data_ptr;
	uint8_t			test_pin_high_low[12] = { 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}; // a b c d e f g dp 1 2 3 4

	out_port[0]=0;
	out_port[1]=0;
	out_port[2]=0;
	temp_io_index = sizeof(test_pin_high_low);
	temp_io_data_ptr = (uint8_t *) LED_7SEG_GPIO_LUT+(temp_io_index*2);
	do
	{
		temp_io_index--;
		if(test_pin_high_low[temp_io_index]!=0)
		{
			uint8_t		port, pin;

			pin  = *--temp_io_data_ptr;
			port = *--temp_io_data_ptr;
			out_port[port] = (1L<<pin);
		}
		else
		{
			temp_io_data_ptr-=2;
		}
	}
	while(temp_io_index>0);

	// output to gpio with mask
	temp_io_index = 3;
	while(temp_io_index-->0)
	{
		LPC_GPIO->MASK[temp_io_index] = ~gpio_mask[temp_io_index];
		LPC_GPIO->MPIN[temp_io_index] = out_port[temp_io_index];
	}
	SW_delay_cnt += (SYSTICK_PER_SECOND*1);
	SW_delay_timeout=false;
	while(SW_delay_timeout==false);
}
