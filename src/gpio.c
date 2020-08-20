/*
 * gpio.c
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#include "chip.h"
#include "board.h"
#include "sw_timer.h"
#include "gpio.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/
uint32_t LED_G_toggle_cnt, LED_R_toggle_cnt, LED_Y_toggle_cnt;

#define _PORT_NO_	(3)
// Direction Output is 1
uint32_t GPIO_Pin_Mode = 0;
uint32_t GPIO_pin_mask[_PORT_NO_] = {
		// Available pin of GPIO_P0
		((1UL<<4) | (1UL<<5) | (1UL<<11) | (1UL<<12) |
		 (1UL<<13) | (1UL<<21) | (1UL<<22)),
		// Available pin of GPIO_P1
		((1UL<<13) | (1UL<<23) | (1UL<<24)),
		// Available pin of GPIO_P2
		0,
};

const uint32_t GPIO_pin_mask_by_mode[][_PORT_NO_] = {
	// HotSpring board mode
	{
		// Available pin of GPIO_P0
		((1UL<<4) | (1UL<<5) | (1UL<<11) | (1UL<<12) |
		 (1UL<<13) | (1UL<<21) | (1UL<<22)),
		// Available pin of GPIO_P1
		((1UL<<13) | (1UL<<23) | (1UL<<24)),
		// Available pin of GPIO_P2
		0
	},
	// All GPIO mode -- HotSpring board is still operating, which means I/O is still in use
	{
		// Available pin of GPIO_P0
		((1UL<<0) | (1UL<<1) | (1UL<<2) | (1UL<<4) | (1UL<<5) | (1UL<<6) | (1UL<<7) | (1UL<<8) | (1UL<<9) | (1UL<<10) | (1UL<<11) | (1UL<<12) |
		 (1UL<<13)| (1UL<<14) | (1UL<<15) | (1UL<<16) | (1UL<<17) | (1UL<<20) | (1UL<<21) | (1UL<<22) | (1UL<<23)),
		// Available pin of GPIO_P1
		((1UL<<13) | (1UL<<20) |(1UL<<21) | (1UL<<23) | (1UL<<24)),
		// Available pin of GPIO_P2
		((1UL<<0) | (1UL<<1) | (1UL<<2) | (1UL<<5) | (1UL<<7))
	},
	// All GPIO mode -- all I/O is free to use
	{
		// Available pin of GPIO_P0
		((1UL<<0) | (1UL<<1) | (1UL<<2) | (1UL<<4) | (1UL<<5) | (1UL<<6) | (1UL<<7) | (1UL<<8) | (1UL<<9) | (1UL<<10) | (1UL<<11) | (1UL<<12) |
		 (1UL<<13)| (1UL<<14) | (1UL<<15) | (1UL<<16) | (1UL<<17) | (1UL<<20) | (1UL<<21) | (1UL<<22) | (1UL<<23)),
		// Available pin of GPIO_P1
		((1UL<<13) | (1UL<<20) |(1UL<<21) | (1UL<<23) | (1UL<<24)),
		// Available pin of GPIO_P2
		((1UL<<0) | (1UL<<1) | (1UL<<2) | (1UL<<5) | (1UL<<7))
	}
};

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
//bool GPIOGoup0_Int;

/*****************************************************************************
 * Private functions
 ****************************************************************************/

/*****************************************************************************
 * Public functions
 ****************************************************************************/

/**
 * @brief	Handle Group GPIO 0 interrupt
 * @return	Nothing
 */
