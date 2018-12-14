/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "pwm.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "LED_7seg.h"
#include "adc.h"
#include "string_detector.h"
#include "lcd_module.h"
#include "sw_timer.h"
#include "string.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief
 * @return
 */
uint16_t	voltage = 0;		//  0.0v ~ 10.0v --> 0-100
uint16_t	current = 0;		// .000A ~ .999A --> 0-999
bool		showing_current = false;

void SetDisplayVoltageCurrent(uint16_t voltage_new, uint16_t current_new)
{
	voltage = voltage_new;
	current = current_new;
}

void UpdateKitV2_LED_7_Segment_Task(void)
{
	char 	 temp_str[4+1], final_str[4];		// For storing 0x0 at the end of string by +1
	int 	 temp_str_len;
	uint32_t temp_value;
	uint8_t	 dp_point;

	if(showing_current==false)		// showing voltage // 0.00v ~ 10.0v
	{
		showing_current=true;

		temp_value = voltage;
		if(temp_value>=100)
		{
			temp_value = 100;
		}
		final_str[0] = ' ';	// Manually filled for 0.0v
		final_str[1] = '0';
		temp_str_len = itoa_10(temp_value, temp_str);
		memcpy((void *)&final_str[3-temp_str_len], temp_str, temp_str_len);
		dp_point = 2;
		final_str[3] = 'U';
	}
	else
	{
		showing_current=false;

		temp_value = current;
		if(temp_value>999)
		{
			temp_value = 999;
		}
		temp_str_len = itoa_10(temp_value, temp_str);
		final_str[0] = '0';	// Manually filled for .000A
		final_str[1] = '0';
		memcpy((void *)&final_str[3-temp_str_len], temp_str, temp_str_len);
		dp_point = 1;
		final_str[3] = 'A';
	}
	Update_LED_7SEG_Message_Buffer((uint8_t*)final_str,dp_point);
}
