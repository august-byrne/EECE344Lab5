/*
 * AlarmWave.c enables the pit and DAC on our board, and sets up a software interrupt-based
 * counter which, depending on the mode, either sends a constant 1.65v out of the DAC,
 * or sends a 300Hz (plus 4 harmonics) sine wave (made from 64 samples) out of the DAC.
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
//sine wave samples for a sine wave with 2nd, 3rd, 4th, and 8th harmonics
static const INT16U alarmSineSamples[64] = {0x8000,0xAAD5,0xC8B7,0xD332,0xCD26,0xC061,
		0xB77B,0xB79E,0xBDCD,0xC145,0xB96D,0xA3CD,0x865B,0x6CB9,0x61EA,0x6A13,0x8000,
		0x97E2,0xA5BA,0xA3C9,0x955A,0x8454,0x7B21,0x7E71,0x8A9A,0x9603,0x972D,0x8AB5,
		0x75BE,0x633E,0x5DB7,0x690C,0x8000,0x96F3,0xA248,0x9CC1,0x8A41,0x754A,0x68D2,
		0x69FC,0x7565,0x818E,0x84DE,0x7BAB,0x6AA5,0x5C36,0x5A45,0x681D,0x8000,0x95EC,
		0x9E15,0x9346,0x79A4,0x5C32,0x4692,0x3EBA,0x4232,0x4861,0x4884,0x3F9E,0x32D9,
		0x2CCD,0x3748,0x552A};

static INT8U PitEventFlag = 0;
static SINE_MODE SineOn = SINE_OFF;
static INT8U SineCounter = 0;
static ALARM_SET_MODE AlarmSetMode = ALARM_OFF;

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
		SineOn = SINE_ON;
	}else if (AlarmSetMode == ALARM_OFF){
		SineOn = SINE_OFF;
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
		DAC0->DAT[0].DATL = (INT8U)((alarmSineSamples[SineCounter]>>4) & 0xff);	//chop off the least significant bits of the 16 bit value, to get a 12 bit value, for use in the 12 bit DAC
		DAC0->DAT[0].DATH = (INT8U)(alarmSineSamples[SineCounter]>>12);
	}else if (SineOn == SINE_OFF){
		//write constant 1.65v to the DAC
		DAC0->DAT[0].DATL = (INT8U)((alarmSineSamples[0]>>4) & 0xff);	//sineSamples[0] represents half of the full scale voltage of the DAC, since it is the middle value of the sine wave
		DAC0->DAT[0].DATH = (INT8U)(alarmSineSamples[0]>>12);
	}else{}
	DB4_TURN_OFF();
}
