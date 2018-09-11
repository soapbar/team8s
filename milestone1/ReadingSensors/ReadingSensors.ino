// program to print values of line following sensors

int RightSensor = A0;
int LeftSensor = A1;


void setup() {
  // put your setup code here, to run once:
    Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
  Serial.println(analogRead(RightSensor));
  Serial.println(analogRead(LeftSensor));
  Serial.println("\n");
  delay(500);                       // wait for half a second
}
