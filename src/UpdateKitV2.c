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
uint16_t	voltage = 0;		//  0.00v ~ 9.99v --> 0-999
uint16_t	current = 0;		// .000A ~ .999A --> 0-999
bool		showing_current = false;

void SetDisplayVoltage(uint16_t voltage_new)
{
	voltage = voltage_new;
}

void SetDisplayCurrent(uint16_t current_new)
{
	current = current_new;
}

void UpdateKitV2_LED_7_ToggleDisplayVoltageCurrent(void)
{
	if(showing_current==false)		// showing voltage // 0.00v ~ 0.99v
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

	if(showing_current==false)		// showing voltage // 0.00v ~ 9.99v
	{
		temp_value = voltage;
		if(temp_value>999)
		{
			temp_value = 999;
		}
		final_str[0] = '0';	// Manually filled for 0.00v
		final_str[1] = '0';
		temp_str_len = itoa_10(temp_value, temp_str);
		memcpy((void *)&final_str[3-temp_str_len], temp_str, temp_str_len);
		dp_point = 1;
		final_str[3] = 'U';
	}
	else
	{
		temp_value = current / 10; //  discard  last digit
		if(temp_value>99)		// protection
		{
			temp_value = 99;
		}
		temp_str_len = itoa_10(temp_value, temp_str);
		final_str[0] = '0';	// Manually filled for 0.00A
		final_str[1] = '0';
		memcpy((void *)&final_str[3-temp_str_len], temp_str, temp_str_len);
		dp_point = 1;
		final_str[3] = 'A';
	}
	Update_LED_7SEG_Message_Buffer((uint8_t*)final_str,dp_point);
}


uint8_t		current_output_stage = DEFAULT_POWER_OUTPUT_STEP;
//uint8_t		pwm_table[25] = { 100, 77, 76, 75, 74,   73, 72,  71, 70, 65,    64, 63, 54, 53, 52,     34, 33, 32, 31, 5,   4, 3, 2, 1, 0};
uint8_t		pwm_table[9] = { 100, 49, 41,   33,   26,    19,   12,   4,   0};
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

#define	CURRENT_HISTORY_DATA_SIZE	128
static RINGBUFF_T current_history;
static uint16_t current_history_data[CURRENT_HISTORY_DATA_SIZE];
static uint32_t	total_current_value;

void init_filtered_input_current(void)
{
	total_current_value = 0;
	RingBuffer_Init(&current_history, current_history_data, sizeof(uint16_t), CURRENT_HISTORY_DATA_SIZE);
	RingBuffer_Flush(&current_history);
}

uint16_t Filtered_Input_current(uint16_t latest_current)
{
	uint16_t	temp;

	if(RingBuffer_IsFull(&current_history))
	{
		RingBuffer_Pop(&current_history, &temp);
		total_current_value -= temp;
	}
	total_current_value += latest_current;
	RingBuffer_Insert(&current_history, &latest_current);

	return (total_current_value/CURRENT_HISTORY_DATA_SIZE);
}

#define	VOLTAGE_HISTORY_DATA_SIZE	16
static RINGBUFF_T voltage_history;
static uint16_t voltage_history_data[VOLTAGE_HISTORY_DATA_SIZE];
static uint32_t	total_voltage_value;

void init_filtered_input_voltage(void)
{
	total_voltage_value = 0;
	RingBuffer_Init(&voltage_history, voltage_history_data, sizeof(uint16_t), VOLTAGE_HISTORY_DATA_SIZE);
	RingBuffer_Flush(&voltage_history);
}

uint16_t Filtered_Input_voltage(uint16_t latest_voltage)
{
	uint16_t	temp;

	if(RingBuffer_IsFull(&voltage_history))
	{
		RingBuffer_Pop(&voltage_history, &temp);
		total_voltage_value -= temp;
	}
	total_voltage_value += latest_voltage;
	RingBuffer_Insert(&voltage_history, &latest_voltage);

	return (total_voltage_value/VOLTAGE_HISTORY_DATA_SIZE);
}
