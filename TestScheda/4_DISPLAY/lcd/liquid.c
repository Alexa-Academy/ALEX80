#include "liquid.h"

#include <stdio.h>
#include <string.h>


uint8_t LiquidCrystal_rs_pin;
uint8_t LiquidCrystal_rw_pin;
uint8_t LiquidCrystal_enable_pin;
uint8_t LiquidCrystal_data_pins[8];
uint8_t LiquidCrystal_displayfunction;
uint8_t LiquidCrystal_displaycontrol;
uint8_t LiquidCrystal_displaymode;
uint8_t LiquidCrystal_row_offsets[4];
uint8_t LiquidCrystal_numlines;


void pio_b_out();

char portB_value = 0;

void pinMode(uint8_t pin, uint8_t mode) {
}

void delayMicroseconds(unsigned int us) {
}

void digitalWrite(uint8_t pin, uint8_t val) {
  if (val) {
    portB_value |= pin;
    pio_b_out(portB_value);
  } else {
    portB_value &= ~pin;
    pio_b_out(portB_value);
  }
}



unsigned long write_string(const uint8_t *buffer, unsigned long size)
{
  unsigned long n = 0;
  while (size--) {
    if (LiquidCrystal_write(*buffer++)) n++;
    else break;
  }
  return n;
}


void LiquidCrystal_send(uint8_t value, uint8_t mode);

void LiquidCrystal_pulseEnable();

void LiquidCrystal_write4bits(uint8_t value);

void LiquidCrystal_write8bits(uint8_t value);

