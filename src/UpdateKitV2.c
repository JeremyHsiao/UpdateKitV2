/*

	UpdateKitV2 main code

 */

#include "chip.h"
#include "board.h"
#include "uart_0_rb.h"
#include "gpio.h"
#include "string_detector.h"
#include "lcd_module.h"
#include "sw_timer.h"
#include "string.h"
#include "UpdateKitV2.h"
#include "build_defs.h"
#include "event.h"
#include "user_opt.h"
#include "fw_version.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint8_t				current_output_stage;
/*
#define			ROW_FOR_VER		((MAX_VER_NO_LEN%LCM_DISPLAY_COL)!=0)?((MAX_VER_NO_LEN/LCM_DISPLAY_COL)+1):(MAX_VER_NO_LEN/LCM_DISPLAY_COL)
uint8_t			lcd_ok_full_info[(ROW_FOR_VER*2+2)*LCM_DISPLAY_COL];
uint32_t		lcd_ok_full_info_str_current_pos;
*/
/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/

UPDATE_STATE	current_system_proc_state;
uint16_t		max_upgrade_time_in_S;
uint8_t			lcm_page_change_duration_in_sec;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

void Init_Value_From_EEPROM(void)
{
	Load_User_Selection(&current_output_stage);
	Load_System_Timeout_v2(current_output_stage,&max_upgrade_time_in_S);
}

void Init_UpdateKitV2_variables(void)
{
	current_system_proc_state = US_SYSTEM_BOOTUP_STATE;
	lcm_page_change_duration_in_sec = DEFAULT_LCM_PAGE_CHANGE_S_WELCOME;
}
