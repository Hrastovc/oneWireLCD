/*
 * single wire:
 * oneWireLCD lcd(pin);
 * oneWireLCD lcd(pin, rs_pin, enable_pin, D4, D5, D6, D7, backlight_pin);
 * 
 * 74HC595:
 * oneWireLCD lcd(clockPin, dataPin, latchPin);
 * oneWireLCD lcd(clockPin, dataPin, latchPin, rs_pin, enable_pin, D4, D5, D6, D7, backlight_pin);
 */
#include "oneWireLCD.h"

//#define singleWire

#if defined(singleWire)
  oneWireLCD lcd(6);        //single wire operation
#else
  oneWireLCD lcd(2, 3, 4);  //lcd(clock, data, latch)
#endif                      //lcd(SH_CP, DS,   ST_CP)


#define resetButton 9

unsigned long Time = 0;
unsigned long Lap = 0;

int timeArray[] = {0, 0, 0, 0};
int lapArray[] = {0, 0, 0, 0};

char timeBuffer[] = {0, 0, 0, 0, 0, 0, 0, 0};
char lapBuffer[] = {0, 0, 0, 0, 0, 0, 0, 0};


void setup() {
  // initialize timer1 
  noInterrupts();
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1  = 0;
  
  OCR1A = 20000;            // compare match register ((16MHz/8/)100Hz)
  TCCR1B |= (1 << WGM12);
  TCCR1B |= (1 << CS11);    // 8 prescaler 
  TIMSK1 |= (1 << OCIE1A);  // compare interrupt
  interrupts();

  //init pins
  pinMode(resetButton, INPUT);
  
  //init display
  lcd.begin(16, 2);
  lcd.print("TIME");
  lcd.setCursor(7, 0);
  lcd.print(":");
  lcd.setCursor(10, 0);
  lcd.print(":");
  lcd.setCursor(13, 0);
  lcd.print(":");
  lcd.setCursor(1, 1);
  lcd.print("LAP");
  lcd.setCursor(7, 1);
  lcd.print(":");
  lcd.setCursor(10, 1);
  lcd.print(":");
  lcd.setCursor(13, 1);
  lcd.print(":");
}

void loop() {
  updateValues("TIME");
  updateValues("LAP");
  
  updateBuffer("TIME");
  updateBuffer("LAP");
  
  updateDisplay("TIME");
  updateDisplay("LAP");
  
  LAP();
  
  delay(10);
}

void updateValues(char row[10]){
  if(row == "TIME"){
    timeArray[0] = Time % 100;
    timeArray[1] = (Time/100) % 60;
    timeArray[2] = ((Time/100)/60) % 60;
    timeArray[3] = (((Time/100)/60)/60) % 60;
  }

  if(row == "LAP"){
    lapArray[0] = (Time-Lap) % 100;
    lapArray[1] = ((Time-Lap)/100) % 60;
    lapArray[2] = (((Time-Lap)/100)/60) % 60;
    lapArray[3] = ((((Time-Lap)/100)/60)/60) % 60;
  }
}

void updateBuffer(char row[10]){
  char tmp[2];
  
  if(row == "TIME"){
    for(int i = 0; i < 4; i++){
      sprintf(tmp, "%02i", timeArray[3 - i]);
      timeBuffer[i*2] = tmp[0];
      timeBuffer[(i*2)+1] = tmp[1];
    }
  }

  if(row == "LAP"){
    for(int i = 0; i < 4; i++){
      sprintf(tmp, "%02i", lapArray[3 - i]);
      lapBuffer[i*2] = tmp[0];
      lapBuffer[(i*2)+1] = tmp[1];
    }
  }
}

void updateDisplay(char row[10]){
  if(row == "TIME"){
    for(byte i = 0; i < 8; i++){
      lcd.setCursor(i + (i/2) + 5, 0);
      lcd.print(timeBuffer[i]);
    }
  }

  if(row == "LAP"){
    for(byte i = 0; i < 8; i++){
      lcd.setCursor(i + (i/2) + 5, 1);
      lcd.print(lapBuffer[i]);
    }
  }
}

void LAP(){
  if(digitalRead(resetButton)){
    Lap = Time;
  }
}

ISR(TIMER1_COMPA_vect){
  Time++;
}

