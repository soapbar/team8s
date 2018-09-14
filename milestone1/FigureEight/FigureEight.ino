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
int SENSELEFT = A0;
int SENSERIGHT = A1;

// counter
int c = 0;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.begin(9600);
}

void loop() {
  //if L/R sensors sense white, correct until see black
  if (analogRead(SENSERIGHT) < 860 && analogRead(SENSELEFT) < 860) { // intersection
    figure8();
  } 
  else if (analogRead(SENSERIGHT) < 860) {
    turnRight();
  }
  else if (analogRead(SENSELEFT) < 860) {
    turnLeft();
  }
  else { // go straight
    LeftServo.write(93);
    RightServo.write(87);
  }
}

void figure8(){
  if (c%8<4) {
    sharpRight();
  } else {
    sharpLeft();
  }
  c++;
}

void turnLeft() {
  LeftServo.write(90);
  RightServo.write(50);
}

void turnRight() {
  LeftServo.write(130);
  RightServo.write(90);
}

void fullStop(){
  LeftServo.write(90);
  RightServo.write(90);
  while(1);
}

void sharpLeft(){
  LeftServo.write(85);
  RightServo.write(20);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);
  digitalWrite(LED_BUILTIN, LOW);

}

void sharpRight(){
  LeftServo.write(160);
  RightServo.write(95);
  delay(300);
}
