#include <oneWireLCD.h>

#define singleWire

#if defined(singleWire)
  oneWireLCD lcd(6);        //single wire operation
#else
  oneWireLCD lcd(2, 3, 4);  //lcd(clock, data, latch)
#endif                      //lcd(SH_CP, DS,   ST_CP)


void setup() {
  // set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  // Print a message to the LCD.
  lcd.print("hello, world!");
}

void loop() {
  // set the cursor to column 0, line 1
  // (note: line 1 is the second row, since counting begins with 0):
  lcd.setCursor(0, 1);
  // print the number of seconds since reset:
  lcd.print(millis() / 1000);
}
