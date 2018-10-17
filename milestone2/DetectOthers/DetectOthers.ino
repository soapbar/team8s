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
int WALLLEFT = A3;

// counter
int c = 0;

void setup() {
  // IR Setup
  Serial.begin(9600); // use the serial port
  
  // Servo Setup
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);

  LeftServo.write(90);
  RightServo.write(90);
}

void loop() {
  while(1) { // reduces jitter
    // try to detect other robots
    checkIR();
    if (analogRead(SENSERIGHT) < 860 && analogRead(SENSELEFT) < 860) { // intersection
      figure8();  
    } 
     if (analogRead(SENSERIGHT) < 860) {
      driftRight();
    }
    else if (analogRead(SENSELEFT) < 860) {
      driftLeft();
    }
    else { // go straight
      goStraight();
    }
  }
}

void checkIR() {
  cli();  // UDRE interrupt slows this way down on arduino1.0
  for (int i = 0 ; i < 512 ; i += 2) { // save 256 samples
    while(!(ADCSRA & 0x10)); // wait for adc to be ready
    ADCSRA = 0xf5; // restart adc
    byte m = ADCL; // fetch adc data
    byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fft_input[i] = k; // put real data into even bins
    fft_input[i+1] = 0; // set odd bins to 0
  }
  fft_window(); // window the data for better frequency response
  fft_reorder(); // reorder the data before doing the fft
  fft_run(); // process the data in the fft
  fft_mag_log(); // take the output of the fft
  sei();
  if (fft_log_out[42] > 160) {
    fullStop();
  }
}

void checkWall(){
  if(analogRead(WALLLEFT) < 100)
    driftLeft();
  else if(analogRead(WALLFRONT) > 100){
    if(analogRead(WALLLEFT) > 100)
      turnAround();
    else
      driftLeft();
  }
  else
    goStraight();
}

void figure8(){
  if (c%8<4) {
    sharpRight();
  } else {
    sharpLeft();
  }
  c++;
}

void goStraight(){
  LeftServo.write(93);
  RightServo.write(87);
}

void driftLeft() {
  LeftServo.write(90);
  RightServo.write(50);
}

void driftRight() {
  LeftServo.write(130);
  RightServo.write(90);
}

void sharpLeft(){
  LeftServo.write(85);
  RightServo.write(20);
  delay(300);
}

void sharpRight(){
  LeftServo.write(160);
  RightServo.write(95);
  delay(300);
}

void turnAround(){
  sharpRight();
  sharpRight();
}

void fullStop(){
  LeftServo.write(90);
  RightServo.write(90);
  while(1);
}

