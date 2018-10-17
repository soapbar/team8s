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
int WALLRIGHT = A3;
int WALLFRONT = A4;
int WALLLEFT = A5;

int c = 0;

int rightLED = 3;
int frontLED = 4;
int stopLED = 2;

void setup() {
  // IR Setup
  Serial.begin(9600); // use the serial port

  // Servo Setup
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(rightLED, OUTPUT);
  pinMode(frontLED, OUTPUT);
  pinMode(stopLED, OUTPUT);
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  Serial.println("hi");
}

void loop() {
  while (1) { // reduces jitter
    followLine();
  }
}

void followLine() {
  bool rightIsWhite = analogRead(SENSERIGHT) < 860;
  bool leftIsWhite = analogRead(SENSELEFT) < 860;
  bool reachedIntersection = rightIsWhite && leftIsWhite;
  if (reachedIntersection) { // intersection
    LeftServo.write(90);
    RightServo.write(90);
    checkIR();
    delay(100);
    //figure8();
    checkWall();
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
  LeftServo.detach();
  RightServo.detach();
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
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
}


void figure8(){
  if (c%8<4) {
    sharpRight();
  } else {
    sharpLeft();
  }
  c++;
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
void checkWall(){
  int right = (analogRead(WALLRIGHT)+analogRead(WALLRIGHT)+analogRead(WALLRIGHT))/3;
  int front = (analogRead(WALLFRONT)+analogRead(WALLFRONT)+analogRead(WALLFRONT))/3;
  if(right < 100) {
    digitalWrite(rightLED, HIGH);
    sharpRight();
    digitalWrite(rightLED, LOW);
  }
  else {
    if (front < 100) {
      digitalWrite(frontLED, HIGH);
      goStraight();
      delay(300);
      digitalWrite(frontLED, LOW);
    }
    else {
      digitalWrite(stopLED, HIGH);
      oneEighty();
      digitalWrite(stopLED, LOW);
    }
  }
}

void oneEighty() {
  LeftServo.write(100);
  RightServo.write(100);
  delay(660);
}
