/*****************************************************************************************
* Lcd.c
* 
* A set of general purpose LCD utilities. This module should not be used with a preemptive
* kernel without protection of the shared LCD.
*  
* Originally the 9S12 LCD module from Andrew Pace, 2/6/99, ET 454 
* MOdified for the K70. Todd Morton, 2/24/2013 
* Modified for the K65 Todd Morton, 11/03/2015
* Fixed LcdDispDecWord, Todd Morton, 12/13/2017
* Modified for MCUXpresso, Todd Morton, 10/29/2018
* Modified for MCUXpresso v11.2, added new LcdDispDecWord(), Todd Morton, 10/31/2020
******************************************************************************************
* Master Include File  
*****************************************************************************************/
#include "MCUType.h"
#include "LCD.h"

/*****************************************************************************************
* LCD Port Defines 
*****************************************************************************************/
#define LCD_RS_BIT     0x2U
#define LCD_E_BIT      0x4U
#define LCD_DB_MASK    0x78U
#define LCD_PORT       GPIOD->PDOR
#define LCD_PORT_DIR   GPIOD->PDDR
#define INIT_BIT_DIR() (LCD_PORT_DIR |= (LCD_RS_BIT|LCD_E_BIT|LCD_DB_MASK))
#define LCD_SET_RS()   GPIOD->PSOR = LCD_RS_BIT
#define LCD_CLR_RS()   GPIOD->PCOR = LCD_RS_BIT
#define LCD_SET_E()    GPIOD->PSOR = LCD_E_BIT
#define LCD_CLR_E()    GPIOD->PCOR = LCD_E_BIT

/*****************************************************************************************
* LCD Defines
*****************************************************************************************/
#define NUM_CHARS     16      /* 16 character display */
#define LCD_DAT_INIT  0x28    /*Data length: 4 bit. Lines: 2. Font: 5x7 dots.*/
#define LCD_SHIFT_CUR 0x06    /*Increments cursor addr after write.*/
#define LCD_DIS_INIT  0x0C    /*Display: on. Cursor: off. Blink: off */
#define LCD_CLR_CMD   0x01    /*Clear display and move cursor home */

#define LCD_LINE1_ADDR 0x80   /* Display address for line1 column1 */
#define LCD_LINE2_ADDR 0xC0   /* Display address for line2 column1 */
#define LCD_BS_CMD     0x10   /* Move cursor left one space */
#define LCD_FS_CMD     0x14   /* Move cursor right one space */

/*****************************************************************************************
* Private Function prototypes
*****************************************************************************************/
static void lcdWrCmd(const INT8U cmd);
static void lcdDly500ns(void);
static void lcdDly40us(void);
static void lcdDlyms(const INT8U ms);
static void lcdWrNib(INT8U nib);
static INT8C lcdHtoA(INT8U hnib);

/*****************************************************************************************
* Function Definitions
******************************************************************************************
* lcdWrCmd(INT8U cmd) - Private
*  PARAMETERS: cmd - Command to be sent to the LCD
*  DESCRIPTION: Sends a command write sequence to the LCD
*****************************************************************************************/
static void lcdWrCmd(const INT8U cmd) {
      LCD_CLR_RS();                 //Select command
      lcdWrNib(cmd>>4);             //Out most sig nibble
      LCD_SET_E();                  //Pulse E. 230ns min per Seiko doc
      lcdDly500ns();
      LCD_CLR_E();
      lcdDly500ns();                //Wait >1us per Seiko doc
      lcdDly500ns();
      lcdWrNib((cmd & 0x0fu));      //Out least sig nibble
      LCD_SET_E();                  //Pulse E
      lcdDly500ns();
      LCD_CLR_E();
      lcdDly40us();                 //Wait 40us per Seiko doc
      LCD_SET_RS();                 //Set back to data
}

/*****************************************************************************************
* lcdWrNib(INT8U nib) - Private
*  PARAMETERS: nib - 4-bit nibble (upper four bits zero) sent to the LCD
*  DESCRIPTION: Sends a nibble to the LCD
*****************************************************************************************/
static void lcdWrNib(INT8U nib){
    INT8U lnib = nib & 0x0fu;
    GPIOD->PDOR = (GPIOD->PDOR & (INT32U)~LCD_DB_MASK)|((INT32U)lnib << 3);
}