void LiquidCrystal_c1(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  LiquidCrystal_init(0, rs, rw, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

void LiquidCrystal_c2(uint8_t rs, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
			     uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
  LiquidCrystal_init(0, rs, 255, enable, d0, d1, d2, d3, d4, d5, d6, d7);
}

void LiquidCrystal_c3(uint8_t rs, uint8_t rw, uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  LiquidCrystal_init(1, rs, rw, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}

void LiquidCrystal_c4(uint8_t rs,  uint8_t enable,
			     uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3)
{
  LiquidCrystal_init(1, rs, 255, enable, d0, d1, d2, d3, 0, 0, 0, 0);
}


void LiquidCrystal_init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
                        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7)
{
    LiquidCrystal_rs_pin = rs;
    LiquidCrystal_rw_pin = rw;
    LiquidCrystal_enable_pin = enable;

    LiquidCrystal_data_pins[0] = d0;
    LiquidCrystal_data_pins[1] = d1;
    LiquidCrystal_data_pins[2] = d2;
    LiquidCrystal_data_pins[3] = d3;
    LiquidCrystal_data_pins[4] = d4;
    LiquidCrystal_data_pins[5] = d5;
    LiquidCrystal_data_pins[6] = d6;
    LiquidCrystal_data_pins[7] = d7;

    if (fourbitmode)
        LiquidCrystal_displayfunction = LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
    else
        LiquidCrystal_displayfunction = LCD_FUNCTIONSET | LCD_8BITMODE | LCD_1LINE | LCD_5x8DOTS;

    LiquidCrystal_begin(16, 1, LCD_5x8DOTS);
}

void LiquidCrystal_begin(uint8_t cols, uint8_t lines, uint8_t dotsize)
{
    if (lines > 1)
    {
        LiquidCrystal_displayfunction |= LCD_2LINE;
    }
    LiquidCrystal_numlines = lines;

    LiquidCrystal_setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

    // for some 1 line displays you can select a 10 pixel high font
    if ((dotsize != LCD_5x8DOTS) && (lines == 1))
    {
        LiquidCrystal_displayfunction |= LCD_5x10DOTS;
    }

    pinMode(LiquidCrystal_rs_pin, OUTPUT);
    // we can save 1 pin by not using RW. Indicate by passing 255 instead of pin#
    if (LiquidCrystal_rw_pin != 255) { 
      pinMode(LiquidCrystal_rw_pin, OUTPUT);
    }
    pinMode(LiquidCrystal_enable_pin, OUTPUT);
    
    // Do these once, instead of every time a character is drawn for speed reasons.
    for (int i=0; i<((LiquidCrystal_displayfunction & LCD_8BITMODE) ? 8 : 4); ++i)
    {
      pinMode(LiquidCrystal_data_pins[i], OUTPUT);
    } 

    // SEE PAGE 45/46 FOR INITIALIZATION SPECIFICATION!
    // according to datasheet, we need at least 40 ms after power rises above 2.7 V
    // before sending commands. Arduino can turn on way before 4.5 V so we'll wait 50
    delayMicroseconds(50000); 
    // Now we pull both RS and R/W low to begin commands
    digitalWrite(LiquidCrystal_rs_pin, LOW);
    digitalWrite(LiquidCrystal_enable_pin, LOW);
    if (LiquidCrystal_rw_pin != 255) { 
      digitalWrite(LiquidCrystal_rw_pin, LOW);
    }
    
    //put the LCD into 4 bit or 8 bit mode
    if (! (LiquidCrystal_displayfunction & LCD_8BITMODE)) {
      // this is according to the Hitachi HD44780 datasheet
      // figure 24, pg 46

      // we start in 8bit mode, try to set 4 bit mode
      LiquidCrystal_write4bits(0x03);
      delayMicroseconds(4500); // wait min 4.1ms

      // second try
      LiquidCrystal_write4bits(0x03);
      delayMicroseconds(4500); // wait min 4.1ms
      
      // third go!
      LiquidCrystal_write4bits(0x03); 
      delayMicroseconds(150);

      // finally, set to 4-bit interface
      LiquidCrystal_write4bits(0x02); 
    } else {
      // this is according to the Hitachi HD44780 datasheet
      // page 45 figure 23

      // Send function set command sequence
      LiquidCrystal_command(LCD_FUNCTIONSET | LiquidCrystal_displayfunction);
      delayMicroseconds(4500);  // wait more than 4.1 ms

      // second try
      LiquidCrystal_command(LCD_FUNCTIONSET | LiquidCrystal_displayfunction);
      delayMicroseconds(150);

      // third go
      LiquidCrystal_command(LCD_FUNCTIONSET | LiquidCrystal_displayfunction);
    }

    // finally, set # lines, font size, etc.
    LiquidCrystal_command(LCD_FUNCTIONSET | LiquidCrystal_displayfunction);  

    // turn the display on with no cursor or blinking default
    LiquidCrystal_displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
    LiquidCrystal_display();

    // clear it off
    LiquidCrystal_clear();

    // Initialize to default text direction (for romance languages)
    LiquidCrystal_displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    // set the entry mode
    LiquidCrystal_command(LCD_ENTRYMODESET | LiquidCrystal_displaymode);
}

void LiquidCrystal_setRowOffsets(int row0, int row1, int row2, int row3)
{
    LiquidCrystal_row_offsets[0] = row0;
    LiquidCrystal_row_offsets[1] = row1;
    LiquidCrystal_row_offsets[2] = row2;
    LiquidCrystal_row_offsets[3] = row3;
}

void LiquidCrystal_clear()
{
    LiquidCrystal_command(LCD_CLEARDISPLAY);
    delayMicroseconds(2000);
}

void LiquidCrystal_home()
{
    LiquidCrystal_command(LCD_RETURNHOME);
    delayMicroseconds(2000);
}

void LiquidCrystal_setCursor(uint8_t col, uint8_t row)
{
    const size_t max_lines = sizeof(LiquidCrystal_row_offsets) / sizeof(*LiquidCrystal_row_offsets);
    if (row >= max_lines)
    {
        row = max_lines - 1;
    }
    if (row >= LiquidCrystal_numlines)
    {
        row = LiquidCrystal_numlines - 1;
    }

    LiquidCrystal_command(LCD_SETDDRAMADDR | (col + LiquidCrystal_row_offsets[row]));
}

void LiquidCrystal_noDisplay()
{
    LiquidCrystal_displaycontrol &= ~LCD_DISPLAYON;
    LiquidCrystal_command(LCD_DISPLAYCONTROL | LiquidCrystal_displaycontrol);
}

void LiquidCrystal_display()
{
    LiquidCrystal_displaycontrol |= LCD_DISPLAYON;
    LiquidCrystal_command(LCD_DISPLAYCONTROL | LiquidCrystal_displaycontrol);
}

// Turns the underline cursor on/off
void LiquidCrystal_noCursor() {
  LiquidCrystal_displaycontrol &= ~LCD_CURSORON;
  LiquidCrystal_command(LCD_DISPLAYCONTROL | LiquidCrystal_displaycontrol);
}
void LiquidCrystal_cursor() {
  LiquidCrystal_displaycontrol |= LCD_CURSORON;
  LiquidCrystal_command(LCD_DISPLAYCONTROL | LiquidCrystal_displaycontrol);
}

// Turn on and off the blinking cursor
void LiquidCrystal_noBlink() {
  LiquidCrystal_displaycontrol &= ~LCD_BLINKON;
  LiquidCrystal_command(LCD_DISPLAYCONTROL | LiquidCrystal_displaycontrol);
}
void LiquidCrystal_blink() {
  LiquidCrystal_displaycontrol |= LCD_BLINKON;
  LiquidCrystal_command(LCD_DISPLAYCONTROL | LiquidCrystal_displaycontrol);
}

// These commands scroll the display without changing the RAM
void LiquidCrystal_scrollDisplayLeft(void) {
  LiquidCrystal_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void LiquidCrystal_scrollDisplayRight(void) {
  LiquidCrystal_command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void LiquidCrystal_leftToRight(void) {
  LiquidCrystal_displaymode |= LCD_ENTRYLEFT;
  LiquidCrystal_command(LCD_ENTRYMODESET | LiquidCrystal_displaymode);
}

// This is for text that flows Right to Left
void LiquidCrystal_rightToLeft(void) {
  LiquidCrystal_displaymode &= ~LCD_ENTRYLEFT;
  LiquidCrystal_command(LCD_ENTRYMODESET | LiquidCrystal_displaymode);
}

// This will 'right justify' text from the cursor
void LiquidCrystal_autoscroll(void) {
  LiquidCrystal_displaymode |= LCD_ENTRYSHIFTINCREMENT;
  LiquidCrystal_command(LCD_ENTRYMODESET | LiquidCrystal_displaymode);
}

// This will 'left justify' text from the cursor
void LiquidCrystal_noAutoscroll(void) {
  LiquidCrystal_displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  LiquidCrystal_command(LCD_ENTRYMODESET | LiquidCrystal_displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void LiquidCrystal_createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  LiquidCrystal_command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    LiquidCrystal_write(charmap[i]);
  }
}

/*********** mid level commands, for sending data/cmds */

void LiquidCrystal_command(uint8_t value) {
  LiquidCrystal_send(value, LOW);
}

unsigned long LiquidCrystal_write(unsigned long value) {
  LiquidCrystal_send(value, HIGH);
  return 1; // assume success
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void LiquidCrystal_send(uint8_t value, uint8_t mode) {
  digitalWrite(LiquidCrystal_rs_pin, mode);

  // if there is a RW pin indicated, set it low to Write
  if (LiquidCrystal_rw_pin != 255) { 
    digitalWrite(LiquidCrystal_rw_pin, LOW);
  }
  
  if (LiquidCrystal_displayfunction & LCD_8BITMODE) {
    LiquidCrystal_write8bits(value); 
  } else {
    LiquidCrystal_write4bits(value>>4);
    LiquidCrystal_write4bits(value);
  }
}

void LiquidCrystal_pulseEnable(void) {
  digitalWrite(LiquidCrystal_enable_pin, LOW);
  delayMicroseconds(1);    
  digitalWrite(LiquidCrystal_enable_pin, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450 ns
  digitalWrite(LiquidCrystal_enable_pin, LOW);
  delayMicroseconds(100);   // commands need >37 us to settle
}

void LiquidCrystal_write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    digitalWrite(LiquidCrystal_data_pins[i], (value >> i) & 0x01);
  }

  LiquidCrystal_pulseEnable();
}

void LiquidCrystal_write8bits(uint8_t value) {
  for (int i = 0; i < 8; i++) {
    digitalWrite(LiquidCrystal_data_pins[i], (value >> i) & 0x01);
  }
  
  LiquidCrystal_pulseEnable();
}