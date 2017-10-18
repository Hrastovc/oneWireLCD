#include "oneWireLCD.h"

#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include "Arduino.h"
#include "OneShift.h"

// When the display powers up, it is configured as follows:
//
// 1. Display clear
// 2. Function set: 
//    DL = 1; 8-bit interface data 
//    N = 0; 1-line display 
//    F = 0; 5x8 dot character font 
// 3. Display on/off control: 
//    D = 0; Display off 
//    C = 0; Cursor off 
//    B = 0; Blinking off 
// 4. Entry mode set: 
//    I/D = 1; Increment by 1 
//    S = 0; No shift 
//
// Note, however, that resetting the Arduino doesn't reset the LCD, so we
// can't assume that its in that state when a sketch starts (and the
// oneWireLCD constructor is called).

OneShift Shift;

oneWireLCD::oneWireLCD(uint8_t pin)
{
  init(pin, 255, 255, 255, 7, 6, 5, 4, 3, 2, 1);
}

oneWireLCD::oneWireLCD(uint8_t pin,
                       uint8_t rs_pin, uint8_t enable_pin, uint8_t D4,
                       uint8_t D5, uint8_t D6, uint8_t D7, uint8_t backlight_pin)
{
  init(pin, 255, 255, 255, rs_pin, enable_pin, D4, D5, D6, D7, backlight_pin);
}

oneWireLCD::oneWireLCD(uint8_t clockPin, uint8_t dataPin, uint8_t latchPin)
{
  init(255, clockPin, dataPin, latchPin, 7, 6, 5, 4, 3, 2, 1);
}

oneWireLCD::oneWireLCD(uint8_t clockPin, uint8_t dataPin, uint8_t latchPin,
                       uint8_t rs_pin, uint8_t enable_pin, uint8_t D4,
                       uint8_t D5, uint8_t D6, uint8_t D7, uint8_t backlight_pin)
{
  init(255, clockPin, dataPin, latchPin, rs_pin, enable_pin, D4, D5, D6, D7, backlight_pin);
}

void oneWireLCD::init(uint8_t pin, uint8_t clockPin, uint8_t dataPin, uint8_t latchPin,
                      uint8_t rs_pin, uint8_t enable_pin, uint8_t D4, uint8_t D5,
                      uint8_t D6, uint8_t D7, uint8_t backlight_pin)
{
  if(pin != 255){
    Shift.init(pin);
    singleWire = 1;
  }else{
    pinMode(clockPin, OUTPUT);
    pinMode(dataPin, OUTPUT);
    pinMode(latchPin, OUTPUT);
    _clockPin = clockPin;
    _dataPin = dataPin;
    _latchPin = latchPin;
    singleWire = 0;
  }
  
  _rs_pin = rs_pin;
  _enable_pin = enable_pin;

  _data_pins[0] = D4;
  _data_pins[1] = D5;
  _data_pins[2] = D6;
  _data_pins[3] = D7;

  _backlight_pin = backlight_pin;
  
  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;
  
  begin(16, 1);

  backlight(HIGH);
}

void oneWireLCD::begin(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }
  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);  

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  if(singleWire > 0){
    Shift.Out(0x00);
  }else{
    digitalWrite(_latchPin, LOW);
    shiftOut(_dataPin, _clockPin, MSBFIRST, 0x00);   
    digitalWrite(_latchPin, HIGH);
  }

  delayMicroseconds(50000);
  
  // this is according to the hitachi HD44780 datasheet
  // figure 24, pg 46

  // we start in 8bit mode, try to set 4 bit mode
  write4bits(0x03);
  delayMicroseconds(4500); // wait min 4.1ms

  // second try
  write4bits(0x03);
  delayMicroseconds(4500); // wait min 4.1ms
  
  // third go!
  write4bits(0x03); 
  delayMicroseconds(150);

  // finally, set to 4-bit interface
  write4bits(0x02); 

  // finally, set # lines, font size, etc.
  command(LCD_FUNCTIONSET | _displayfunction);

  // turn the display on with no cursor or blinking default
  _displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;  
  display();

  // clear it off
  clear();

  // Initialize to default text direction (for romance languages)
  _displaymode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  // set the entry mode
  command(LCD_ENTRYMODESET | _displaymode);

}

void oneWireLCD::setRowOffsets(int row0, int row1, int row2, int row3)
{
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void oneWireLCD::clear()
{
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void oneWireLCD::home()
{
  command(LCD_RETURNHOME);  // set cursor position to zero
  delayMicroseconds(2000);  // this command takes a long time!
}

void oneWireLCD::setCursor(uint8_t col, uint8_t row)
{
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if ( row >= max_lines ) {
    row = max_lines - 1;    // we count rows starting w/0
  }
  if ( row >= _numlines ) {
    row = _numlines - 1;    // we count rows starting w/0
  }
  
  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void oneWireLCD::noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void oneWireLCD::display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void oneWireLCD::noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void oneWireLCD::cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void oneWireLCD::noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void oneWireLCD::blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void oneWireLCD::scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void oneWireLCD::scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void oneWireLCD::leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void oneWireLCD::rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void oneWireLCD::autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void oneWireLCD::noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void oneWireLCD::createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7; // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i=0; i<8; i++) {
    write(charmap[i]);
  }
}

void oneWireLCD::backlight(uint8_t state) {
  MODdigitalWrite(_backlight_pin, state);
}

/*********** mid level commands, for sending data/cmds */

inline void oneWireLCD::command(uint8_t value) {
  send(value, LOW);
}

inline size_t oneWireLCD::write(uint8_t value) {
  send(value, HIGH);
  return 1; // assume sucess
}

/************ low level data pushing commands **********/

// write either command or data, with automatic 4/8-bit selection
void oneWireLCD::send(uint8_t value, uint8_t mode) {
  MODdigitalWrite(_rs_pin, mode);
  
  write4bits(value>>4);
  write4bits(value);
}

void oneWireLCD::pulseEnable(void) {
  MODdigitalWrite(_enable_pin, LOW);
  delayMicroseconds(1);    
  MODdigitalWrite(_enable_pin, HIGH);
  delayMicroseconds(1);    // enable pulse must be >450ns
  MODdigitalWrite(_enable_pin, LOW);
  delayMicroseconds(100);   // commands need > 37us to settle
}

void oneWireLCD::write4bits(uint8_t value) {
  for (int i = 0; i < 4; i++) {
    MODdigitalWrite(_data_pins[i], (value >> i) & 0x01);
  }
  pulseEnable();
}

void oneWireLCD::MODdigitalWrite(uint8_t pin, uint8_t state){
  if(state){
    Buffer |= (1 << pin);
  }else{
    Buffer &= ~(1 << pin);
  }
  
  if(singleWire > 0){
    Shift.Out(Buffer);
  }else{
    digitalWrite(_latchPin, LOW);
    shiftOut(_dataPin, _clockPin, MSBFIRST, Buffer);   
    digitalWrite(_latchPin, HIGH);
  }
}