/*****************************************************************************************
* LcdDispInit()
*  PARAMETERS: None
*  DESCRIPTION: Initialises LCD ports to outputs and sends LCD reset sequence per Seiko
*               Data sheet. In this case, 4-bit mode.
*****************************************************************************************/
void LcdDispInit(void) {
    SIM->SCGC5 |= SIM_SCGC5_PORTD(1);
	PORTD->PCR[1] = PORT_PCR_MUX(1);
	PORTD->PCR[2] = PORT_PCR_MUX(1);
	PORTD->PCR[3] = PORT_PCR_MUX(1);
	PORTD->PCR[4] = PORT_PCR_MUX(1);
	PORTD->PCR[5] = PORT_PCR_MUX(1);
	PORTD->PCR[6] = PORT_PCR_MUX(1);
	INIT_BIT_DIR();
    LCD_CLR_E(); 
    LCD_SET_RS();               /*Data select unless in lcdWrCmd()  */
    lcdDlyms(15);               /* LCD requires 15ms delay at powerup */
   
    LCD_CLR_RS();               /*Send first command for RESET sequence*/
    lcdWrNib(0x3u);
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDlyms(5);                /*Wait >4.1ms */
  
    lcdWrNib(0x3u);             /*Repeat */
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDlyms(1);                /*Wait >100us */
  
    lcdWrNib(0x3u);             /* Repeat */
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly40us();               /*Wait >40us*/
  
    lcdWrNib(0x2u);             /*Send last command for RESET sequence*/
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly40us();
  
    lcdWrCmd(LCD_DAT_INIT);     /*Send command for 4-bit mode */
    lcdWrCmd(LCD_SHIFT_CUR);
    lcdWrCmd(LCD_DIS_INIT);
    LcdDispClear();
} 

/*****************************************************************************************
** LcdDispChar() - Public
*
*  PARAMETERS: c - ASCII character to be sent to the LCD
*
*  DESCRIPTION: Displays a character at current LCD address. Assumes that the LCD port is
*               configured for a data write.
*****************************************************************************************/
void LcdDispChar(const INT8C c) {
    lcdWrNib(((INT8U)c >> 4));
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly500ns();
    lcdDly500ns();
    lcdWrNib(((INT8U)c & 0x0fu));
    LCD_SET_E();
    lcdDly500ns();
    LCD_CLR_E();
    lcdDly40us();
}

/*****************************************************************************************
** LcdDispClear
*  PARAMETERS: None
*  DESCRIPTION: Clears the LCD display and returns the cursor to row1, col1.
*****************************************************************************************/
void LcdDispClear(void) {

    lcdWrCmd(LCD_CLR_CMD);
    lcdDlyms(2);
}

/*****************************************************************************************
** LcdDispLineClear
*  PARAMETERS: line - Line to be cleared (1 or 2).
*  DESCRIPTION: Writes spaces to every location in a line and then returns the cursor to
*               column 1 of that line.
*****************************************************************************************/
void LcdDispLineClear(const INT8U line) {
  
   INT8U start_addr; 
   INT8U i;

   if(line == 1){ 
      start_addr = LCD_LINE1_ADDR;  
      lcdWrCmd(start_addr);
      for(i = 0x0; i < (NUM_CHARS); i++) {
         LcdDispChar(' ');
      }
      lcdWrCmd(start_addr);
   }else if(line == 2){ 
      start_addr = LCD_LINE2_ADDR;
      lcdWrCmd(start_addr);
      for(i = 0x0; i < (NUM_CHARS); i++) {
         LcdDispChar(' ');
      }
      lcdWrCmd(start_addr);
   }else{
      /* Input error, do nothing */
   }
}
 
/*****************************************************************************************
* LcdDispHexWord()
*  PARAMETERS: word - word to be displayed.
*              num_nib - number of nibbles to display, from least sig. to most sig.
*  DESCRIPTION: Displays word, in hex.
*****************************************************************************************/
void LcdDispHexWord(const INT32U word, const INT8U num_nib) {
    INT8U currentnib;
    // Limit number of nibbles
    if((num_nib > 0) && (num_nib <= 8)){
        currentnib = num_nib;
        while(currentnib > 0){
            LcdDispChar(lcdHtoA((word>>((currentnib-1)*4))&0x0F));
            currentnib--;
        }
    }else{
        LcdDispString("HexNibError");
    }

}
/*****************************************************************************************
* LcdDispString()
*  PARAMETERS: *strg - pointer to the NULL terminated string to be displayed.
*  DESCRIPTION: Displays the string pointed to by strg.
*****************************************************************************************/
void LcdDispString(INT8C *const strg) {

    INT8C *sptr = (INT8C *)strg;

    while(*sptr != '\0') {
        LcdDispChar(*sptr);
        sptr++;
    }
}

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
*
* Contributions from Brad Cowgill
*********************************************************************************************/
void LcdDispDecWord(INT32U binword, INT8U field, LCD_MODE mode){
    INT8C digitstrg[11];
    INT32U lbinword = binword;
    INT8U num_digits = field;
    INT8U digit_index;
    INT8U val_index;

    //Clamp field size to acceptable values
    if(num_digits > 10){
        num_digits = 10;
    }else if(num_digits < 1){
        num_digits = 1;
    }else{
    }
   //Calculate the digits needed for lbinword
    digit_index = num_digits + 1;
    digit_index--;
    digitstrg[digit_index] = '\0';   //always ends in null
    while((digit_index > 0) && (lbinword > 0)){
        digit_index--;
        digitstrg[digit_index] = (INT8C)((lbinword % 10) +'0');
        lbinword = lbinword/10;
    }

    if(digit_index == num_digits){       //always at least a '0'
        digit_index--;
        digitstrg[digit_index] = '0';
    }else{
    }
    if(lbinword > 0){  //Writes '-' to all field slots if bin length exceeded field
        digit_index = 0;
        while(digit_index < num_digits){
            digitstrg[digit_index] = '-';
            digit_index++;
        }
        digitstrg[digit_index] = '\0';
        LcdDispString(digitstrg);
    }else{
        if((mode == LCD_DEC_MODE_AR) || (mode == LCD_DEC_MODE_LZ)){   //align right so fill rest with spaces to clear
            while(digit_index > 0){
                digit_index--;
                if(mode == LCD_DEC_MODE_AR){
                    digitstrg[digit_index] = ' ';
                }else if(mode == LCD_DEC_MODE_LZ){
                    digitstrg[digit_index] = '0';
                }else{
                }
            }
            LcdDispString(digitstrg);

        }else if(mode == LCD_DEC_MODE_AL){
            val_index = digit_index;
            digit_index = 0;
            while(val_index < (num_digits)){
                digitstrg[digit_index] = digitstrg[val_index];
                val_index++;
                digit_index++;
            }
            while(digit_index < num_digits){
                digitstrg[digit_index] = ' ';
                digit_index++;
            }
            digitstrg[(digit_index)] = '\0';
            LcdDispString(digitstrg);
        }else{
        }
    }
}