//void GINT0_IRQHandler(void)
//{
//	Chip_GPIOGP_ClearIntStatus(LPC_GPIOGROUP, 0);
//	//Board_LED_Set(0, true);
//	GPIOGoup0_Int = true;
//}
static inline void GPIO_Command_Update_Mux_and_Direction(uint8_t port, uint32_t new_dir_value)
{
	const uint32_t mux_lut[]=
	{
		FUNC0_MUX_IN,
		FUNC0_MUX_OUT,
		FUNC1_MUX_IN,
		FUNC1_MUX_OUT,
		P0_4_5_MUX_IN,
		P0_4_5_MUX_OUT,
	};
	uint8_t 	pin, mux_index;
	uint32_t	mask_value, out_pin, in_pin;

	mask_value = GPIO_pin_mask[port] & GPIO_pin_mask_by_mode[GPIO_Pin_Mode][port];
	out_pin =  new_dir_value & mask_value;
	Chip_GPIO_SetPortDIROutput(LPC_GPIO,port,out_pin);
	in_pin  = ~new_dir_value & mask_value;
	Chip_GPIO_SetPortDIRInput(LPC_GPIO,port,in_pin);

	pin = 31;
	do
	{
		if(mask_value&(1UL<<31)) // only enabled bit are set.
		{
			mux_index = 0;			// Default is FUNC0 for GPIO function
			if(port==0)	// special mux_value port/pin
			{
				if((pin==4)||(pin==5))
				{
					mux_index = 2*2;	// P0_4 & P0_5
				}
				else if ((pin==0)||((pin>=10)&&(pin<=15)))
				{
					mux_index = 1*2;	// FUNC1 is GPIO function
				}
			}

			if(new_dir_value&(1UL<<31))	// if output?
			{
				++mux_index;
				Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, mux_lut[mux_index]);
			}
			else
			{
				Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, mux_lut[mux_index]);
			}
		}
		mask_value<<=1;
		new_dir_value<<=1;
	}
	while(pin-->0);
}

void Init_GPIO(void)
{
#if defined(_REAL_UPDATEKIT_V2_BOARD_)
	// Set PIO0_3
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 3, (IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7)));				// P0.0-3; 6-10; 17-21);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 3);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, 3);

#endif // defined(_REAL_UPDATEKIT_V2_BOARD_)

#if defined(_REAL_UPDATEKIT_V2_BOARD_) || defined (_HOT_SPRING_BOARD_V2_)

	// This is for input button - original
	Chip_IOCON_PinMuxSet(LPC_IOCON, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN, SWITCH_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN);
	//Chip_GPIO_SetPinDIRInput(LPC_GPIO, ISP_KEY_GPIO_PORT, ISP_KEY_GPIO_PIN);

	// 2nd button - for voltage output selection branch
	Chip_IOCON_PinMuxSet(LPC_IOCON, SECOND_KEY_GPIO_PORT, SECOND_KEY_GPIO_PIN, SECOND_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SECOND_KEY_GPIO_PORT, SECOND_KEY_GPIO_PIN);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GROUP0INT);
	Chip_GPIOGP_SelectLowLevel(LPC_GPIOGROUP, 0, SWITCH_KEY_GPIO_PORT, (1 << SWITCH_KEY_GPIO_PIN) );
	// Chip_GPIOGP_SelectLowLevel(LPC_GPIOGROUP, 0, UP_SWITCH_PORT, 1 << UP_SWITCH_BIT); // Use this if different port is used in the same group
	Chip_GPIOGP_EnableGroupPins(LPC_GPIOGROUP, 0, SWITCH_KEY_GPIO_PORT, (1 << SWITCH_KEY_GPIO_PIN) );
	//Chip_GPIOGP_EnableGroupPins(LPC_GPIOGROUP, 0, UP_SWITCH_PORT, 1 << UP_SWITCH_BIT); // Use this if different port is used in the same group
	Chip_GPIOGP_SelectAndMode(LPC_GPIOGROUP, 0);
	Chip_GPIOGP_SelectEdgeMode(LPC_GPIOGROUP, 0);

	// Set as gpio without pull-up/down/open-drain.
	Chip_IOCON_PinMuxSet(LPC_IOCON, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN, VOUT_ENABLE_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, VOUT_ENABLE_GPIO_PORT, VOUT_ENABLE_GPIO_PIN);

	// Set as gpio without pull-up/down/open-drain for LED_R
	Chip_IOCON_PinMuxSet(LPC_IOCON, LED_R_GPIO_PORT, LED_R_GPIO_PIN, LED_R_GPIO_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN);

	// Set as gpio without pull-up/down/open-drain for LED_G
	Chip_IOCON_PinMuxSet(LPC_IOCON, LED_G_GPIO_PORT, LED_G_GPIO_PIN, LED_G_GPIO_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN);

	// Set as gpio without pull-up/down/open-drain for LED_Y
	Chip_IOCON_PinMuxSet(LPC_IOCON, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN, LED_Y_GPIO_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN);

	LED_G_toggle_cnt = LED_R_toggle_cnt = LED_Y_toggle_cnt = 0;

