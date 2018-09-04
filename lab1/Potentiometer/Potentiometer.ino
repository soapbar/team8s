int PINNAME = A0;
void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.println(analogRead(PINNAME));
  delay(500);                       
}
