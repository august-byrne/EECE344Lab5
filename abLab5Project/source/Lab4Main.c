/*******************************************************************************
* EECE344 Lab 4 Code
*	This program allows the user to activate and deactivate an alarm tone. The
*	choice is displayed on the LCD, along with the checksum of the program, and
*	you can switch between alarm off and alarm on with the press of either A or D
*	on the K65TWR's keypad.
* August Byrne, 11/20/2020, edited on 12/10/2020
*******************************************************************************/
#include "MCUType.h"               /* Include header files                    */
#include "MemTest.h"
#include "K65TWR_ClkCfg.h"
#include "K65TWR_GPIO.h"
#include "Key.h"
#include "LCD.h"
#include "SysTickDelay.h"
#include "AlarmWave.h"

#define POLL_PERIOD 10
#define LOWADDR (INT32U) 0x00000000		//low memory address
#define HIGHADRR (INT32U) 0x001FFFFF		//high memory address

static void ControlDisplayTask(void);

typedef enum {ALARM_OFF, ALARM_ON} ALARM_STATE;
static ALARM_STATE CurrentAlarmState = ALARM_OFF;	//initial state is alarm off
static ALARM_STATE PreviousAlarmState = ALARM_ON;

void main(void){
	INT16U math_val = 0;
	K65TWR_BootClock();             /* Initialize MCU clocks                  */
	SysTickDlyInit();
	GpioDBugBitsInit();
	LcdDispInit();
	KeyInit();
	AlarmWaveInit();

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
	case ALARM_OFF:
		if (PreviousAlarmState != CurrentAlarmState){		//display "alarm off" on the LCD
			LcdDispLineClear(1);
			LcdDispString("ALARM OFF");
			PreviousAlarmState = CurrentAlarmState;
			AlarmWaveSetMode();			//toggle the alarm wave mode
		}else{}
		if (KeyGet() == DC1){			//if a is pressed, set alarm state as on
			CurrentAlarmState = ALARM_ON;
		}else{}
		break;
	case ALARM_ON:
		if (PreviousAlarmState != CurrentAlarmState){		//display "alarm on" on the LCD
			LcdDispLineClear(1);
			LcdDispString("ALARM ON");
			PreviousAlarmState = CurrentAlarmState;
			AlarmWaveSetMode();			//toggle the alarm wave mode
		}else{}
		if (KeyGet() == DC4){			//if d is pressed, set alarm state as off
			CurrentAlarmState = ALARM_OFF;
		}else{}
		break;
	default:
		CurrentAlarmState = ALARM_OFF;
		PreviousAlarmState = ALARM_ON;
	}
	DB1_TURN_OFF();
}
