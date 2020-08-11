/*
 * tpic6b595.c
 *
 *  Created on: May 7, 2020
 *      Author: Jeremy
 */

#include "board.h"
#include "stdbool.h"
#include "res_state.h"

//#define _TEST_ON_BOARD_LPC11U68
#ifdef _TEST_ON_BOARD_LPC11U68

#define MCU_RCK_port	2
#define MCU_RCK_pin		11
#define MCU_RCK_mux		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		// P1.0-2, 4-8. 10-21, 23-28, 30-31

#define MCU_SRCK_port 	2
#define MCU_SRCK_pin	12
#define MCU_SRCK_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

// used as input pin
#define MCU_SERIN_port 	1		// SEROUT pin from last tpic6b595 IC
#define MCU_SERIN_pin	18
#define MCU_SERIN_mux	(IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGMODE_EN)		// P0.0-3; 6-10; 17-21;

#define MCU_SRCLR_port 	1
#define MCU_SRCLR_pin	24
#define MCU_SRCLR_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

// PIO0_8 - D12
#define MCU_G_port 		1
#define MCU_G_pin		19
#define MCU_G_mux		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

// PIO1_29 - D13
#define MCU_SEROUT_port 1		// SERIN pin to first tpic6b595 IC
#define MCU_SEROUT_pin	26
#define MCU_SEROUT_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)		// P2.0-1 -- Digital mode (ie ADMODE is off)

/*
// PIO1_28 - D8
#define MCU_RCK_port	1
#define MCU_RCK_pin		28
#define MCU_RCK_mux		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		// P1.0-2, 4-8. 10-21, 23-28, 30-31

// PIO2_3 - D9
#define MCU_SRCK_port 	2
#define MCU_SRCK_pin	3
#define MCU_SRCK_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

// PIO0_2 - D10 -- used as input pin
#define MCU_SERIN_port 	0		// SEROUT pin from last tpic6b595 IC
#define MCU_SERIN_pin	2
#define MCU_SERIN_mux	(IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGMODE_EN)		// P0.0-3; 6-10; 17-21;

// PIO0_9 - D11
#define MCU_SRCLR_port 	0
#define MCU_SRCLR_pin	9
#define MCU_SRCLR_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		// P0.0-3; 6-10; 17-21;

// PIO0_8 - D12
#define MCU_G_port 		0
#define MCU_G_pin		8
#define MCU_G_mux		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

// PIO1_29 - D13
#define MCU_SEROUT_port 1		// SERIN pin to first tpic6b595 IC
#define MCU_SEROUT_pin	29
#define MCU_SEROUT_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)		// P2.0-1 -- Digital mode (ie ADMODE is off)
*/

#else

#define MCU_RCK_port	2
#define MCU_RCK_pin		0
#define MCU_RCK_mux		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)		// P2.0-1 -- Digital mode (ie ADMODE is off)

#define MCU_SRCK_port 	2
#define MCU_SRCK_pin	2
#define MCU_SRCK_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

#define MCU_SRCLR_port 	2
#define MCU_SRCLR_pin	5
#define MCU_SRCLR_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23

#define MCU_G_port 		2
#define MCU_G_pin		7
#define MCU_G_mux		(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN )		//P2.2-23
//
#define MCU_SEROUT_port 2		// SERIN pin to first tpic6b595 IC
#define MCU_SEROUT_pin	1
#define MCU_SEROUT_mux	(IOCON_FUNC0 | IOCON_MODE_INACT | IOCON_DIGMODE_EN)		// P2.0-1 -- Digital mode (ie ADMODE is off)

// PIO0_2 - D10
#define MCU_SERIN_port 	0		// SEROUT pin from last tpic6b595 IC
#define MCU_SERIN_pin	2
#define MCU_SERIN_mux	(IOCON_FUNC0 | IOCON_MODE_PULLUP | IOCON_DIGMODE_EN)		// P0.0-3; 6-10; 17-21;

#endif // _BOARD_LPC11U68

