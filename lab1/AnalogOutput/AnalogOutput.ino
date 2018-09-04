int PINNAME = A0;
int OUTPIN = 3;

void setup() {
  pinMode(OUTPIN, OUTPUT);
}

void loop() {
  analogWrite(OUTPIN, analogRead(PINNAME));
}
