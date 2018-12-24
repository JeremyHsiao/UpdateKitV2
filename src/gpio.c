/*
 * gpio.c
 *
 *  Created on: 2018年11月26日
 *      Author: jeremy.hsiao
 */

#include "board.h"
#include "sw_timer.h"
#include "gpio.h"

/*****************************************************************************
 * Private types/enumerations/variables
 ****************************************************************************/

/*****************************************************************************
 * Public types/enumerations/variables
 ****************************************************************************/
bool GPIOGoup0_Int;

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
void GINT0_IRQHandler(void)
{
	Chip_GPIOGP_ClearIntStatus(LPC_GPIOGROUP, 0);
	//Board_LED_Set(0, true);
	GPIOGoup0_Int = true;
}

void Init_GPIO(void)
{
#ifdef _REAL_UPDATEKIT_V2_BOARD_

	// Set PIO0_3
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 3, (IOCON_FUNC0 | IOCON_MODE_INACT | (1L<<7)));				// P0.0-3; 6-10; 17-21);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 3);
	Chip_GPIO_SetPinOutLow(LPC_GPIO, 0, 3);

	// This is for input button - original
	Chip_IOCON_PinMuxSet(LPC_IOCON, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN, SWITCH_KEY_PIN_MUX);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN);
	//Chip_GPIO_SetPinDIRInput(LPC_GPIO, ISP_KEY_GPIO_PORT, ISP_KEY_GPIO_PIN);

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

#else
	Chip_IOCON_PinMuxSet(LPC_IOCON, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN, (SWITCH_KEY_PIN_AS_GPIO | IOCON_MODE_INACT ));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN);
	//Chip_GPIO_SetPinDIRInput(LPC_GPIO, ISP_KEY_GPIO_PORT, ISP_KEY_GPIO_PIN);

	Chip_Clock_EnablePeriphClock(SYSCTL_CLOCK_GROUP0INT);
	Chip_GPIOGP_SelectLowLevel(LPC_GPIOGROUP, 0, GINT0_GPIO_PORT0, ((1 << GINT0_GPIO_BIT0) | (1 << GINT0_GPIO_BIT1)));
	// Chip_GPIOGP_SelectLowLevel(LPC_GPIOGROUP, 0, UP_SWITCH_PORT, 1 << UP_SWITCH_BIT); // Use this if different port is used in the same group
	Chip_GPIOGP_EnableGroupPins(LPC_GPIOGROUP, 0, GINT0_GPIO_PORT0, ((1 << GINT0_GPIO_BIT0) | (1 << GINT0_GPIO_BIT1)));
	//Chip_GPIOGP_EnableGroupPins(LPC_GPIOGROUP, 0, UP_SWITCH_PORT, 1 << UP_SWITCH_BIT); // Use this if different port is used in the same group
	Chip_GPIOGP_SelectAndMode(LPC_GPIOGROUP, 0);
	Chip_GPIOGP_SelectEdgeMode(LPC_GPIOGROUP, 0);

#endif // #ifdef _REAL_UPDATEKIT_V2_BOARD_

	GPIOGoup0_Int = false;

	/* Enable Group GPIO interrupt 0 */
//	NVIC_EnableIRQ(GINT0_IRQn);
// Use SW debounce now

}

bool Get_GPIO_Switch_Key(void)
{
	return (Chip_GPIO_GetPinState(LPC_GPIO, SWITCH_KEY_GPIO_PORT, SWITCH_KEY_GPIO_PIN));
}

bool		negFlag = false;
bool    	posFlag = false;
uint32_t	count;

bool Debounce_Button(void)
{
	count++;

	//check switch
	if(!(Get_GPIO_Switch_Key()) && !negFlag && !posFlag)
	{
	    count = 0;         		//first debounce count start.
	    negFlag = true;     	//fall edge debounce
	    return true;
	}

	//if falling-edge debounce time-out
	if(negFlag && !posFlag && (count > DEBOUNCE_COUNT))
	{
	    if(!Get_GPIO_Switch_Key())
	        posFlag = true;      //  rising edge debounce is required later when button is released
	    else
	        negFlag = false;     // already back-to-high, no need to debounce low-to-high when release pressing
	}

	// falling debounce is done & now start a rising debounce
	if(Get_GPIO_Switch_Key() && negFlag && posFlag)
	{
	    count = 0;          //rise edge debounce count start.
	    negFlag = false;
	    return false;
	}

	//switch rise edge debounce success.
	if(!negFlag && posFlag && (count > DEBOUNCE_COUNT) )
	{
	    posFlag = false; //finish rising-edge Debounce cycle.
	}

	return false;
}

