/*
 * AlarmWave.c enables the pit and DAC on our board, and sets up a software interrupt-based
 * counter which, depending on the mode, either sends a constant 1.65v out of the DAC,
 * or sends a 300Hz sine wave (made from 64 samples) out of the DAC, which goes on and off
 * at a period of 1 second.
 *
 *  Created on: Nov 18, 2020
 *  Last edited on: Dec 10, 2020
 *      Author: August Byrne
 */

#include "MCUType.h"
#include "AlarmWave.h"
#include "K65TWR_GPIO.h"

#define CONSTVOLT 0x8000	//a constant which represents half of the full scale voltage of the DAC
#define HALFSEC 9600		//the number needed to create a half second (triggers on half second, for a 1 second period) (3125*9600=0.5sec)

void AlarmWaveInit(void);
void AlarmWaveControlTask(void);
void AlarmWaveSetMode(void);
void PIT0_IRQHandler(void);

typedef enum {ALARM_OFF, ALARM_ON} ALARM_SET_MODE;
typedef enum {SINE_OFF, SINE_ON} SINE_MODE;

//sine wave samples
static const INT16U sineSamples[64] = {0x8000,0x8C8B,0x98F8,0xA528,0xB0FB,0xBC56,
		0xC71C,0xD133,0xDA82,0xE2F2,0xEA6D,0xF0E2,0xF641,0xFA7D,0xFD8A,0xFF62,0xFFFF,
		0xFF62,0xFD8A,0xFA7D,0xF641,0xF0E2,0xEA6D,0xE2F2,0xDA82,0xD133,0xC71C,0xBC56,
		0xB0FB,0xA528,0x98F8,0x8C8B,0x8000,0x7374,0x6707,0x5AD7,0x4F04,0x43A9,0x38E3,
		0x2ECC,0x257D,0x1D0D,0x1592,0xF1D,0x9BE,0x582,0x275,0x9D,0x0,0x9D,0x275,0x582,
		0x9BE,0xF1D,0x1592,0x1D0D,0x257D,0x2ECC,0x38E3,0x43A9,0x4F04,0x5AD7,0x6707,0x7374};

static INT8U PitEventFlag = 0;
static INT8U SineOn = 0;
static INT16U HalfSecTimer = 0;
static SINE_MODE SineCounter = SINE_OFF;
static ALARM_SET_MODE AlarmSetMode = ALARM_ON;

void AlarmWaveInit(void){
	SIM->SCGC6 |= SIM_SCGC6_PIT(1);		//turn on the PIT clock
	SIM->SCGC2 |= SIM_SCGC2_DAC0(1);	//Turn on DAC clock
	PIT->MCR = PIT_MCR_MDIS(0);
	PIT->CHANNEL[0].LDVAL = 3124;		//Ts=1/19200, which comes from (60MHz/(64 samples*300Hz sine wave))-1=3124
	//INITIALIZE DAC (set these three bits to enable software DAC usage with ref. voltage 2 (Vcc))
	DAC0->C0 |= (DAC_C0_DACRFS(1) | DAC_C0_DACTRGSEL(1) | DAC_C0_DACEN(1));
	//start pit timer at 0
	PIT->CHANNEL[0].TCTRL = (PIT_TCTRL_TIE(1)|PIT_TCTRL_TEN(1));
	//Enable pit interrupt
	NVIC_EnableIRQ(PIT0_IRQn);
}

void AlarmWaveControlTask(void){
	DB3_TURN_ON();
	PitEventFlag = 0;
	if (AlarmSetMode == ALARM_ON){
		HalfSecTimer++;
		if (HalfSecTimer == 50){
			SineOn = 1;
		}else if (HalfSecTimer >= 100){
			SineOn = 0;
			HalfSecTimer = 0;
		}else{}
	}else if (AlarmSetMode == ALARM_OFF){
		SineOn = 0;
	}else{}
	DB3_TURN_OFF();
}

void AlarmWaveSetMode(void){
	if (AlarmSetMode == ALARM_ON){	//toggle the alarm mode when we go into AlarmWaveSetMode
		AlarmSetMode = ALARM_OFF;
	}else if (AlarmSetMode == ALARM_OFF){
		AlarmSetMode = ALARM_ON;
	}else{}
	SineOn = SINE_OFF;
	SineCounter = 0;
	//reset the AlarmOnModeToggle to start with the sine output
	HalfSecTimer = 0;
}

void PIT0_IRQHandler(void){
	DB4_TURN_ON();
	PIT->CHANNEL[0].TFLG = PIT_TFLG_TIF(1);		//reset the timer interrupt flag
	PitEventFlag++;			//timer flag in order to run the AlarmWaveControlTask() at the right time
	if (SineOn == SINE_ON){
		SineCounter++;
		if(SineCounter >= 64){
			SineCounter = 0;
		}else{}
		//write sine wave to the DAC
		DAC0->DAT[0].DATL = (INT8U)((sineSamples[SineCounter]>>4) & 0xff);	//chop off the least significant bits of the 16 bit value, to get a 12 bit value, for use in the 12 bit DAC
		DAC0->DAT[0].DATH = (INT8U)(sineSamples[SineCounter]>>12);
	}else if (SineOn == SINE_OFF){
		//write constant 1.65v to the DAC
		DAC0->DAT[0].DATL = (INT8U)((sineSamples[0]>>4) & 0xff);	//sineSamples[0] represents half of the full scale voltage of the DAC, since it is the middle value of the sine wave
		DAC0->DAT[0].DATH = (INT8U)(sineSamples[0]>>12);
	}else{}
	DB4_TURN_OFF();
}