#define RCK_0()			Chip_GPIO_SetPinOutLow(LPC_GPIO,MCU_RCK_port,MCU_RCK_pin)
#define RCK_1()			Chip_GPIO_SetPinOutHigh(LPC_GPIO,MCU_RCK_port,MCU_RCK_pin)
#define SRCK_0()		Chip_GPIO_SetPinOutLow(LPC_GPIO,MCU_SRCK_port,MCU_SRCK_pin)
#define SRCK_1()		Chip_GPIO_SetPinOutHigh(LPC_GPIO,MCU_SRCK_port,MCU_SRCK_pin)
#define SRCLR_0()		Chip_GPIO_SetPinOutLow(LPC_GPIO,MCU_SRCLR_port,MCU_SRCLR_pin)
#define SRCLR_1()		Chip_GPIO_SetPinOutHigh(LPC_GPIO,MCU_SRCLR_port,MCU_SRCLR_pin)
#define G_0()			Chip_GPIO_SetPinOutLow(LPC_GPIO,MCU_G_port,MCU_G_pin)
#define G_1()			Chip_GPIO_SetPinOutHigh(LPC_GPIO,MCU_G_port,MCU_G_pin)
#define SEROUT_0()		Chip_GPIO_SetPinOutLow(LPC_GPIO,MCU_SEROUT_port,MCU_SEROUT_pin)
#define SEROUT_1()		Chip_GPIO_SetPinOutHigh(LPC_GPIO,MCU_SEROUT_port,MCU_SEROUT_pin)

#define G_SetValue(x)		Chip_GPIO_SetPinState(LPC_GPIO,MCU_G_port,MCU_G_pin,x)
#define SEROUT_SetValue(x)	Chip_GPIO_SetPinState(LPC_GPIO,MCU_SEROUT_port,MCU_SEROUT_pin,x)
#define SERIN_GetValue()	Chip_GPIO_GetPinState(LPC_GPIO,MCU_SERIN_port,MCU_SERIN_pin)

static void inline Short_Delay(void)
{
	// estimated 20.883 ns
	__NOP();
//	// estimated 41.667 ns
//	__NOP();
//	__NOP();
//	// estimated 62.5ns
//	__NOP();
//	__NOP();
//	__NOP();
}

#define RESISTOR_BIT_LEN			(20)
#define RESISTOR_MSB_INDEX			(RESISTOR_BIT_LEN-1)
#define RESISTOR_MSB				(1UL<<RESISTOR_MSB_INDEX)
#define SMALL_RESISITOR_LEN      	(4)
#define SMALL_RESISITOR_BOUND		(1UL<<SMALL_RESISITOR_LEN)

#define RESISTOR_BIT_MASK			((1UL<<RESISTOR_BIT_LEN)-1)

bool 		disable_relay_control = false, selftest_is_on = false;
uint32_t	selftest_total_count = 0, selftest_ok_count = 0;

bool get_disable_relay_control(void)
{
	return disable_relay_control;
}

void set_disable_relay_control(bool set_disable)
{
	disable_relay_control = set_disable;
}

bool get_tpic6b595_selftest_On(void)
{
	return selftest_is_on;
}

void set_tpic6b595_selftest_On(bool OnOff)
{
	selftest_is_on = OnOff;
	if(selftest_is_on)
	{
		selftest_total_count = selftest_ok_count = 0;
	}
}

uint32_t get_tpic6b595_selftest_Total_Count(void)
{
	return selftest_total_count;
}

uint32_t get_tpic6b595_selftest_OK_Count(void)
{
	return selftest_ok_count;
}

void Calc_Relay_Value(uint32_t *Resistor, uint64_t *Relay)
{
	uint64_t	Relay_Value;
//	uint8_t		resistor_index;

#ifdef _BOARD_DEBUG_SW_
	if(Resistor[0]==~0UL)
	{
		// RELAY-A/B/C is high to disconnect shortcut to small value Relay (default is to connect)
		*Relay = (7ULL<<60);
		return;
	}
#endif // #ifdef _BOARD_DEBUG_SW_

	Relay_Value  = Resistor[2]&RESISTOR_BIT_MASK;
	Relay_Value <<= RESISTOR_BIT_LEN;
	Relay_Value |= Resistor[1]&RESISTOR_BIT_MASK;
	Relay_Value <<= RESISTOR_BIT_LEN;
	Relay_Value |= Resistor[0]&RESISTOR_BIT_MASK;

	// Shortcut for small resistor value
	if(Resistor[2]>=SMALL_RESISITOR_BOUND)
	{
		Relay_Value |= (1ULL<<62);
	}
	if(Resistor[1]>=SMALL_RESISITOR_BOUND)
	{
		Relay_Value |= (1ULL<<61);
	}
	if(Resistor[0]>=SMALL_RESISITOR_BOUND)
	{
		Relay_Value |= (1ULL<<60);
	}

	// Return relay value
	*Relay = Relay_Value;
}

