/*
  OneShift.h - Library for 1-wire shift registers interface.
  Created by Hrastovc, June 16, 2016.
  Source: http://www.romanblack.com/shift1.htm
  Released into the public domain.
*/

#ifndef OneShift_h
#define OneShift_h

#include "Arduino.h"

class OneShift
{
  public:
    OneShift();
    void Out(byte Byte);
    void init(byte pin);
  private:
    void Bit(boolean Bit);
    void LastBit();
    uint8_t bit;
    volatile uint8_t *out;
};

#endif