#else

	Chip_IOCON_PinMuxSet(LPC_IOCON, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN, SWITCH_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN);
	//Chip_GPIO_SetPinDIRInput(LPC_GPIO, ISP_KEY_GPIO_PORT, ISP_KEY_GPIO_PIN);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GROUP0INT);
	Chip_GPIOGP_SelectLowLevel(LPC_GPIOGROUP, 0, GINT0_GPIO_PORT0, ((1 << GINT0_GPIO_BIT0) | (1 << GINT0_GPIO_BIT1)));
	// Chip_GPIOGP_SelectLowLevel(LPC_GPIOGROUP, 0, UP_SWITCH_PORT, 1 << UP_SWITCH_BIT); // Use this if different port is used in the same group
	Chip_GPIOGP_EnableGroupPins(LPC_GPIOGROUP, 0, GINT0_GPIO_PORT0, ((1 << GINT0_GPIO_BIT0) | (1 << GINT0_GPIO_BIT1)));
	//Chip_GPIOGP_EnableGroupPins(LPC_GPIOGROUP, 0, UP_SWITCH_PORT, 1 << UP_SWITCH_BIT); // Use this if different port is used in the same group
	Chip_GPIOGP_SelectAndMode(LPC_GPIOGROUP, 0);
	Chip_GPIOGP_SelectEdgeMode(LPC_GPIOGROUP, 0);

	// 2nd button - for voltage output selection branch
	Chip_IOCON_PinMuxSet(LPC_IOCON, SECOND_KEY_GPIO_PORT, SECOND_KEY_GPIO_PIN, SECOND_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SECOND_KEY_GPIO_PORT, SECOND_KEY_GPIO_PIN);

	// Set as gpio without pull-up/down/open-drain for LED_R
	Chip_IOCON_PinMuxSet(LPC_IOCON, LED_R_GPIO_PORT, LED_R_GPIO_PIN, LED_R_GPIO_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN);
	//Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_R_GPIO_PORT, LED_R_GPIO_PIN);

	// Set as gpio without pull-up/down/open-drain for LED_G
	Chip_IOCON_PinMuxSet(LPC_IOCON, LED_G_GPIO_PORT, LED_G_GPIO_PIN, LED_G_GPIO_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN);
	//Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_G_GPIO_PORT, LED_G_GPIO_PIN);

	// Set as gpio without pull-up/down/open-drain for LED_Y
	Chip_IOCON_PinMuxSet(LPC_IOCON, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN, LED_Y_GPIO_PIN_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN);
	//Chip_GPIO_SetPinOutLow(LPC_GPIO, LED_Y_GPIO_PORT, LED_Y_GPIO_PIN);


#endif // #if defined(_REAL_UPDATEKIT_V2_BOARD_) || (_HOT_SPRING_BOARD_V2_)

//	GPIOGoup0_Int = false;
#if defined (_HOT_SPRING_BOARD_V2_)
	Chip_IOCON_PinMuxSet(LPC_IOCON, BLUE_KEY_GPIO_PORT, BLUE_KEY_GPIO_PIN, BLUE_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, BLUE_KEY_GPIO_PORT, BLUE_KEY_GPIO_PIN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, YELLOW_KEY_GPIO_PORT, YELLOW_KEY_GPIO_PIN, YELLOW_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, YELLOW_KEY_GPIO_PORT, YELLOW_KEY_GPIO_PIN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, GREEN_KEY_GPIO_PORT, GREEN_KEY_GPIO_PIN, GREEN_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, GREEN_KEY_GPIO_PORT, GREEN_KEY_GPIO_PIN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, RED_KEY_GPIO_PORT, RED_KEY_GPIO_PIN, RED_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, RED_KEY_GPIO_PORT, RED_KEY_GPIO_PIN);

	GPIO_Command_Update_Mux_and_Direction(0,0);
	GPIO_Command_Update_Mux_and_Direction(1,0);
	GPIO_Command_Update_Mux_and_Direction(2,0);