//void Calc_Relay_Value(uint32_t *Resistor, uint64_t *Relay)
//{
//	uint64_t	Relay_Value;
//	uint8_t		resistor_index;
//
//#ifdef _BOARD_DEBUG_SW_
//	if(Resistor[0]==~0UL)
//	{
//		// RELAY-A/B/C is high to disconnect shortcut to small value Relay (default is to connect)
//		*Relay = (7ULL<<60);
//		return;
//	}
//#endif // #ifdef _BOARD_DEBUG_SW_
//
//	Relay_Value = 0;
//	resistor_index = 2; // RC->RB->RA
//	do
//	{
//		uint32_t	Resistor_Value = Resistor[resistor_index];
//		uint8_t		index = RESISTOR_MSB_INDEX;
//		do
//		{
//			Relay_Value<<=1;
//			if(Resistor_Value&RESISTOR_MSB)
//			{
//				Relay_Value |= 1;
//			}
//			Resistor_Value<<=1;
//		}
//		while(index-->0);
//
//	}
//	while (resistor_index-->0);
//
//	// Shortcut for small resistor value
//	if(Resistor[2]>=SMALL_RESISITOR_BOUND)
//	{
//		Relay_Value |= (1ULL<<62);
//	}
//	if(Resistor[1]>=SMALL_RESISITOR_BOUND)
//	{
//		Relay_Value |= (1ULL<<61);
//	}
//	if(Resistor[0]>=SMALL_RESISITOR_BOUND)
//	{
//		Relay_Value |= (1ULL<<60);
//	}
//
//	// Return relay value
//	*Relay = Relay_Value;
//}

// 0-19 is for 2N ohm
// 20 is for 0ohm with shortcut (RL21/42/63 low)
// 21 is for 0phm without shortcut (RL21/42/63 high)
void Calc_Relay_Value_2N(uint32_t Value_2N, uint64_t *Relay)
{
	uint64_t	Relay_Value;

	if (Value_2N<SMALL_RESISITOR_LEN)
	{
		Relay_Value = (7ULL<<60) | (((1ULL<<0)|(1ULL<<RESISTOR_BIT_LEN)|(1ULL<<(RESISTOR_BIT_LEN*2)))<<Value_2N);
	}
	else if (Value_2N<RESISTOR_BIT_LEN)
	{
		Relay_Value = (((1ULL<<0)|(1ULL<<RESISTOR_BIT_LEN)|(1ULL<<(RESISTOR_BIT_LEN*2)))<<Value_2N);
	}
	else if (Value_2N==20)
	{
		Relay_Value = 0;				// the shortest path
	}
	else
	{
		Relay_Value = (7ULL<<60);
	}
	*Relay = Relay_Value;
}

static uint32_t MCU_SERIN_log = 0;

uint8_t const Shift_Register_GPIO_Initial_Matrix [] =
{
//		port				pin				output	init_high
		MCU_G_port,			MCU_G_pin, 		true,	true,			// D12
		MCU_SRCLR_port,		MCU_SRCLR_pin,  true,	true,			// D11
		MCU_RCK_port, 		MCU_RCK_pin, 	true,	false,			// D8
		MCU_SRCK_port,		MCU_SRCK_pin,	true,	false,			// D9
		MCU_SEROUT_port,	MCU_SEROUT_pin,	true,	true,			// D13
		MCU_SERIN_port,		MCU_SERIN_pin,	false,	true,			// D10
};

uint32_t const Shift_Register_GPIO_Initial_Mux [] =
{
		MCU_G_mux,
		MCU_SRCLR_mux,
		MCU_RCK_mux,
		MCU_SRCK_mux,
		MCU_SEROUT_mux,
		MCU_SERIN_mux,
};

void Init_Shift_Register_GPIO(void)
{
	uint8_t	index = sizeof(Shift_Register_GPIO_Initial_Matrix)/sizeof(uint8_t);
	uint8_t	index_mux = sizeof(Shift_Register_GPIO_Initial_Mux)/sizeof(uint32_t);

	while(index>0)
	{
		uint8_t		dir, pin, port, init_state;
		uint32_t	mux;

		init_state = Shift_Register_GPIO_Initial_Matrix[--index];
		dir = Shift_Register_GPIO_Initial_Matrix[--index];
		pin = Shift_Register_GPIO_Initial_Matrix[--index];
		port = Shift_Register_GPIO_Initial_Matrix[--index];

		mux = Shift_Register_GPIO_Initial_Mux[--index_mux];
		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, mux);

		Chip_GPIO_SetPinDIR(LPC_GPIO, port, pin, (bool)dir);
		if(dir)
		{
			Chip_GPIO_SetPinState(LPC_GPIO,port,pin,init_state);
		}
	}
}

