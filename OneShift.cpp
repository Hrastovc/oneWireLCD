/*
  OneShift.h - Library for 1-wire shift registers interface.
  Created by Hrastovc, June 16, 2016.
  Source: http://www.romanblack.com/shift1.htm
  Released into the public domain.
*/

#include "Arduino.h"
#include "OneShift.h"

OneShift::OneShift(){
  //init(3);
}

void OneShift::init(byte _pin){
  pinMode(_pin, OUTPUT);
  digitalWrite(_pin, HIGH);
  bit = digitalPinToBitMask(_pin);
  uint8_t port = digitalPinToPort(_pin);
  out = portOutputRegister(port);
}

void OneShift::Out(byte Byte){
  byte BitOfByte;
  for(int i = 128; i > 1; i = i / 2){
    BitOfByte = Byte & i;
    if(BitOfByte > 0){
      Bit(1);
    }else{
      Bit(0);
    }
  }
  LastBit();
}

void OneShift::Bit(boolean Bit){
  if(Bit == 1){
    *out &= ~bit;
    *out |= bit;
    delayMicroseconds(15);
  }else{
    *out &= ~bit;
    delayMicroseconds(15);
    *out |= bit;
    delayMicroseconds(30);
  }
}

void OneShift::LastBit(){
  *out &= ~bit;
  delayMicroseconds(200);
  *out |= bit;
  delayMicroseconds(300);
}
