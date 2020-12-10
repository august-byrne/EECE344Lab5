/****************************************************************************************
* Key.c - A keypad module for a 4x4 matrix keypad. This version allows for multiple
*         column keys to be pressed and mapped to a different code by changing ColTable[]
*         in KeyScan().
*         Multiple rows can not be resolved. The topmost button will be used.
*         The KeyCoeTable[] is currently set to generate ASCII codes.
* 02/20/2001 TDM Original key.c for 9S12
* 01/14/2013 TDM Modified for K70 custom tower board.
* 11/04/2015 Modified for K65 and improve code standards. Todd Morton
*            Note: When using this module with the K65TWR board, a junper is required
*            from B60 to A64 on the tower. Also, PORTA bit 6 must remain an unsued input.
* 12/08/2015 Changed type for control codes.
* 10/29/2018 Modified for MCUXpresso, Todd Morton
*****************************************************************************************
* Project master header file
****************************************************************************************/
#include "MCUType.h"
#include "Key.h"
#include "K65TWR_GPIO.h"
/****************************************************************************************
* Private Resources
****************************************************************************************/
static INT8U keyScan(void);         /* Makes a single keypad scan  */
static INT8C keyBuffer;             /* Holds the ASCII code for key*/
static const INT8C keyCodeTable[16] =
   {'1','2','3',DC1,'4','5','6',DC2,'7','8','9',DC3,'*','0','#',DC4};
static void keyDly(void);           /* Added for GPIO to settle before read */
/****************************************************************************************
* Module Defines
* This version is designed for the custom LCD/Keypad board, which has the following
* mapping:
*  COL1->PTC3, COL2->PTC4, COL3->PTC5, COL4->PTC6
*  ROW1->PTC7, ROW2->PTC8, ROW3->PTC9, ROW4->PTC10
****************************************************************************************/
typedef enum{KEY_OFF,KEY_EDGE,KEY_VERF} KEYSTATES;
#define KEY_PORT_OUT   GPIOC->PDOR
#define KEY_PORT_DIR   GPIOC->PDDR
#define KEY_PORT_IN	   GPIOC->PDIR
#define COLS_MASK 0x00000078U
#define ROWS_MASK 0x00000780U
#define COLS_IN() (((~KEY_PORT_IN) & COLS_MASK)>>3)
/****************************************************************************************
* KeyGet() - This function provides public access to keyBuffer. It clears keyBuffer
*            after for read-once handshaking. Note: this handshaking method only works
*            when 0x00 is not a valid keycode. If it is a valid keycode then a semaphore
*            should be added.
* - Public
****************************************************************************************/
INT8C KeyGet(void){
    INT8C key;
    key = keyBuffer;
    keyBuffer = '\0';
    return (key);
}

/****************************************************************************************
* KeyInit() - Initialization routine for the keypad module. The columns are normally set
*             as inputs and, since they are pulled high, they are one. Then to pull a row
*             low during scanning, the direction for that pin is changed to an output.
****************************************************************************************/
void KeyInit(void){

    /* Key port init */
    SIM->SCGC5 |= SIM_SCGC5_PORTC(1);     //Make sure that the gated clock for PORTC is on
    PORTC->PCR[3]=PORT_PCR_MUX(1)|PORT_PCR_PS_MASK|PORT_PCR_PE_MASK;   //Columns
    PORTC->PCR[4]=PORT_PCR_MUX(1)|PORT_PCR_PS_MASK|PORT_PCR_PE_MASK;
    PORTC->PCR[5]=PORT_PCR_MUX(1)|PORT_PCR_PS_MASK|PORT_PCR_PE_MASK;
    PORTC->PCR[6]=PORT_PCR_MUX(1)|PORT_PCR_PS_MASK|PORT_PCR_PE_MASK;
    PORTC->PCR[7]=PORT_PCR_MUX(1);     //Rows
    PORTC->PCR[8]=PORT_PCR_MUX(1);
    PORTC->PCR[9]=PORT_PCR_MUX(1);
    PORTC->PCR[10]=PORT_PCR_MUX(1);
    KEY_PORT_OUT &= ~ROWS_MASK;            /* Preset all rows to zero    */
    keyBuffer = '\0';                      /* Init keyBuffer      */
}

