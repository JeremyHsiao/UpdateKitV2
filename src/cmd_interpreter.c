/*
 * cmd_interpreter.c
 *
 *  Created on: 2018年11月27日
 *      Author: Jeremy.Hsiao
 */
#include "chip.h"
#include "string.h"
#include "stdlib.h"
#include "ctype.h"
#include "gpio.h"
#include "uart_0_rb.h"
#include "event.h"
#include "build_defs.h"
#include "fw_version.h"
#include "cmd_interpreter.h"
#include "cdc_main.h"
#include "res_state.h"
#include "sw_timer.h"
#include "user_if.h"
#include "tpic6b595.h"
#include "adc.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
char 			*ptr_str;
bool 			EchoEnabled;
char 			serial_gets_return_string[MAX_SERIAL_GETS_LEN+1];	// Extra one is for '\0'
char 			command_return_string[MAX_SERIAL_GETS_LEN+1];

// internal structure for execution: 24-bit value + 6-bit object + 2-bit cmd
// get obj
// set obj value
// reserved_1
// reserved_2

#define		CmdCodeBitPos		(0)
#define     CmdCodeBitLen		(2)
#define		CmdObjBitPos		(CmdCodeBitPos+CmdCodeBitLen)
#define		CmdOjbBitLen		(6)
#define		CmdValueBitPos		(CmdObjBitPos+CmdOjbBitLen)
#define		CmdValueBitLen		(CmdPacketLen-CmdOjbBitLen-CmdCodeBitLen)
#define		CmdCodeBitMask		( ( ( ((CmdExecutionPacket)1UL) << (CmdCodeBitLen+1) )  - 1)  << CmdCodeBitPos )
#define     CmdOjbBitMask		( ( ( ((CmdExecutionPacket)1UL) << (CmdOjbBitLen+1) )   - 1)  << CmdObjBitPos )
#define		CmdValueBitMask		( ( ( ((CmdExecutionPacket)1UL) << (CmdValueBitLen+1) ) - 1 ) << CmdValueBitPos )

enum
{
	CMD_CODE_NONE_COMMAND = 0,
	CMD_CODE_GET,
	CMD_CODE_SET,
//	CMD_CODE_RESERVED_2,
	CMD_CODE_MAX_NO
};

static const char *command_code_list[CMD_CODE_MAX_NO - 1] =
{
		// not checking non-command --> so the enum number is +1 larger than the sequence of command_code_list
		"get",
		"set",
//		"reserved_2",
};

static const char enter_user_ctrl_mode_str[] = "x&Vht&GD";

enum
{
	CMD_OBJECT_NONE_OBJECT = 0,
	CMD_OBJECT_USERMODE,
	CMD_OBJECT_ECHO,
	CMD_OBJECT_FW_VER,
	CMD_OBJECT_ENTER_ISP,
	CMD_OBJECT_RESISTOR_A,
	CMD_OBJECT_RESISTOR_B,
	CMD_OBJECT_RESISTOR_C,
	CMD_OBJECT_RESISTOR_2N,
	CMD_OBJECT_6B595_SELFTEST,
	CMD_OBJECT_NOP,
	CMD_OBJECT_BUTTON_IO,
	CMD_OBJECT_INPUT_VOLTAGE,
	CMD_OBJECT_DISABLE_RELAY_CONTROL,
	CMD_OBJECT_MAX_NO,
};

static const char *command_object_list[CMD_OBJECT_MAX_NO - 1] =
{
	// not checking non-object--> so the enum number is +1 larger than the sequence of command_object_list
	"user_mode",
	"echo",
	"fw_ver",
	"enter_isp",
	"RA",
	"RB",
	"RC",
	"R2N",
	"6B595_selftest",
	"nop",
	"button_io",
	"input_voltage",
	"disable_relay_control"
//	// JP5 & JP3
//	"PIO2_0",
//	"PIO2_1",
//	"PIO2_5",
//	"PIO0_20",
//	"PIO0_2",
//	"PIO2_2",
//	"PIO0_5",
//	"PIO0_21",
//	"PIO1_23",
//	"PIO2_7",
//	"PIO1_24",
//	"PIO0_22",
//	"PIO0_11",
//	"PIO0_12",
//	"PIO0_13",
//	"PIO0_14",
//	"PIO1_13",
//	"PIO0_16",
//	"PIO0_17",
//	// JP4 - LCD module
//	"PIO0_6",
//	"PIO0_7",
//	"PIO0_8",
//	"PIO0_9",
//	"PIO0_23",
//	"PIO1_20",
//	"PIO1_21",
//	// JP2 - SWD interface & JP3 P0_1
//	"PIO0_0",
//	"PIO0_1",
//	"PIO0_10",
//	"PIO0_15",
//	// Hidden
//	"PIO0_4",
};

