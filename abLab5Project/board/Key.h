/*****************************************************************************************
* Key.h - The header file for the keypad module, Key.c
* 02/20/2001 TDM Original key.c for 9S12
* 01/14/2013 TDM Modified for K70 custom tower board.
* 11/04/2015 Modified for K65 and improve code standards. Todd Morton
*            Note: When using this module with the K65TWR board, a junper is required
*            from B60 to A64 on the tower. Also, PORTA bit 6 must remain an unsued input.
* 12/08/2015 Changed type for control codes.
* 10/29/2018 Modified for MCUXpresso, Todd Morton
******************************************************************************************
* Public Resources
*****************************************************************************************/
#ifndef KEY_INC
#define KEY_INC

/*****************************************************************************************
* Defines for the alpha keys: A, B, C, D
* These are ASCII control characters
*****************************************************************************************/
#define DC1 (INT8C)0x11     /*ASCII control code for the A button */
#define DC2 (INT8C)0x12     /*ASCII control code for the B button */
#define DC3 (INT8C)0x13     /*ASCII control code for the C button */
#define DC4 (INT8C)0x14     /*ASCII control code for the D button */


/*****************************************************************************************
* KeyGet() - Returns current value of the keypad buffer then clears the buffer.
*            The value returned is the ASCII code for the key pressed or zero if no key
*            was pressed.
*****************************************************************************************/
INT8C KeyGet(void);
                              
/*****************************************************************************************
* KeyInit() - Keypad Initialization. Must run before calling KeyTask.
*****************************************************************************************/
void KeyInit(void);

/*****************************************************************************************
* KeyTask() - The main keypad scanning task. It scans the keypad and updates the keypad
*             buffer if a keypress was verified. This is a cooperative task that must be
*             called with a period between: Tb/2 < Tp < (Tact-Tb)/2
*****************************************************************************************/
void KeyTask(void);

#endif
