#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 128 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//-----------------------------------------------------------
// Adjustable parameters

int startLocation = 0;
int dir   = 1; //0-North, 1-East, 2-South, 3-West
const int turnDelay = 300;
const int turnAroundDelay = 750;

//-----------------------------------------------------------

const int dim = 10; // dimensions of the maze
const int mazeSize = 100;

RF24 radio(9,10);
const uint64_t pipes[2] = { 0x0000000016LL, 0x00000000017LL };
unsigned long got_time;

// wheels
Servo LeftServo;
Servo RightServo;

// MUX
// 000: microphone 
// 001: IR sensor
int MS0 = 2;
int MS1 = 3;
int MS2 = 4;

// wheels - output pins
int OUTLEFT = 5;
int OUTRIGHT = 6;

// line following sensors - analog input pins
// L/R on either side of line
int SENSERIGHT = A1;
int SENSELEFT = A2;
int WALLRIGHT = A3;
int WALLFRONT = A4;
int WALLLEFT = A5;
int MUXSENSORS = A0;

int c = 0;

/* The direction that the robot is currently facing:
    - 0-North
    - 1-East
    - 2-South
    - 3-West
   The coordinates of the robot are encoded as a single number
    - x=location%dim
    - y=location/dim
*/

byte location;
byte newLocation;

byte settled [mazeSize];
byte frontier [mazeSize];
byte walls [mazeSize]; 
byte path [mazeSize];

int last_s = 0;
int last_f = -1;
byte last_p = 0;

void setup() {
  pinMode(8, OUTPUT);
  digitalWrite(8, LOW);
  // IR Setup
  Serial.begin(9600); // use the serial port
  Serial.println("Please work");
  Serial.flush();
  pinMode(MS0, OUTPUT);
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);
  radio.setAutoAck(true);
  // set the channel
  radio.setChannel(0x50);
  // set the power
  // RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm, RF24_PA_MED=-6dBM, and RF24_PA_HIGH=0dBm.
  radio.setPALevel(RF24_PA_HIGH);
  //RF24_250KBPS for 250kbs, RF24_1MBPS for 1Mbps, or RF24_2MBPS for 2Mbps
  radio.setDataRate(RF24_250KBPS);

  // optionally, reduce the payload size, seems to improve reliability
  radio.setPayloadSize(2);
  
  // Start listening
  radio.startListening();

  // Dump the configuration of the rf unit for debugging
  radio.printDetails();
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  Serial.println("WAIT FOR START TONE");
  waitForSignal();
  Serial.println("Starting!");

  // Servo Setup
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  dir = 1;
  
  location = startLocation;
  path[last_p] = startLocation;
  
  for(int i=0;i<dim*dim;i++){
      settled[i] = 100;
      frontier[i] = 100;
      walls[i] = 100;
  }

  
  
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
  int msg = 0;