#endif // #if defined(_REAL_UPDATEKIT_V2_BOARD_) || defined (_HOT_SPRING_BOARD_V2_)

	/* Enable Group GPIO interrupt 0 */
//	NVIC_EnableIRQ(GINT0_IRQn);
// Use SW debounce now

}

void Set_GPIO_Dirction_Command(uint8_t port, uint32_t new_dir_value)
{
	GPIO_Command_Update_Mux_and_Direction(port,new_dir_value);
}

uint32_t Get_GPIO_Direction_Command(uint8_t port)
{
	uint32_t mask;

	mask = GPIO_pin_mask[port] & GPIO_pin_mask_by_mode[GPIO_Pin_Mode][port];
	return (Chip_GPIO_GetPortDIR(LPC_GPIO, port) & mask);
}

void Set_GPIO_Mask_Command(uint8_t port, uint32_t set_mask_value)
{
	uint32_t current_mask, pin_mask_by_mode;

	pin_mask_by_mode = GPIO_pin_mask_by_mode[GPIO_Pin_Mode][port];
	// (1) clear all mask-able pin so that it can be OR later
	current_mask = GPIO_pin_mask[port] & ~pin_mask_by_mode;
	// (2) set according to set_mask_value
	current_mask |= set_mask_value & pin_mask_by_mode;
	GPIO_pin_mask[port] = current_mask;
}

uint32_t Get_GPIO_Mask_Command(uint8_t port)
{
	uint32_t mask;

	mask = GPIO_pin_mask[port] & GPIO_pin_mask_by_mode[GPIO_Pin_Mode][port];
	return (mask);
}

void Set_GPIO_Output_Command(uint8_t port, uint32_t out_value)
{
	uint32_t mask;

	mask = GPIO_pin_mask[port] & GPIO_pin_mask_by_mode[GPIO_Pin_Mode][port];
	Chip_GPIO_SetPortMask(LPC_GPIO, ~port,mask);
	Chip_GPIO_SetMaskedPortValue(LPC_GPIO,port,out_value);
}

uint32_t Get_Input_Command(uint8_t port)
{
	uint32_t mask;

	mask = GPIO_pin_mask[port] & GPIO_pin_mask_by_mode[GPIO_Pin_Mode][port];
	return (Chip_GPIO_GetPortValue(LPC_GPIO, port)&mask);
}

void Set_GPIO_PinMode_Command(uint32_t pin_mode)
{
	if(pin_mode<=Get_Max_PinMode())
	{
		GPIO_Pin_Mode = pin_mode;
		GPIO_pin_mask[0] &= GPIO_pin_mask_by_mode[GPIO_Pin_Mode][0];
		GPIO_pin_mask[1] &= GPIO_pin_mask_by_mode[GPIO_Pin_Mode][1];
		GPIO_pin_mask[2] &= GPIO_pin_mask_by_mode[GPIO_Pin_Mode][2];
	}
}

uint32_t Get_PinMode_Command(void)
{
	return GPIO_Pin_Mode;
}

uint8_t Get_Max_PinMode(void)
{
	return ( ( sizeof(GPIO_pin_mask_by_mode) / (sizeof(uint32_t)*_PORT_NO_) ) - 1 );
}

#define GPIO_INPUT_MUX		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)
#define GPIO_OUTPUT_MUX		(IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_HYS_EN | IOCON_DIGMODE_EN)

bool Get_GPIO_Pin_Command(uint8_t port, uint8_t pin)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, GPIO_INPUT_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	return (Chip_GPIO_GetPinState(LPC_GPIO, port, pin));
}

void Set_GPIO_Pin_Command(uint8_t port, uint8_t pin, bool pin_value)
{
	Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, GPIO_OUTPUT_MUX);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
	Chip_GPIO_SetPinState(LPC_GPIO, port, pin, pin_value);
}

bool Get_GPIO_Switch_Key(void)
{
	return (Chip_GPIO_GetPinState(LPC_GPIO, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN));
}

