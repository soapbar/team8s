// line following robot: follows white line (against dark background)
// does a figure 8

#include <Servo.h>

// wheels
Servo LeftServo;
Servo RightServo;

// wheels - output pins
int OUTLEFT = 5;
int OUTRIGHT = 6;

// line following sensors - analog input pins
// L/R on either side of line
int SENSELEFT = A2;
int SENSERIGHT = A1;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.begin(9600);
}

void loop() {
  LeftServo.write(90);
  RightServo.write(180);
  //if L/R sensors sense white, correct until see black
  /*if (analogRead(SENSERIGHT) < 860) driftRight(); 
  else if (analogRead(SENSELEFT) < 860) driftLeft(); 
  else goStraight();  */
}

void goStraight(){
  LeftServo.write(93);
  RightServo.write(87);
}

void driftLeft() {
  Serial.println("in drift right");

  LeftServo.write(90);
  RightServo.write(50);
}

void driftRight() {
  Serial.println("in drift right");
  LeftServo.write(130);
  RightServo.write(90);
}
