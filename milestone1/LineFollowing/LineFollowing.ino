#include <Servo.h>

Servo LeftServo;
Servo RightServo;

int OUTLEFT = 5;
int OUTRIGHT = 6;

int SENSERIGHT = A0;
int SENSELEFT = A1;

void setup() {
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.begin(9600);
}

void loop() {
  if (analogRead(SENSERIGHT) > 900) {
    turnLeft();
  }
  else if (analogRead(SENSELEFT) > 900) {
    turnRight();
  }
  else {
    LeftServo.write(93);
    RightServo.write(87);
  }
}

void turnRight() {
  LeftServo.write(90);
  RightServo.write(87);
}

void turnLeft() {
  LeftServo.write(93);
  RightServo.write(90);
}
