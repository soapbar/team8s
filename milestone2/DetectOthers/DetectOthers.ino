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
int SENSELEFT = A1;
int SENSERIGHT = A2;

void setup() {
  // IR Setup
  Serial.begin(9600); // use the serial port
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0

  // Servo Setup
  pinMode(LED_BUILTIN, OUTPUT);
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
}

void loop() {
  while(1) { // reduces jitter
    // try to detect other robots
//    checkIR();
//    if (analogRead(SENSERIGHT) < 860 && analogRead(SENSELEFT) < 860) { // intersection
//      fullStop();
//    } 
//    else if (analogRead(SENSERIGHT) < 860) {
//      turnRight();
//    }
//    //else if (analogRead(SENSELEFT) < 860) {
//      //turnLeft();
//    //}
    //else { // go straight
      LeftServo.write(93);
      RightServo.write(87);
    //}
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