bool		negFlag = false;
bool    	posFlag = false;
uint32_t	count;
bool		event_already_raised = false;

bool Debounce_Button(void)
{
	bool bRet = false;

	count++;

	//check switch
	if(!(Get_GPIO_Switch_Key()) && !negFlag && !posFlag)
	{
	    count = 0;         		//first debounce count start.
	    negFlag = true;     	//fall edge debounce - stage 2
	    event_already_raised=false;
	}
	//after debounce time-out, check if button is still pressed - stage 2
	else if(negFlag && !posFlag && (count > DEBOUNCE_COUNT))
	{
	    if(!Get_GPIO_Switch_Key())
	    {
	    	// If still button-pressed & if event not-yet raised for this button-pressed (after debounce)
	    	if(event_already_raised==false)
	    	{
	    		event_already_raised = true;
	    		bRet = true;
	    	}
    		posFlag = true;      //  wait for button-release & debounce (stage 3)
	    }
	    else
	        negFlag = false;     // already back-to-high after debounce-time --> treat it as glitch of GPIO and back to stage 1
	}
	// if button released then wait debounce - stage 3
	else if(Get_GPIO_Switch_Key() && negFlag && posFlag)
	{
	    count = 0;          //rise edge debounce count start.
	    negFlag = false;
	}
	// after debounce of button released (stage 4), always back to stage 1
	else if(!negFlag && posFlag && (count > DEBOUNCE_COUNT) )
	{
    	posFlag = false; //finish rising-edge Debounce cycle.
	}

	return bRet;
}

// For voltage output selection branch

bool Get_GPIO_2nd_Key(void)
{
	return (Chip_GPIO_GetPinState(LPC_GPIO, SECOND_KEY_GPIO_PORT, SECOND_KEY_GPIO_PIN));
}

bool		negFlag_2nd_key = false;
bool    	posFlag_2nd_key = false;
uint32_t	count_2nd_key;
bool		event_already_raised_2nd_key = false;

bool Debounce_2nd_Key(void)
{
	bool bRet = false;

	count_2nd_key++;

	//check switch
	if(!(Get_GPIO_2nd_Key()) && !negFlag_2nd_key && !posFlag_2nd_key)
	{
		count_2nd_key = 0;         		//first debounce count start.
	    negFlag_2nd_key = true;     	//fall edge debounce - stage 2
	    event_already_raised_2nd_key=false;
	}
	//after debounce time-out, check if button is still pressed - stage 2
	else if(negFlag_2nd_key && !posFlag_2nd_key && (count_2nd_key > DEBOUNCE_COUNT))
	{
	    if(!Get_GPIO_2nd_Key())
	    {
	    	// If still button-pressed & if event not-yet raised for this button-pressed (after debounce)
	    	if(event_already_raised_2nd_key==false)
	    	{
	    		event_already_raised_2nd_key = true;
	    		bRet = true;
	    	}
	    	posFlag_2nd_key = true;      //  wait for button-release & debounce (stage 3)
	    }
	    else
	    	negFlag_2nd_key = false;     // already back-to-high after debounce-time --> treat it as glitch of GPIO and back to stage 1
	}
	// if button released then wait debounce - stage 3
	else if(Get_GPIO_2nd_Key() && negFlag_2nd_key && posFlag_2nd_key)
	{
		count_2nd_key = 0;          //rise edge debounce count start.
	    negFlag_2nd_key = false;
	}
	// after debounce of button released (stage 4), always back to stage 1
	else if(!negFlag_2nd_key && posFlag_2nd_key && (count_2nd_key > DEBOUNCE_COUNT) )
	{
		posFlag_2nd_key = false; //finish rising-edge Debounce cycle.
	}

	return bRet;
}

// For voltage output selection branch

void LED_Status_Set_Value(uint32_t LED_status_value)
{
	// Assumption: all status LED use the same GPIO port
	Chip_GPIO_SetPortMask(LPC_GPIO, LED_R_GPIO_PORT, LED_STATUS_MASK);
	Chip_GPIO_SetMaskedPortValue(LPC_GPIO,LED_R_GPIO_PORT,LED_status_value);
}

