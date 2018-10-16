#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 256 // set to 256 point fft
#include <FFT.h> // include the library

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
  // IR Setup
  Serial.begin(9600); // use the serial port

  // Servo Setup
  pinMode(LED_BUILTIN, OUTPUT);
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.println("hi");
}

void loop() {
  while (1) { // reduces jitter
    Serial.println(analogRead(SENSERIGHT));
    Serial.println("LEFT");
    Serial.println(analogRead(SENSELEFT));

    followLine();
  }
}

void followLine() {
  bool rightIsWhite = analogRead(SENSERIGHT) < 860;
  bool leftIsWhite = analogRead(SENSELEFT) < 860;
  bool reachedIntersection = rightIsWhite && leftIsWhite;
  if (reachedIntersection) { // intersection
    checkIR();
    fullStop();
  }
  else if (rightIsWhite) {
    turnRight();
  }
  else if (leftIsWhite) {
    turnLeft();
  }
  else {
    goStraight();
  }
}

void checkIR() {
  cli();  // UDRE interrupt slows this way down on arduino1.0

  byte prevTIMSK0 = TIMSK0;
  byte prevADCSRA = ADCSRA;
  byte prevADMUX = ADMUX;
  byte prevDIDR0 = DIDR0;

    TIMSK0 = 0; // turn off timer0 for lower jitter
    ADCSRA = 0xe5; // set the adc to free running mode
    ADMUX = 0x40; // use adc0
    DIDR0 = 0x01; // turn off the digital input for adc0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    while (!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i + 1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  if (fft_log_out[42] > 160) {
    fullStop();
  }

  TIMSK0 = prevTIMSK0;
  ADCSRA = prevADCSRA;
  ADMUX = prevADMUX;
  DIDR0 = prevDIDR0;
}


// directions
void turnLeft() {
  LeftServo.write(90);
  RightServo.write(50);
}
void turnRight() {
  LeftServo.write(130);
  RightServo.write(90);
}
void sharpLeft() {
  LeftServo.write(85);
  RightServo.write(20);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(300);
  digitalWrite(LED_BUILTIN, LOW);
}
void sharpRight() {
  LeftServo.write(160);
  RightServo.write(95);
  delay(300);
}
void goStraight() {
  LeftServo.write(93);
  RightServo.write(87);
}
void fullStop() {
  LeftServo.write(90);
  RightServo.write(90);
  while (1);
}