#define CMD_DEFINE_PACK_CMD(cmd)				( CmdCodeBitMask  & (((CmdExecutionPacket)cmd)    << CmdCodeBitPos ) )
#define CMD_DEFINE_GET							CMD_DEFINE_PACK_CMD(CMD_CODE_GET)
#define CMD_DEFINE_SET							CMD_DEFINE_PACK_CMD(CMD_CODE_SET)

#define CMD_DEFINE_PACK_OBJ(obj)				( CmdOjbBitMask   & (((CmdExecutionPacket)obj)    << CmdObjBitPos  ) )
#define CMD_GET_OBJECT_VALUE(OBJECT)			( CMD_DEFINE_GET | CMD_DEFINE_PACK_OBJ(OBJECT) )
#define CMD_SET_OBJECT_VALUE(OBJECT)	    	( CMD_DEFINE_SET | CMD_DEFINE_PACK_OBJ(OBJECT) )

#define CMD_DEFINE_PACK_PARAMETER(param)		( CmdValueBitMask & (((CmdExecutionPacket)param) << CmdValueBitPos) )

#define CMD_EXTRACT_CODE_WITH_OBJECT(PACKET)	(PACKET&(~CmdValueBitMask))
#define CMD_EXTRACT_PARAMETER(PACKET)			((PACKET&CmdValueBitMask)>>CmdValueBitPos)

typedef enum {
	DEFAULT_NON_CMD			= 0,
	//  No GET
	SET_USER_MODE 			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_USERMODE),			// Enter/Leave user control mode
	GET_FW_VERSION			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_FW_VER),				// get FW version
	//  No SET
	GET_ECHO 				= CMD_GET_OBJECT_VALUE(CMD_OBJECT_ECHO),				// get echo status
	SET_ECHO 				= CMD_SET_OBJECT_VALUE(CMD_OBJECT_ECHO),				// set echo status
	//  No GET
	SET_ENTER_ISP			= CMD_SET_OBJECT_VALUE(CMD_OBJECT_ENTER_ISP),			// enter isp mode from user code
	GET_RA 					= CMD_GET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_A),			// get resistor A value
	SET_RA	 				= CMD_SET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_A),			// set resistor A value
	GET_RB 					= CMD_GET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_B),			// get resistor B value
	SET_RB	 				= CMD_SET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_B),			// set resistor B value
	GET_RC 					= CMD_GET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_C),			// get resistor C value
	SET_RC	 				= CMD_SET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_C),			// set resistor C value
	GET_R2N 				= CMD_GET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_2N),			// get resistor R2N value
	SET_R2N	 				= CMD_SET_OBJECT_VALUE(CMD_OBJECT_RESISTOR_2N),			// set resistor R2N value
	GET_6B595_SELFTEST		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_6B595_SELFTEST),
	SET_6B595_SELFTEST		= CMD_SET_OBJECT_VALUE(CMD_OBJECT_6B595_SELFTEST),
	GET_NOP					= CMD_GET_OBJECT_VALUE(CMD_OBJECT_NOP),
	SET_NOP					= CMD_SET_OBJECT_VALUE(CMD_OBJECT_NOP),
	GET_BUTTON_IO			= CMD_GET_OBJECT_VALUE(CMD_OBJECT_BUTTON_IO),
	// No SET
	GET_INPUT_VOLTAGE		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_INPUT_VOLTAGE),
	// No SET
	GET_RELAY_CONTROL		= CMD_GET_OBJECT_VALUE(CMD_OBJECT_DISABLE_RELAY_CONTROL),
	SET_RELAY_CONTROL		= CMD_SET_OBJECT_VALUE(CMD_OBJECT_DISABLE_RELAY_CONTROL),
	//
} CMD_LIST;

char *error_parameter  		= "Parameter error.";
char *error_out_of_range  	= "Parameter error.";
char *error_command    		= "Command error.";
char *error_developing 		= "Under development";
char *message_ok       		= "OK";
char *message_On       		= "ON";
char *message_Off      		= "OFF";

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
CmdExecutionPacket		received_cmd_packet;
char					*command_string;

/*****************************************************************************
 * Private functions
 ****************************************************************************/