void LED_Status_Set_Auto_Toggle(uint32_t LED_status_pin, uint8_t flashing_100ms, uint32_t flashing_cnt)
{
	if(LED_status_pin & LED_STATUS_G)
	{
		LED_G_toggle_cnt = flashing_cnt;
		//Start_SW_Timer(LED_G_TIMER_IN_100MS,flashing_100ms-1,flashing_100ms-1,TIMER_100MS, false, false);		// countdown / repeated
		Repeat_DownCounter(LED_G_TIMER_IN_100MS,flashing_100ms,TIMER_100MS);
	}
	if(LED_status_pin & LED_STATUS_Y)
	{
		LED_Y_toggle_cnt = flashing_cnt;
		//Start_SW_Timer(LED_Y_TIMER_IN_100MS,flashing_100ms-1,flashing_100ms-1,TIMER_100MS, false, false);		// countdown / repeated
		Repeat_DownCounter(LED_Y_TIMER_IN_100MS,flashing_100ms,TIMER_100MS);
	}
	if(LED_status_pin & LED_STATUS_R)
	{
		LED_R_toggle_cnt = flashing_cnt;
		//Start_SW_Timer(LED_R_TIMER_IN_100MS,flashing_100ms-1,flashing_100ms-1,TIMER_100MS, false, false);		// countdown / repeated
		Repeat_DownCounter(LED_R_TIMER_IN_100MS,flashing_100ms,TIMER_100MS);
	}
}

void LED_Status_Clear_Auto_Toggle(uint32_t LED_status_pin)
{
	if(LED_status_pin & LED_STATUS_G)
	{
		LED_G_toggle_cnt = 0;
		Pause_SW_Timer(LED_G_TIMER_IN_100MS);
	}
	if(LED_status_pin & LED_STATUS_Y)
	{
		LED_Y_toggle_cnt = 0;
		Pause_SW_Timer(LED_Y_TIMER_IN_100MS);
	}
	if(LED_status_pin & LED_STATUS_R)
	{
		LED_R_toggle_cnt = 0;
		Pause_SW_Timer(LED_R_TIMER_IN_100MS);
	}
}

void LED_Status_Update_Process(void)
{
	if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_G_TIMER_IN_100MS))
	{
		if(LED_G_toggle_cnt>0)
		{
			LED_G_TOGGLE;
			LED_G_toggle_cnt--;
		}
		if(LED_G_toggle_cnt==0)
		{
            Pause_SW_Timer(LED_G_TIMER_IN_100MS);
		}
	}

	if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_Y_TIMER_IN_100MS))
	{
		if(LED_Y_toggle_cnt>0)
		{
			LED_Y_TOGGLE;
			LED_Y_toggle_cnt--;
		}
		if(LED_Y_toggle_cnt==0)
		{
            Pause_SW_Timer(LED_Y_TIMER_IN_100MS);
		}
	}

	if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_R_TIMER_IN_100MS))
	{
		if(LED_R_toggle_cnt>0)
		{
			LED_R_TOGGLE;
			LED_R_toggle_cnt--;
		}
		if(LED_R_toggle_cnt==0)
		{
            Pause_SW_Timer(LED_R_TIMER_IN_100MS);
		}
	}
}

void DeInit_GPIO(void)
{
	NVIC_DisableIRQ(USART0_IRQn);
	Chip_UART0_DeInit(LPC_USART0);
}


//void LED_demo(void)
//{
////		switch(time_elapse_in_sec&0x03)
//		switch(Read_SW_TIMER_Value(SYSTEM_TIME_ELAPSE_IN_SEC)&0x03)
//		{
//			case 0:
//				LED_R_HIGH;
//				LED_Y_LOW;
//				LED_G_LOW;
//				break;
//			case 1:
//				LED_R_LOW;
//				LED_Y_HIGH;
//				LED_G_LOW;
//				break;
//			case 2:
//				LED_R_LOW;
//				LED_Y_LOW;
//				LED_G_HIGH;
//				break;
//			case 3:
//				LED_R_LOW;
//				LED_Y_LOW;
//				LED_G_LOW;
//				break;
//		}
//}
