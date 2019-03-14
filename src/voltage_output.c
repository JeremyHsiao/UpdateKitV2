/*
 * voltage_output.c
 *
 *  Created on: 2019年3月14日
 *      Author: Jeremy.Hsiao
 */

#include "chip.h"
#include "board.h"
#include "lcd_module.h"
#include "uart_0_rb.h"
#include "build_defs.h"
#include "fw_version.h"
#include "UpdateKitV2.h"
#include "voltage_output.h"


//	LCM_PWM_WELCOME,
//	LCM_PWM_OUT_ON,
//	LCM_PWM_OUT_OFF,
//	LCM_PWM_USER_CTRL,
//	LCM_MAX_PAGE_NO

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

void lcm_content_init_for_voltage_output(void)
{
	char const		*welcome_message_line1 =  " Voltage Output ";
	const uint8_t	welcome_message_line2[] =
	{   'V', 'e', 'r', ':', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "FW:Vx.x-" - total 8 chars
	   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
		'\0'};

	// Prepare firmware version for welcome page

	// Welcome page														 1234567890123456
//	memcpy((void *)&lcd_module_display_content[LCM_WELCOME_PAGE][0][0], welcome_message_line1, LCM_DISPLAY_COL);
//	memcpy((void *)&lcd_module_display_content[LCM_WELCOME_PAGE][1][0], welcome_message_line2, LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_PWM_WELCOME,0,0,welcome_message_line1,LCM_DISPLAY_COL);
	lcm_text_buffer_cpy(LCM_PWM_WELCOME,1,0,welcome_message_line2,LCM_DISPLAY_COL);

	// PC Mode page		     										1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_PC_MODE][0][0], "PC Mode: Press  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_PC_MODE][1][0], "button to change", LCM_DISPLAY_COL);

	// FW Upgrade mode reminder page											   0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][0][0], "TV Output:  0.0V", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_REMINDER_BEFORE_OUTPUT][1][0], "Starts in 5 Sec.", LCM_DISPLAY_COL);

	// FW Upgrading page						  					          1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][0][0], "Upgrade:   0 Sec", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADING_PAGE][1][0], "OUT: 0.00V 0.00A", LCM_DISPLAY_COL);

	// FW Upgrade too long page						  					             0123456789012345
	memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADE_TOO_LONG_PAGE][0][0], "Upgrade:   0 Sec", LCM_DISPLAY_COL);
    memcpy((void *)&lcd_module_display_content[LCM_FW_UPGRADE_TOO_LONG_PAGE][1][0], " ** Timeout **  ", LCM_DISPLAY_COL);

	// FW upgrade is done and show software version page				   1234567890123456
    lcm_reset_FW_VER_Content();

	// FW upgrade info of previous update                				   1234567890123456
    lcm_reset_Previous_FW_VER_Content();

	// TV in standby page		     										   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][0][0], "TV is in Standby", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_TV_IN_STANDBY_PAGE][1][0], "Pls power on TV ", LCM_DISPLAY_COL);

	// TV is entering ISP mode page		     							   1234567890123456
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][0][0], "Enter ISP mode  ", LCM_DISPLAY_COL);
	memcpy((void *)&lcd_module_display_content[LCM_ENTER_ISP_PAGE][1][0], "Off-On after ISP", LCM_DISPLAY_COL);
/*
	// lCM_FULL_INFO_FW_OK_PAGE
	lcm_reset_FW_OK_FULL_INFO_Content();
*/
	// enable/disable some page/
	memset((void *)lcd_module_display_enable, 0x00, LCM_MAX_PAGE_NO);	// Initial only - later sw determine which page is to be displayed

//    memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][0][0], 		welcome_message_line2, LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_TITLE_PAGE][1][0], 		"Elapse: 0000 Sec", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][0][0], 	"OUT: 0.00V 0.00A", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_MEASURE_PAGE][1][0], 	"PWM Duty:100    ", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][0][0], "Ver:            ", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_UPGRADE_VER_PAGE][1][0], "detecting...    ", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][0][0], 	"PWR detecting...", LCM_DISPLAY_COL);
//    memcpy((void *)&lcd_module_display_content[LCM_DEV_OK_DETECT_PAGE][1][0], 	"OK detecting... ", LCM_DISPLAY_COL);
    //													                      	 1234567890123456
    //memset((void *)lcd_module_display_enable+(uint8_t)LCM_DEV_TITLE_PAGE, 0x01, 4);


}