char *trimwhitespace(char *str)
{
  char *end;

  // Trim leading space
  while(isspace((unsigned char)*str)) str++;

  if(*str == 0)  // All spaces?
    return str;

  // Trim trailing space
  end = str + strlen(str) - 1;
  while(end > str && isspace((unsigned char)*end)) end--;

  // Write new null terminator character
  *++end = '\0';

  return str;
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/
void init_cmd_interpreter(void)
{
	*serial_gets_return_string = '\0';
	ptr_str = serial_gets_return_string;
	EchoEnabled = true; //
	received_cmd_packet = 0;
	command_string = (char *)NULL;
}

void EchoEnable(bool enabled)
{
	EchoEnabled = enabled;
}

bool CheckEchoEnableStatus(void)
{
	return EchoEnabled;
}

char *serial_gets(char input_ch)
{
	char *ret_ptr;

	switch (input_ch)
	{
		case '\r':
		case '\n':
			// Append a '\0' to the end of return string'; then reset ptr_str for next line of command.
			*ptr_str = '\0';
			ptr_str = serial_gets_return_string;
			ret_ptr = serial_gets_return_string;
			break;
		case '\b':
			// remove previous char (if any)
			if(ptr_str>(serial_gets_return_string))
			{
				ptr_str--;
			}
			ret_ptr = (char *)(NULL);
			break;
		case 3:		//ctrl-c
		case 4:		//ctrl-d
		case 26:	//ctrl-z
			// reset ptr_str for next line of command.
			ptr_str = serial_gets_return_string;
			ret_ptr = (char *)(NULL);
			break;
		default:
			// Fill input string buffer until last 1 slot left (for storing end-of-string '\0')
			if(ptr_str<(serial_gets_return_string+MAX_SERIAL_GETS_LEN))
			{
				*ptr_str++ = input_ch;
			}
			ret_ptr = (char *)(NULL);
			break;
	}

	return ret_ptr;
}

bool CheckIfUserCtrlModeCommand(char *input_str)
{
	// OutputString_with_newline(input_str);				 // debug purpose
	// Must be the exact string to enter user control mode
	if (strcmp(trimwhitespace(input_str),enter_user_ctrl_mode_str)==0)
		return true;
	else
		return false;
}

bool CommandInterpreter(char *input_str, CmdExecutionPacket *ptr_packet)
{
	char 				*token = strtok(trimwhitespace(input_str), " ");

	*ptr_packet = 0;			// clear for further OR after command/object/parameter are decoded

	// 1st command
	if(token==NULL)
	{
		return false;
	}
	else
	{
		int	index = CMD_CODE_MAX_NO-1;
		while(index-->0)
		{
			if (strcmp(token,command_code_list[index])==0)
			{
				*ptr_packet = CMD_DEFINE_PACK_CMD(index+1);
				// OutputString_with_newline(token);	// debug purpose
				break;
			}
		}
	}

	// 2nd object
	token = strtok(NULL, " ");
	if(token==NULL)
	{
		return false;
	}
	else
	{
		int	index = CMD_OBJECT_MAX_NO-1;
		while(index-->0)
		{
			if (strcmp(token,command_object_list[index])==0)
			{
				*ptr_packet |= CMD_DEFINE_PACK_OBJ(index+1);
				// OutputString_with_newline(token);	// debug purpose
				break;
			}
		}
	}

	// 3rd parameter
	token = strtok(NULL, " ");
	if(token!=NULL)
	{
		int val;
		val = atoi(token);
		// OutputString_with_newline(token);	// debug purpose
		// OutputHexValue_with_newline(val);	// debug purpose
		*ptr_packet |= CMD_DEFINE_PACK_PARAMETER(val);
	}

	return true;
}

//
// test commands:
//
// x&Vht&GD
// set user_mode 1
// set pwm_percent 100
// set pwm_percent 50
// set pwm_percent 0
// set pwm_output 0
// set pwm_output 1
// get pwm_output
// set pwm_use_table 0
// set pwm_use_table 9
// set pwm_use_table 5
// set user_mode 0
// x&Vht&GD
// get fw_ver

bool CommandExecution(CmdExecutionPacket cmd_packet, char **return_string_ptr)
{
	uint32_t	param = CMD_EXTRACT_PARAMETER(cmd_packet);
	bool		ret_value = false;

	switch(CMD_EXTRACT_CODE_WITH_OBJECT(cmd_packet))
	{
		case SET_USER_MODE:
			if(param!=0)
			{
				EVENT_Enter_User_Ctrl_Mode = true;
			}
			else
			{
				EVENT_Leave_User_Ctrl_Mode = true;
			}
			*return_string_ptr = message_ok;
			ret_value = true;
			break;
		case GET_FW_VERSION:
			{
				const char voltage_output_welcome_message_line2[] =
				{   'V', 'e', 'r', ':', FW_MAJOR, FW_MIDDLE, FW_MINOR, '_', // "Ver:x.x_" - total 8 chars
				   BUILD_MONTH_CH0, BUILD_MONTH_CH1, BUILD_DAY_CH0, BUILD_DAY_CH1, BUILD_HOUR_CH0, BUILD_HOUR_CH1,  BUILD_MIN_CH0, BUILD_MIN_CH1, // 8 chars
					'\0'};

				strcpy(command_return_string, voltage_output_welcome_message_line2);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;
		case GET_ECHO:
			*return_string_ptr = (CheckEchoEnableStatus())?message_On:message_Off;
			ret_value = true;
			break;
		case SET_ECHO:
			EchoEnable((param!=0)?true:false);
			*return_string_ptr = message_ok;
			ret_value = true;
			break;
		case SET_ENTER_ISP:
			DeInit_UART0();
			while((Check_USB_IsConfigured()));
			for(int i=0; i<100000; i++){}
			Chip_IAP_ReinvokeISP();
			*return_string_ptr = message_ok;
			ret_value = true;
			break;
		case GET_RA:
			{
				uint32_t	*resistor_ptr = GetResistorValue();
				itoa_10(resistor_ptr[0], command_return_string);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;
		case GET_RB:
			{
				uint32_t	*resistor_ptr = GetResistorValue();
				itoa_10(resistor_ptr[1], command_return_string);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;
		case GET_RC:
			{
				uint32_t	*resistor_ptr = GetResistorValue();
				itoa_10(resistor_ptr[2], command_return_string);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;
		case SET_RA:
			{
				uint32_t	*resistor_ptr = GetResistorValue();
				if(Check_if_Resistor_in_Range(param))
				{
					resistor_ptr[0] = param;
					*return_string_ptr = message_ok;
				}
				else
				{
					*return_string_ptr = error_out_of_range;
				}
				ret_value = true;
			}
			break;
		case SET_RB:
			{
				uint32_t	*resistor_ptr = GetResistorValue();
				if(Check_if_Resistor_in_Range(param))
				{
					resistor_ptr[1] = param;
					*return_string_ptr = message_ok;
				}
				else
				{
					*return_string_ptr = error_out_of_range;
				}
				ret_value = true;
			}
			break;
		case SET_RC:
			{
				uint32_t	*resistor_ptr = GetResistorValue();
				if(Check_if_Resistor_in_Range(param))
				{
					resistor_ptr[2] = param;
					*return_string_ptr = message_ok;
				}
				else
				{
					*return_string_ptr = error_out_of_range;
				}
				ret_value = true;
			}
			break;

		case GET_NOP:
		case SET_NOP:
			//*return_string_ptr = command_string_usb; // set in main.c
			ret_value = true;
			break;

		case GET_BUTTON_IO:
			{
				strcpy(command_return_string,"button_io:");
				itoa_10(Get_Button_IO_Value(), command_return_string+sizeof("button_io:")-1);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;

		case GET_RELAY_CONTROL:
			itoa_10(get_disable_relay_control(), command_return_string);
			*return_string_ptr = command_return_string;
			ret_value = true;
			break;
		case SET_RELAY_CONTROL:
			if(param)
			{
				set_disable_relay_control(true);
				*return_string_ptr = message_ok;
			}
			else
			{
				set_disable_relay_control(false);
				*return_string_ptr = message_ok;
			}
			ret_value = true;
			break;

		case GET_6B595_SELFTEST:
			{
				int ch_index;
				ch_index = itoa_10(get_tpic6b595_selftest_OK_Count(), command_return_string);
				command_return_string[ch_index++] = '/';
				itoa_10(get_tpic6b595_selftest_Total_Count(), command_return_string+ch_index);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;
		case SET_6B595_SELFTEST:
			if(param)
			{
				set_tpic6b595_selftest_On(true);
				*return_string_ptr = message_ok;
			}
			else
			{
				set_tpic6b595_selftest_On(false);
				*return_string_ptr = message_ok;
			}
			ret_value = true;
			break;

		case GET_INPUT_VOLTAGE:
			{
				strcpy(command_return_string,"input_voltage:");
				itoa_10(adc_voltage, command_return_string+sizeof("input_voltage:")-1);
				*return_string_ptr = command_return_string;
				ret_value = true;
			}
			break;

		case GET_R2N:
		case SET_R2N:
			*return_string_ptr = error_developing;	// To be implemented
			break;
		default:
			// command error
			*return_string_ptr = error_command;
			break;
	}

	return ret_value;
}