uint8_t	LED_G_flashing = 0, LED_R_flashing = 0, LED_Y_flashing = 0;
#define 	DEFAULT_LED_TIMER_RELOAD_VALUE	(5)
// flashing_period 0: always low
// flashing_period 0xff: always high
// flashing_period others: toggle every flashing_period 100ms (max 25.4 s)
void LED_G_setting(uint8_t flashing_100ms)
{
	if(LED_G_flashing!=flashing_100ms)
	{
		uint32_t		new_timer_reload_value;
		bool			led_high;

		if(flashing_100ms==0)
		{
			//new_timer_reload_value = ~1;
			Pause_SW_Timer(LED_G_TIMER_IN_100MS);
			led_high = false;
		}
		else if (flashing_100ms==0xff)
		{
			//new_timer_reload_value = ~1;
			Pause_SW_Timer(LED_G_TIMER_IN_100MS);
			led_high = true;
		}
		else
		{
			new_timer_reload_value = flashing_100ms - 1;
			led_high = true;
			// System elapse timer: starting from 0 / no-reload-upper-value / 1000ms each count / upcount / not-oneshot
			Start_SW_Timer(LED_G_TIMER_IN_100MS,new_timer_reload_value,new_timer_reload_value,TIMER_100MS, false, false);
		}
		if(led_high) { LED_G_HIGH; } else { LED_G_LOW; }
		LED_G_flashing = flashing_100ms;
//		led_g_toggle_timer_in_100ms = led_g_toggle_timer_reload = new_timer_reload_value;
//		lcd_g_toggle_timeout=false;
	}
}
void LED_R_setting(uint8_t flashing_100ms)
{
	if(LED_R_flashing!=flashing_100ms)
	{
		uint32_t		new_timer_reload_value;
		bool			led_high;

		if(flashing_100ms==0)
		{
			new_timer_reload_value = ~1;
			led_high = false;
		}
		else if (flashing_100ms==0xff)
		{
			new_timer_reload_value = ~1;
			led_high = true;
		}
		else
		{
			new_timer_reload_value = flashing_100ms - 1;
			led_high = true;
		}
		if(led_high) { LED_R_HIGH; } else { LED_R_LOW; }
		LED_R_flashing = flashing_100ms;
		led_r_toggle_timer_in_100ms = led_r_toggle_timer_reload = new_timer_reload_value;
		lcd_r_toggle_timeout=false;
	}
}
void LED_Y_setting(uint8_t flashing_100ms)
{
	if(LED_Y_flashing!=flashing_100ms)
	{
		uint32_t		new_timer_reload_value;
		bool			led_high;

		if(flashing_100ms==0)
		{
			//new_timer_reload_value = ~1;
			Pause_SW_Timer(LED_Y_TIMER_IN_100MS);
			led_high = false;
		}
		else if (flashing_100ms==0xff)
		{
			//new_timer_reload_value = ~1;
			Pause_SW_Timer(LED_Y_TIMER_IN_100MS);
			led_high = true;
		}
		else
		{
			new_timer_reload_value = flashing_100ms - 1;
			led_high = true;
			// System elapse timer: starting from 0 / no-reload-upper-value / 1000ms each count / not-upcount / not-oneshot
			Start_SW_Timer(LED_Y_TIMER_IN_100MS,new_timer_reload_value,new_timer_reload_value,TIMER_100MS, false, false);
		}
		if(led_high) { LED_Y_HIGH; } else { LED_Y_LOW; }
		LED_Y_flashing = flashing_100ms;
//		led_y_toggle_timer_in_100ms = led_y_toggle_timer_reload = new_timer_reload_value;
//		lcd_y_toggle_timeout=false;
	}
}

void LED_Status_Update_Process(void)
{
	if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_G_TIMER_IN_100MS))
	{
		if(LED_G_flashing==0)
		{
			LED_G_LOW;
		}
		else if (LED_G_flashing==0xff)
		{
			LED_G_HIGH;
		}
		else
		{
			LED_G_TOGGLE;
		}
	}

	if(lcd_r_toggle_timeout)
	{
		lcd_r_toggle_timeout=false;

		if(LED_R_flashing==0)
		{
			LED_R_LOW;
		}
		else if (LED_R_flashing==0xff)
		{
			LED_R_HIGH;
		}
		else
		{
			LED_R_TOGGLE;
		}
	}

	if(Read_and_Clear_SW_TIMER_Reload_Flag(LED_Y_TIMER_IN_100MS))
	{
		//lcd_y_toggle_timeout=false;

		if(LED_Y_flashing==0)
		{
			LED_Y_LOW;
		}
		else if (LED_Y_flashing==0xff)
		{
			LED_Y_HIGH;
		}
		else
		{
			LED_Y_TOGGLE;
		}
	}

}

void DeInit_GPIO(void)
{
	NVIC_DisableIRQ(USART0_IRQn);
	Chip_UART0_DeInit(LPC_USART0);
}


void LED_demo(void)
{
//		switch(time_elapse_in_sec&0x03)
		switch(Read_SW_TIMER_Value(SYSTEM_TIME_ELAPSE_IN_SEC)&0x03)
		{
			case 0:
				LED_R_HIGH;
				LED_Y_LOW;
				LED_G_LOW;
				break;
			case 1:
				LED_R_LOW;
				LED_Y_HIGH;
				LED_G_LOW;
				break;
			case 2:
				LED_R_LOW;
				LED_Y_LOW;
				LED_G_HIGH;
				break;
			case 3:
				LED_R_LOW;
				LED_Y_LOW;
				LED_G_LOW;
				break;
		}
}