//  checkIR(msg);
  if (reachedIntersection) { // intersection
    //Serial.println("I'M AT AN INTERSECTION");
    LeftServo.write(90);
    RightServo.write(90);
    msg = checkIR(msg);
    delay(100);
    msg = checkWall(msg);
    LeftServo.write(90);
    RightServo.write(90);
    //Serial.println("Location");
    //Serial.println(location);
    msg = updateCoord(msg);
    //Serial.println("New Location");
    //Serial.println(newLocation);
    location = newLocation;
    sendMSG(msg);
    //Serial.println("Message Sent!");   
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

int checkIR(int msg) {
  int robot = 0;
  LeftServo.write(90);
  RightServo.write(90);
  LeftServo.detach();
  RightServo.detach();
  cli();  // UDRE interrupt slows this way down on arduino1.0

  byte prevTIMSK0 = TIMSK0;
  byte prevADCSRA = ADCSRA;
  byte prevADMUX = ADMUX;
  byte prevDIDR0 = DIDR0;

  //001 = IR
  digitalWrite(MS0, HIGH);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  bool seeRobot = true;
  while(seeRobot){
    for (int i = 0 ; i < 256 ; i += 2) { // save 256 samples
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
    if (fft_log_out[21] > 100) {
      digitalWrite(8, LOW);
      delay(1000);
      robot = 1;
    }
    else
      digitalWrite(8, HIGH);
      seeRobot = false;
  }
  TIMSK0 = prevTIMSK0;
  ADCSRA = prevADCSRA;
  ADMUX = prevADMUX;
  DIDR0 = prevDIDR0;
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  return msg | (robot << 14);
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

void goStraight() {
  LeftServo.write(94);
  RightServo.write(86);
}

int checkWall(int msg){
  byte north = 0;
  byte east = 0;
  byte south = 0;
  byte west = 0;
  LeftServo.write(90);
  RightServo.write(90);
  boolean right = (analogRead(WALLRIGHT)+analogRead(WALLRIGHT)/2) > 200;
  boolean front = (analogRead(WALLFRONT)+analogRead(WALLFRONT)/2) > 200;
  boolean left = (analogRead(WALLLEFT)+analogRead(WALLLEFT)/2) > 200;
  delay(500);
  if (right) {
    //Serial.println("Wall Right");
    switch(dir) {
      case 0: east = 1;
              break;
      case 1: south = 1;
              break;
      case 2: west = 1;
              break;
      case 3: north = 1;
              break;
    }
  }
  if (front) {
    //Serial.println("Wall Front");
    switch(dir) {
      case 0: north = 1;
              break;
      case 1: east = 1;
              break;
      case 2: south = 1;
              break;
      case 3: west = 1;
              break;
    }
  }
  if (left) {
    //Serial.println("Wall Left");
    switch(dir) {
      case 0: west = 1;
              break;
      case 1: north = 1;
              break;
      case 2: east = 1;
              break;
      case 3: south = 1;
              break;
    }
  }

  //Update wall info for djikstra
  walls[location] = (north << 0) | (east << 2) | (south << 1) | (west << 3);
     
  return msg | (north << 7) | (east << 6) | (south << 5) | (west << 4);
}

void sharpLeft() {
  LeftServo.write(85);
  RightServo.write(20);
  
  // update direction
  dir = dir-1;
  if(dir < 0){
    dir = dir+4;   
  }
  dir = dir%4;
  
  delay(turnDelay);

}
void sharpRight() {
  LeftServo.write(160);
  RightServo.write(95);
  dir = (dir+1)%4; // update direction
  delay(turnDelay);
}

void oneEighty() {
  LeftServo.write(160);
  RightServo.write(95);
  dir = (dir+2)%4; //update direction
  delay(turnAroundDelay);
}

void fullStop() {
  LeftServo.write(90);
  RightServo.write(90);
  while (1);
}

//Finds and Goes to new coordinate
int updateCoord(int msg) {
  newLocation = djikstra();
  //Serial.println("Currently at");
  //Serial.println(location);
  //Serial.println("Go to new location");
  //Serial.println(newLocation);
  int goTo = newLocation-location;
  if(goTo == 1){
    faceEast();
    //Serial.println("go east");
  }
  else if(goTo == -1){
    faceWest();
    //Serial.println("go west");
  }
  else if(goTo == dim){
    faceSouth();
    //Serial.println("go south");
  }
  else if(goTo == -dim){
    faceNorth();
    //Serial.println("go north");
  }
  else{
    //Serial.println("Done searching maze");
    msg = (location << 8) | msg;
    sendMSG(msg);   
    fullStop();
  }
  goStraight();
  delay(300);  
  return (location << 8) | msg;
}


void faceNorth(){
  switch(dir) {
    case 1: sharpLeft();
            break;
    case 2: oneEighty();
            break;
    case 3: sharpRight();
            break;
    default: break;   
  }
}

void faceEast(){
    switch(dir) {
      case 0: sharpRight();
            break;
      case 2: sharpLeft();
            break;
      case 3: oneEighty();
            break;
      default: break;   
  }
}

void faceSouth(){
    switch(dir) {
      case 0: oneEighty();
            break;
      case 1: sharpRight();
            break;
      case 3: sharpLeft();
            break;
      default: break;   
  }
}

void faceWest(){
  switch(dir) {
      case 0: sharpLeft();
            break;
      case 1: oneEighty();
            break;
      case 2: sharpRight();
            break;
      default: break;   
  }
}

int djikstra(){

    byte wall_data = walls[location];
    settled[last_s] = location;
    last_s++;
    
    if(wall_data % 2 != 1) {
        last_f++;
        frontier[last_f] = location-dim;
    }
    if((wall_data >> 1) %2 != 1) {
        last_f++;
        frontier[last_f] = location+dim;        
    }
    if((wall_data >> 2) %2 != 1) {
        last_f++;
        frontier[last_f] = location+1;
        
    }
    if((wall_data >> 3) %2 != 1) {
        last_f++;
        frontier[last_f] = location-1;
        
    }
    for(int i = 0; i<=last_s; i++ ){
        byte point = settled[i];
        int c = 0;
        while (c <= last_f){
            
            if (point == frontier[c] || frontier[c] < 0 || frontier[c] >= dim*dim){
                if (c == 0){
                    frontier[0] = frontier[last_f];
                    last_f--;
                }
                else if (c == last_f){
                    frontier[last_f] = -1;
                    last_f--;
                }
                else{
                    frontier[c] = frontier[last_f];
                    last_f--;
                }
            }
            else
                c++;
                        
        }   
    }
    
    if (last_f == -1){
        return location;
    }
    else{
        int min_dist = mazeSize+1;
        byte dest = 100;
        int dest_ind = -1;
//        //Serial.println("Location");
//        //Serial.println(location);
//        //Serial.println("dir");
//        //Serial.println(dir);
//        Serial.println("Frontier:");
        for(int i=0;i<=last_f;i++){
//          Serial.println(frontier[i]);
          int xdist = abs(frontier[i]%dim-location%dim);
          int y1 = frontier[i]/dim;
          int y2 = location/dim;
          int ydist = abs(y1-y2);
          int f_dir = (int)frontier[i]-(int)location;
          
          //0-North, 1-East, 2-South, 3-West
          switch(f_dir) {
              case -1: f_dir = 3;
                    break;
              case dim: f_dir = 2;
                    break;
              case -1*dim: f_dir = 0;
                    break;
              default: break;   
          }
          if(f_dir == dir){
            dest = frontier[i];
            break;
          }
//          int bonus = abs(dir - f_dir);
          int dist = xdist+ydist;
//          Serial.println("my dist");
//          Serial.println(dist);
          if(dist < min_dist){
            min_dist = dist;
            dest = frontier[i];
            dest_ind = i;
          }
        }
        //Serial.println("dijkstra dest:");
        //Serial.println(dest);

        byte next_loc = greedyPath(dest,location);
        if(next_loc == dest){
            if( dest_ind == 0){
                frontier[0] = frontier[last_f];
                last_f--;
            }
            else if( dest_ind == last_f){
                frontier[last_f] = -1;
                last_f--;
            }
            else{
                frontier[dest_ind] = frontier[last_f];
                last_f--;
            }
        }
        return next_loc;
    }      
}


byte greedyPath(byte dest,byte start){
      //Serial.println("Finding path!");
      byte next = location;
      byte wall_data = walls[start];
      int last_fp = -1;
      byte frontier_path[4];
          
      for(int i=0;i<4;i++){
        frontier_path[i] = 100;
      }
          
      if(wall_data % 2 != 1){
          last_fp++;
          frontier_path[last_fp] = start-dim;
      }
      if((wall_data >> 1) %2 != 1){
          last_fp++;
          frontier_path[last_fp] = start+dim;
      }
      if((wall_data >> 2) %2 != 1){
          last_fp++;
          frontier_path[last_fp] = start+1;
      }
      if((wall_data >> 3) %2 != 1){
          last_fp++;
          frontier_path[last_fp] = start-1;
      }

      c = 0;
      for(int i = 0; i <= last_p; i++ ){
        byte point = path[i];
        while (c <= last_fp){
            if (point == frontier_path[c] || frontier_path[c] >= mazeSize){
                if (c == 0){ 
                    frontier_path[0] = frontier_path[last_fp];
                    last_fp--;
                }
                else if(c == last_fp){
                    frontier_path[last_fp] = -1;
                    last_fp--;
                }
                else{
                    frontier_path[c] = frontier_path[last_fp];
                    last_fp--;
                }
            }
            else
                c++;
          }
      }
          
      int i = 0;
      int found = 0;
      //Serial.println("Possible locations");
      while(i <= last_fp && found == 0){
          //Serial.println(frontier_path[i]);
          if( dest == frontier_path[i]){
              found = 1;
              next = dest;
              last_p++;
              path[last_p] = dest;
          }
          else
            i++;
      }
          
      if(found == 0){
          last_p--;
          next = path[last_p];
      }

      return next;
             
}


void sendMSG(int msg) {
  //Serial.println("Pause to send MSG");
  LeftServo.write(90);
  RightServo.write(90);
  
  // First, stop listening so we can talk.
  radio.stopListening();
  
  // Take the time, and send it.  This will block until complete
  unsigned long time = millis();
  printf("Now sending %lu...",msg);
  bool ok = radio.write( &msg, sizeof(int) );

  if (ok)
    printf("ok...");
  else
    printf("failed.\n\r");

  // Now, continue listening
  radio.startListening();

  // Wait here until we get a response, or timeout (250ms)
  unsigned long started_waiting_at = millis();
  bool timeout = false;
  while ( ! radio.available() && ! timeout )
    if (millis() - started_waiting_at > 200 )
      timeout = true;

  // Describe the results
  if ( timeout )
  {
    while (timeout) {
      radio.stopListening();
      printf("Now sending %lu...",msg);
      bool ok = radio.write( &msg, sizeof(int) );
  
      if (ok)
        printf("ok...");
      else
        printf("failed.\n\r");
  
      // Now, continue listening
      radio.startListening();
  
      // Wait here until we get a response, or timeout (250ms)
      started_waiting_at = millis();
      timeout = false;
      while ( ! radio.available() && ! timeout )
        if (millis() - started_waiting_at > 200 )
          timeout = true;
    }
  }
  else
  {
    // Grab the response, compare, and send to debugging spew
    unsigned long got_time;
    radio.read( &got_time, sizeof(unsigned long) );

    // Spew it
    printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
  }
}

void waitForSignal(){
  byte prevTIMSK0 = TIMSK0;
  byte prevADCSRA = ADCSRA;
  byte prevADMUX = ADMUX;
  byte prevDIDR0 = DIDR0;
  // 000 = microphone
  digitalWrite(MS0, LOW);
  digitalWrite(MS1, LOW);
  digitalWrite(MS2, LOW);
  int startButton = 1;
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  int start = 0;
  while(start == 0){
    for (int i = 0 ; i < 256 ; i += 2) { // save 256 samples
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
    startButton = digitalRead(0);
    if ((fft_log_out[3] > 150) || startButton == 0) 
      start = 1;
  }
  TIMSK0 = prevTIMSK0;
  ADCSRA = prevADCSRA;
  ADMUX = prevADMUX;
  DIDR0 = prevDIDR0;

}
