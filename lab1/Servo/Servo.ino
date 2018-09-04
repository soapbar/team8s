#include <Servo.h>

Servo ServoName;

int PINNAME = A0;
int OUTPIN = 3;
void setup() {
  ServoName.attach(OUTPIN);
  Serial.begin(9600);
}

void loop() {
  // we normalized the values we were reading to get numbers between 0 and 180
  int val = (analogRead(PINNAME)-315)/4;
  ServoName.write(val);
  Serial.println(val);
  delay(500);
}
