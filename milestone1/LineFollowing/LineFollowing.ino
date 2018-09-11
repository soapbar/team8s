// line following robot: follows white line (against dark background)

#include <Servo.h>

// wheels
Servo LeftServo;
Servo RightServo;

// wheels - output pins
int OUTLEFT = 5;
int OUTRIGHT = 6;

// line following sensors - analog input pins
// L/R on either side of line
int SENSERIGHT = A0;
int SENSELEFT = A1;


void setup() {
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.begin(9600);
}

void loop() {
  //if L/R sensors sense white, correct until see black
  if (analogRead(SENSERIGHT) < 900) {
    turnRight();
  }
  else if (analogRead(SENSELEFT) < 900) {
    turnLeft();
  }
  else { // go straight
    LeftServo.write(93);
    RightServo.write(87);
  }
}

void turnLeft() {
  LeftServo.write(90);
  RightServo.write(87);
}

void turnRight() {
  LeftServo.write(93);
  RightServo.write(90);
}