void Enable_Shift_Register_Output(bool enable_true)
{
	// Setup _G pin -- low is enabled.
	G_SetValue(!enable_true);
}

void Latch_Register_Byte_to_Output(void)
{
	if(disable_relay_control)
		return;

	// an RCK high-pulse to latch value to output
	RCK_1();
	Short_Delay();
	RCK_0();
}

void Shift_and_Set_Register_Bit(bool set_bit_data)
{
	// Record data to be shift-out in next SRC-high-pulse into LSB of MCU_SERIN_log
	MCU_SERIN_log<<=1;
	if(SERIN_GetValue())
	{
		MCU_SERIN_log |= 1;
	}

	// Setup 595_SERIN pin
	SEROUT_SetValue(set_bit_data);
//	Short_Delay();

	// SRCK high-pulse to shift register byte and set register data bit
	SRCK_1();
//	Short_Delay();
	SRCK_0();
}

void Clear_Register_Byte(void)
{
	// _SRCLR low-pulse to clear shift register
	SRCLR_0();
	Short_Delay();
	SRCLR_1();
}

void Clear_Shiftout_log(void)
{
	MCU_SERIN_log = 0;
}

uint32_t Read_Shiftout_log(void)
{
	return MCU_SERIN_log;
}

uint32_t Setup_Shift_Register_32it(uint32_t value)
{
	uint32_t	value_temp;
	uint8_t		index;

	value_temp = value;

	index = 32;
	do
	{
		Shift_and_Set_Register_Bit(((value_temp&(1U<<31))!=0)?1:0);
		value_temp<<=1;
	}
	while(--index);

	return Read_Shiftout_log();;
}

//uint32_t Test_Shift_Register(uint8_t test_value)
//{
//	// Setup bit1 then bit0
//	Shift_and_Set_Register_Bit(((test_value&0x2)!=0)?1:0);
//	Shift_and_Set_Register_Bit(test_value & 0x1);
//	Latch_Register_Byte_to_Output();
//
//	return Read_Shiftout_log();
//}

bool SelfTest_Shift_Register(void)
{
	uint32_t	high, low, readout_high, readout_low;
	bool		bRet=false;

	high = 0x55aa55aa;
	low  = 0xff00ff00;
	readout_high = Setup_Shift_Register_32it(high);
	readout_low = Setup_Shift_Register_32it(low);
	readout_high = Setup_Shift_Register_32it(readout_high);
	readout_low = Setup_Shift_Register_32it(readout_low);
	if((readout_high==high)&&(readout_low==low))
	{
		bRet = true;
		selftest_ok_count++;
	}
	selftest_total_count++;
	return bRet;
}

bool Setup_Shift_Register_and_Test_64bit(uint64_t value)
{
	uint32_t	high, low, readout_high, readout_low;
	bool		bRet=false;

	high = (uint32_t)((value>>32)&~(0UL));
	low  = (uint32_t)(value&~(0UL));
	Setup_Shift_Register_32it(high);
	Setup_Shift_Register_32it(low);
	readout_high = Setup_Shift_Register_32it(high);
	readout_low = Setup_Shift_Register_32it(low);
	if((readout_high==high)&&(readout_low==low))
	{
		bRet = true;
	}
	return bRet;
}

void Setup_Shift_Register_64bit(uint64_t value)
{
	uint32_t	high, low;

	high = (uint32_t)((value>>32)&~(0UL));
	low  = (uint32_t)(value&~(0UL));
	Setup_Shift_Register_32it(high);
	Setup_Shift_Register_32it(low);
}

/*
void Update_Resistor_Relay(uint32_t *latest_res_value)
{
	static UPDATE_RELAY_STATE state = URS_INITIAL;
	static uint32_t res_setup[3] = {0, 0, 0}, res_modified = { ~1UL, ~1UL, ~1UL };
	uint8_t		index;

	switch(state)
	{
		case URS_INITIAL:
			// setup relay
			// setup variable
			state = URS_POLLING_CHANGE;
			break;
		case URS_WAIT_STABLE:
			if(Read_and_Clear_SW_TIMER_Reload_Flag(RELAY_SETUP_HYSTERSIS_IN_100MS))
			{
				state = URS_POLLING_CHANGE;
			}
			break;
		case URS_POLLING_CHANGE:
			index=2;
			do
			{
				if(res_modified[index] != latest_res_value[index])
				{
					res_modified[index] = latest_res_value[index];
					Countdown_Once(RELAY_SETUP_HYSTERSIS_IN_100MS,10,TIMER_100MS);
					state = URS_WAIT_STABLE;
				}
			}
			while(index-->0);
			break;
	}


}
*/

