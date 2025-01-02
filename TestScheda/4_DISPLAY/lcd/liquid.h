#ifndef LIQUID_H
#define LIQUID_H

#define uint8_t char
#define HIGH 1
#define LOW 0
#define OUTPUT 0

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void pinMode(uint8_t pin, uint8_t mode);
void delayMicroseconds(unsigned int us);
void digitalWrite(uint8_t pin, uint8_t val);

unsigned long write_string(const uint8_t *buffer, unsigned long size);

void LiquidCrystal_c1(uint8_t rs, uint8_t rw, uint8_t enable, 
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
void LiquidCrystal_c2(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
void LiquidCrystal_c3(uint8_t rs, uint8_t rw, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);
void LiquidCrystal_c4(uint8_t rs, uint8_t enable,
  uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3);


void LiquidCrystal_init(uint8_t fourbitmode, uint8_t rs, uint8_t rw, uint8_t enable,
                        uint8_t d0, uint8_t d1, uint8_t d2, uint8_t d3,
                        uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7);
void LiquidCrystal_begin(uint8_t cols, uint8_t lines, uint8_t dotsize);   // default LCD_5x8DOTS
void LiquidCrystal_clear();
void LiquidCrystal_home();
void LiquidCrystal_noDisplay();
void LiquidCrystal_display();
void LiquidCrystal_noBlink();
void LiquidCrystal_blink();
void LiquidCrystal_noCursor();
void LiquidCrystal_cursor();
void LiquidCrystal_scrollDisplayLeft();
void LiquidCrystal_scrollDisplayRight();
void LiquidCrystal_leftToRight();
void LiquidCrystal_rightToLeft();
void LiquidCrystal_autoscroll();
void LiquidCrystal_noAutoscroll();

void LiquidCrystal_setRowOffsets(int row0, int row1, int row2, int row3);
void LiquidCrystal_createChar(uint8_t location, uint8_t charmap[]);
void LiquidCrystal_setCursor(uint8_t col, uint8_t row);
unsigned long LiquidCrystal_write(unsigned long value);
void LiquidCrystal_command(uint8_t value);

#endif   // LIQUID_H