/*****************************************************************************************
** LcdCursorMove()
*  PARAMETERS: row - Destination row (1 or 2).
*              col - Destination column (1 - 16).
*  DESCRIPTION: Moves the cursor to [row,col].
*****************************************************************************************/
void LcdCursorMove(const INT8U row, const INT8U col) {

    if(row == 1) {
        lcdWrCmd((INT8U)(LCD_LINE1_ADDR + col - 1));
    }else{
        lcdWrCmd((INT8U)(LCD_LINE2_ADDR + col - 1));
    }
}

/*****************************************************************************************
** LcdCursorMode(INT8U on, INT8U blink)
*  PARAMETERS: on - (Binary)Turn cursor on if TRUE, off if FALSE.
*              blink - (Binary)Cursor blinks if TRUE.
*  Changes LCD cursor state - four possible combinations.
*  RETURNS: None
*****************************************************************************************/
void LcdCursorMode(const INT8U on, const INT8U blink) {

    INT8U curcmd;
    
    if(on == 0){
        curcmd = 0x0CU;     //Cursor off
    }else{
        curcmd = 0x0EU;     //Cursor on
    }
    if(blink != 0) {
        curcmd |= 0x01;     //Cursor blink
    }else{
                            //Cursor no blink
    }
    lcdWrCmd(curcmd);
}

/*****************************************************************************************
** lcdDly500ns(void)
*  	Delays, at least, 500ns
*   Designed for 180MHz max clock.
 * 	Tdly >= (45ns)i (at 180MHz)
 * Currently set to ~540ns with i=12.
 * TDM 01/20/2013
 * TDM 11/03/2015
*****************************************************************************************/
static void lcdDly500ns(void){
	INT32U i;
	for(i=0;i<12;i++){
	}
}

/*****************************************************************************************
** lcdDly40us(void)
* 	Clock frequency independent because it uses lcdDly500ns.
*****************************************************************************************/
static void lcdDly40us(void){
    INT8U cnt;
    for(cnt=80;cnt > 0;cnt--){
        lcdDly500ns();
    }
}

/*****************************************************************************************
** lcdDlyms(INT8U ms)
*   Delays, at least, ms milliseconds. Maximum ~255ms.
*   Note, based on lcdDly500ns() so not very accurate but always greater than ms
*   milliseconds
*****************************************************************************************/
static void lcdDlyms(const INT8U ms){
    INT32U cnt;
    for(cnt = (INT32U)ms*2000;cnt > 0;cnt--){
        lcdDly500ns();
    }
}

/*****************************************************************************************
* LcdBSpace()
*   Moves cursor back one space.
*****************************************************************************************/
void LcdBSpace(void) {
    lcdWrCmd(LCD_BS_CMD);
}

/*****************************************************************************************
* LcdFSpace()
*   Moves cursor right one space.
*****************************************************************************************/
void LcdFSpace(void) {
    lcdWrCmd(LCD_FS_CMD);
}
/*******************************************************************************************
* lcdHtoA() - Converts a hex nibble to ASCII - private
* hnib is the byte with the LSN to be sent
*******************************************************************************************/
static INT8C lcdHtoA(INT8U hnib){
    INT8C asciic;
    INT8U hnmask = hnib & 0x0fu; /* Take care of any upper nibbles */
    if(hnmask <= 9U){
        asciic = (INT8C)(hnmask + 0x30U);
    }else{
        asciic = (INT8C)(hnmask + 0x37U);
    }
    return asciic;
}
/****************************************************************************************/
