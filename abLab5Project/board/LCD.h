/*****************************************************************************************
** Lcd.h
* A set of general purpose LCD utilities. This module should not be used with a preemptive
* kernel without protection of the shared LCD.
*
* Originally the 9S12 LCD module from Andrew Pace, 2/6/99, ET 454
* Modified for the K70. Todd Morton, 2/24/2013
* Modified for the K65 Todd Morton, 11/03/2015
* Fixed LcdDispDecWord, Todd Morton, 12/13/2017
* Modified for MCUXpresso, Todd Morton, 10/29/2018
* Modified for MCUXpresso v11.2, added new LcdDispDecWord(), Todd Morton, 10/31/2020
*****************************************************************************************/
#ifndef LCD_INC
#define LCD_INC
/*************************************************************************
* LCD Rows and Columns Defines
*************************************************************************/
// LCD ROWS
#define LCD_ROW_1 1
#define LCD_ROW_2 2

// LCD COLUMNS
#define LCD_COL_1 1
#define LCD_COL_2 2
#define LCD_COL_3 3
#define LCD_COL_4 4
#define LCD_COL_5 5
#define LCD_COL_6 6
#define LCD_COL_7 7
#define LCD_COL_8 8
#define LCD_COL_9 9
#define LCD_COL_10 10
#define LCD_COL_11 11
#define LCD_COL_12 12
#define LCD_COL_13 13
#define LCD_COL_14 14
#define LCD_COL_15 15
#define LCD_COL_16 16

/*************************************************************************
* Enumerated type for mode parameter in LcdDispDecWord()
*************************************************************************/

typedef enum {
    LCD_DEC_MODE_LZ,
    LCD_DEC_MODE_AR,
    LCD_DEC_MODE_AL
} LCD_MODE;

/*****************************************************************************************
* WWULCD Function prototypes
*****************************************************************************************/
/*****************************************************************************************
* LcdDispInit() Initializes display. Takes ~24ms to run
*****************************************************************************************/
void LcdDispInit(void);

/*****************************************************************************************
** LcdDispClear
*  PARAMETERS: None
*  DESCRIPTION: Clears the LCD display and returns the cursor to row1, col1.
*****************************************************************************************/
void LcdDispClear(void);

/*****************************************************************************************
 * LcdDispLineClear
*  PARAMETERS: line - Line to be cleared (1 or 2).
*  DESCRIPTION: Writes spaces to every location in a line and then returns the cursor to
*               column 1 of that line.
*****************************************************************************************/
void LcdDispLineClear(const INT8U line);

/*****************************************************************************************
** LcdDispChar() - Public
*  PARAMETERS: c - ASCII character to be sent to the LCD
*  DESCRIPTION: Displays a character at current LCD address. Assumes that the LCD port is
*               configured for a data write.
*****************************************************************************************/
void LcdDispChar(const INT8C c);

/*****************************************************************************************
* LcdDispHexWord()
*  PARAMETERS: word - word to be displayed.
*              num_nib - number of nibbles to display, from least sig. to most sig.
*  DESCRIPTION: Displays the word, word, in hex.
*****************************************************************************************/
void LcdDispHexWord(const INT32U word, const INT8U num_nib);

/*****************************************************************************************
* LcdDispString()
*  PARAMETERS: *strg - pointer to the NULL terminated string to be displayed.
*  DESCRIPTION: Displays the string pointed to by strg.
*****************************************************************************************/
void LcdDispString(INT8C *const strg);

/*********************************************************************************************
* LcdDispDecWord() - Outputs a decimal value of a 32-bit word.
*    Parameters: binword - is the word to be sent,
*                field - is the maximum number of digits to be shown on LCD. Range 1-10. field
*                   starts at the current column.
*                mode - determines the behavior of field and binword,
*                3 modes:
*                   1. MODE_LZ: Shows leading zeros (digits will be present in entire field).
*                   2. MODE_AR: Aligns binword to rightmost field digits.
*                   3. MODE_AL: Aligns binword to leftmost field digits.
*    Examples:
*    binword = 123, field = 5, mode = MODE_LZ, Result: 00123
*    binword = 123, field = 5, mode = MODE_AR, Result: XX123 (Xs are spaces)
*    binword = 123, field = 5, mode = MODE_AL, Result: 123XX
*    binword = 123, field = 2, mode = MODE_LZ, Result: --    (binword exceeds field)
*********************************************************************************************/
void LcdDispDecWord(INT32U binword, INT8U field, LCD_MODE mode);

/*****************************************************************************************
** LcdCursorMove()
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*  DESCRIPTION: Moves the cursor to [row,col].
*****************************************************************************************/
void LcdCursorMove(const INT8U row, const INT8U col);
                                    
/*****************************************************************************************
** LcdCursorMode(INT8U on, INT8U blink)
*  PARAMETERS: on - (Binary)Turn cursor on if TRUE, off if FALSE.
*              blink - (Binary)Cursor blinks if TRUE.
*  Changes LCD cursor state - four possible combinations.
*  RETURNS: None
*****************************************************************************************/
void LcdCursorMode(const INT8U on, const INT8U blink);

/*****************************************************************************************
* LcdBSpace()
*   Moves cursor back one space.
*****************************************************************************************/
void LcdBSpace(void);

/*****************************************************************************************
* LcdFSpace()
*   Moves cursor right one space.
*****************************************************************************************/
void LcdFSpace(void);

/****************************************************************************************/
#endif
