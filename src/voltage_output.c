/*
 * voltage_output.c
 *
 *  Created on: 2019年3月14日
 *      Author: Jeremy.Hsiao
 */

#include "chip.h"
#include "board.h"
#include "string.h"
#include "lcd_module.h"
#include "uart_0_rb.h"
#include "build_defs.h"
#include "fw_version.h"
#include "UpdateKitV2.h"
#include "voltage_output.h" // For voltage output branch

//	// For voltage output branch
//	US_PWM_WELCOME,
//	US_PWM_CHECK_SEL,
//	US_PWM_OUT_ON,
//	US_PWM_OUT_OFF,
//	US_PWM_USER_CTRL,
//	// For voltage output branch	-- END

void OutputVoltageCurrentViaUART_Task(void)
{
    uint8_t temp = 16;
    uint8_t *ptr = lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1];

    do
    {
    	UART0_PutChar(*ptr++);
    }
    while(--temp>0);
    UART0_PutChar('\n');
    UART0_PutChar('\r');
}

//	LCM_PWM_WELCOME,
//	LCM_PWM_OUT_ON,
//	LCM_PWM_OUT_OFF,
//	LCM_PWM_USER_CTRL,
//	LCM_MAX_PAGE_NO


void lcm_content_init_for_voltage_output(void)
{
	const uint8_t	welcome_message_line2[] =
	{   'V', 'e', 'r', ':', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "Ver:x.x_" - total 8 chars
	   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
		'\0'};

	// Prepare firmware version for welcome page

	// PWM Welcome page	                                                1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_WELCOME][0][0], " Voltage Output ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_WELCOME][1][0], welcome_message_line2, LCM_DISPLAY_COL);

	// PWM Output ON page		     								   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_ON][0][0], "PWM Duty is    %", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_ON][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// PWM Output OFF page		     							        1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_OFF][0][0], "PWM is disabled ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_OUT_OFF][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// PWM user control mdoe page										  0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_PWM_USER_CTRL][0][0], "TV Output:  0.0V", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PWM_USER_CTRL][1][0], "Starts in 5 Sec.", LCM_DISPLAY_COL);
}

