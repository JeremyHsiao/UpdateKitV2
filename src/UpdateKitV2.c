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
#include "UpdateKitV2.h"

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

void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void)
{
	if(showing_current==false)		// showing voltage // 0.00v ~ 10.0v
	{
		showing_current=true;
	}
	else
	{
		showing_current=false;
	}
}

void UpdateKitV2_LED_7_UpdateDisplayValueAfterADC_Task(void)
{
	char 	 temp_str[4+1], final_str[4];		// For storing 0x0 at the end of string by +1
	int 	 temp_str_len;
	uint32_t temp_value;
	uint8_t	 dp_point;

	if(showing_current==false)		// showing voltage // 0.00v ~ 10.0v
	{
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


uint8_t		current_output_stage = DEFAULT_POWER_OUTPUT_STEP;
//uint8_t		pwm_table[25] = { 100, 77, 76, 75, 74,   73, 72,  71, 70, 65,    64, 63, 54, 53, 52,     34, 33, 32, 31, 5,   4, 3, 2, 1, 0};
uint8_t		pwm_table[9] = { 100, 76, 75,   71,   65,    55,   35,   5,   0};
//Key toggle :				 0V, 6.5V, 7V,  7.5V, 8V,   8.5V, 9V,  9.5V, 10V
						//	0 / 680 / 702 / 749 / 799 / 852 / 909 / 948/ 980

void PowerOutputSetting(uint8_t current_step)
{
	if(current_step==0)
	{
		setPWMRate(0, pwm_table[current_output_stage]);
		Chip_GPIO_SetPinOutLow(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	}
	else
	{
		setPWMRate(0, pwm_table[current_output_stage]);
		Chip_GPIO_SetPinOutHigh(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	}
}

void ButtonPressedTask(void)
{
	if(++current_output_stage>=(sizeof(pwm_table)/sizeof(uint8_t)))
	{
		current_output_stage = 0;

	}

	PowerOutputSetting(current_output_stage);
	// temp for debug purpose
	{
					char temp_str[LCM_DISPLAY_COL+1];
					int  temp_str_len;
					temp_str_len = itoa_10(pwm_table[current_output_stage], temp_str);
					memcpy((void *)&lcd_module_display_content[1][1][9], temp_str, temp_str_len);
					memset((void *)&lcd_module_display_content[1][1][9+temp_str_len], ' ', LCM_DISPLAY_COL-9-temp_str_len);
					lcm_force_to_display_page(1);
	}
}
