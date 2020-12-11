/*******************************************************************************
* EECE344 Lab 5 Code
*	This program allows the user to arm, disarm, and alarm an 'alarm and led'-based
*	security system. The state of the security system is displayed on the LCD,
*	along with the checksum of the program, and you can switch between armed and
*	disarmed with the press of either A or D on the K65TWR's keypad.
* August Byrne, 12/10/2020
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "MemTest.h"
#include "K65TWR_ClkCfg.h"
#include "K65TWR_GPIO.h"
#include "Key.h"
#include "LCD.h"
#include "SysTickDelay.h"
#include "AlarmWave.h"
#include "K65TWR_TSI.h"

#define POLL_PERIOD 10
#define LOWADDR (INT32U) 0x00000000		//low memory address
#define HIGHADRR (INT32U) 0x001FFFFF		//high memory address

static void ControlDisplayTask(void);
static void SensorTask(void);
static void LEDTask(void);

typedef enum {NO_TOUCH, PAD_1, PAD_2} PAD_TOUCH;
typedef enum {ALARM_DISARMED, ALARM_ARMED, ALARM_ON} ALARM_STATE;
static ALARM_STATE CurrentAlarmState = ALARM_DISARMED;	//initial state is alarm off
static ALARM_STATE PreviousAlarmState = ALARM_ON;
static PAD_TOUCH TSIPadTouched = NO_TOUCH;
static INT16U MiliSecTimer = 0;
static INT16U TSIFlagsValue = 0;

void main(void){
	INT16U math_val = 0;
	K65TWR_BootClock();             /* Initialize MCU clocks                  */
	SysTickDlyInit();
	GpioDBugBitsInit();
	LcdDispInit();
	KeyInit();
	AlarmWaveInit();
	TSIInit();
	GpioLED8Init();
	GpioLED9Init();

	//Initial program checksum, which is displayed on the second row of the LCD
	LcdCursorMove(2,1);
	math_val = CalcChkSum((INT8U *)LOWADDR,(INT8U *)HIGHADRR);
	LcdDispString("CS: ");
	LcdDispHexWord(math_val,4);
	LcdCursorMove(1,1);

	while(1){
		SysTickWaitEvent(POLL_PERIOD);
		ControlDisplayTask();
		AlarmWaveControlTask();
		KeyTask();
		SensorTask();
		LEDTask();
	}
}

//handles all TSI scanning
static void SensorTask(void){
	TSITask();
	TSIFlagsValue = TSIGetSensorFlags();
}

//handles all LED control
static void LEDTask(void){
	switch (CurrentAlarmState){
	case ALARM_DISARMED:
		if (TouchSensorValue == 1){
			MiliSecTimer++;
			if (MiliSecTimer >= 25){	//need a period of 500ms, so toggle every 250ms
				LED8_TOGGLE();
				LED9_TOGGLE();
				MiliSecTimer = 0;
			}else{}
		}else{}
		break;
	case ALARM_ARMED:
		MiliSecTimer++;
		if (MiliSecTimer == 25){	//need a period of 500ms, so swap every 250ms
			LED8_OFF();
			LED9_ON();
		}else if (MiliSecTimer >= 50){	//need a period of 500ms, so swap every 250ms
			LED8_ON();
			LED9_OFF();
			MiliSecTimer = 0;
		}else{}
		break;
	case ALARM_ON:
		MiliSecTimer++;
		if (MiliSecTimer >= 5){	//need a period of 100ms, so toggle every 50ms
			if (TSIPadTouched == PAD_1){
				LED8_TOGGLE();
			}else{}
			if (TSIPadTouched == PAD_2){
				LED9_TOGGLE();
			}else{}
			MiliSecTimer = 0;
		}
		break;
	default:
		LED8_TURN_OFF();
		LED9_TURN_OFF();
	}
}

/****************************************************************************************
* ControlDisplayTask() - A task that poles for keypad presses (using KeyGet()) and
*             updates the alarm mode (by using AlarmWaveSetMode() to toggle the alarm mode)
*             and the LCD message accordingly.
* (private)
****************************************************************************************/
static void ControlDisplayTask(void){
	DB1_TURN_ON();
	switch (CurrentAlarmState){
	case ALARM_DISARMED:
		if (PreviousAlarmState != CurrentAlarmState){		//display "alarm off" on the LCD
			LcdDispLineClear(1);
			LcdDispString("DISARMED");
			if (PreviousAlarmState == ALARM_ON){
				AlarmWaveSetMode();			//toggle the alarm wave mode
			}else{}
			PreviousAlarmState = CurrentAlarmState;
			TSIPadTouched = NO_TOUCH;	//reset the pad touch memory
		}else{}
		if (KeyGet() == DC1){			//if a is pressed, set alarm state as armed
			CurrentAlarmState = ALARM_ARMED;
		}else{}
		break;
	case ALARM_ARMED:
		if (PreviousAlarmState != CurrentAlarmState){		//display "alarm on" on the LCD
			LcdDispLineClear(1);
			LcdDispString("ARMED");
			PreviousAlarmState = CurrentAlarmState;
		}else{}
		if (KeyGet() == DC4){			//if d is pressed, set alarm state as disarmed
			CurrentAlarmState = ALARM_DISARMED;
		}else{}
		if ((TouchSensorValue & (1<<BRD_PAD1_CH)) != 0){
			TSIPadTouched = PAD_1;
			CurrentAlarmState = ALARM_ON;
		}else if ((TouchSensorValue & (1<<BRD_PAD2_CH)) != 0){
			TSIPadTouched = PAD_2;
			CurrentAlarmState = ALARM_ON;
		}else{
			//do nothing, since we only want to remove the memory of the pad touch once we become disarmed
		}
		break;
	case ALARM_ON:
		if (PreviousAlarmState != CurrentAlarmState){		//display "alarm on" on the LCD
			LcdDispLineClear(1);
			LcdDispString("ALARM");
			PreviousAlarmState = CurrentAlarmState;
			AlarmWaveSetMode();			//toggle the alarm wave mode
		}else{}
		if (KeyGet() == DC4){			//if d is pressed, set alarm state as disarmed
			CurrentAlarmState = ALARM_DISARMED;
		}else{}
		break;
	default:
		CurrentAlarmState = ALARM_DISARMED;
		PreviousAlarmState = ALARM_ON;
	}
	DB1_TURN_OFF();
}