/****************************************************************************************
* KeyTask() - Reads the keypad and updates keyBuffer. A task decomposed into states for
*             detecting and verifying keypresses. This task should be called periodically
*             with a period between: Tb/2 < Tp < (Tact-Tb)/2
*             The switch must be released to have multiple acknowledged presses.
* (Public)
****************************************************************************************/
void KeyTask(void) {
	DB2_TURN_ON();
    INT8U cur_key;
    static INT8U last_key = 0;
    static KEYSTATES keyState = KEY_OFF;

    cur_key = keyScan();
    if(keyState == KEY_OFF){    /* Key released state */
        if(cur_key != 0){
            keyState = KEY_EDGE;
        }else{ /* wait for key press */
        }
    }else if(keyState == KEY_EDGE){     /* Keypress detected state*/
        if(cur_key == last_key){        /* Keypress verified */
            keyState = KEY_VERF;
            keyBuffer = keyCodeTable[cur_key - 1]; /*update buffer */
        }else if(cur_key == 0){        /* Unvalidated, start over */
            keyState = KEY_OFF;
        }else{                          /*Unvalidated, diff key edge*/
        }
    }else if(keyState == KEY_VERF){     /* Keypress verified state */
        if((cur_key == 0) || (cur_key != last_key)){
            keyState = KEY_OFF;
        }else{ /* wait for release or key change */
        }
    }else{ /* In case of error */
        keyState = KEY_OFF;             /* Should never get here */
    }
    last_key = cur_key;                 /* Save key for next time */
    DB2_TURN_OFF();
}

/****************************************************************************************
* keyScan() - Scans the keypad and returns a keycode.
*           - Designed for 4x4 keypad with columns pulled high.
*           - Current keycodes follow:
*               1->0x01,2->0x02,3->0x03,A->0x04
*               4->0x05,5->0x06,6->0x07,B->0x08
*               7->0x09,8->0x0A,9->0x0B,C->0x0C
*               *->0x0D,0->0x0E,#->0x0F,D->0x10
*           - Returns zero if no key is pressed.
*           - ColTable[] can be changed to distinguish multiple keys
*             pressed in the same row.
* (Private)
****************************************************************************************/
static INT8U keyScan(void) {

    INT8U kcode;
    INT8U roff;
    INT32U rbit;

    const INT8U ColTable[16] = {0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4};

    rbit = 0x00000080U;
    roff = 0x00U;
    while(rbit != 0){ /* Until all rows are scanned */
        KEY_PORT_OUT &= ~ROWS_MASK;
        KEY_PORT_DIR = (KEY_PORT_DIR & ~ROWS_MASK)|rbit;    /* Pull row low */
        keyDly();	// wait for direction and col inputs to settle
        kcode = (INT8U)(COLS_IN());  /*Read columns */
        KEY_PORT_DIR = (KEY_PORT_DIR &~ROWS_MASK); 
        if(kcode != 0){        /* generate key code if key pressed */
            kcode = (INT8U)(roff + ColTable[kcode]);
            break;
        }else{}
        rbit = ROWS_MASK & (rbit<<1);       /* setup for next row */
        roff = (INT8U)(roff + 4);
    }
    return (kcode); 
}
/****************************************************************************************
 * KeyDly() a software delay for KeyScan() to wait until port row bit direction and
 * column inputs are settled.
 * With debug bits included and a 180Mhz bus the delay is
 * 	Tdly = (55.5ns)i + 85ns
 * Currently set to ~862ns with i=14.
 * TDM 01/20/2013
 * Modified for 180MHz bus. Todd Morton 11/04/2015
 ****************************************************************************************/
static void keyDly(void){
	INT32U i;
	for(i=0;i<14;i++){
	}
}
