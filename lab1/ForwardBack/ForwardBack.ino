#include <Servo.h>

Servo LeftServo;
Servo RightServo;


int OUTLEFT = 5;
int OUTRIGHT = 6;

void setup() {
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.begin(9600);
  pinMode(4, OUTPUT);
  digitalWrite(4, HIGH);
}

void loop() {
  LeftServo.write(0);
  RightServo.write(0);
  delay(5000);
  LeftServo.write(90);
  RightServo.write(90);
  delay(2000);
  LeftServo.write(180);
  RightServo.write(180);
  delay(5000); 
}
