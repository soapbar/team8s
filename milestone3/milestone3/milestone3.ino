#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 256 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

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

int start = 0;
int c = 0;

void waitForSignal();
void sendMSG(int msg);
/* The direction that the robot is currently facing:
    - 0-North
    - 1-East
    - 2-South
    - 3-West
   The coordinates of the robot are encoded as a single number
    - x=location%dim
    - y=location/dim
*/
int dir;
int dim = 3; // dimentions of the maze
int location;

int settled [9];
int frontier [9];
int walls [9]; //TEMP FIX FIX THIS FIX THIS

int last_s = 0;
int last_f = -1;

void setup() {
  // IR Setup
  Serial.begin(9600); // use the serial port
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
  radio.setPALevel(RF24_PA_MIN);
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

  // Servo Setup
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  dir = 1;
  location = 0;
  for(int i=0;i<dim*dim;i++){
      settled[i] = -1;
      frontier[i] = -1;
      walls[i] = -1;
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
  if (reachedIntersection) { // intersection
    Serial.println("I'M AT AN INTERSECTION");
    int msg = 0;
    LeftServo.write(90);
    RightServo.write(90);
    msg = checkIR(msg);
    delay(100);
    msg = checkWall(msg);
    LeftServo.write(90);
    RightServo.write(90);
    msg = updateCoord(msg);
    sendMSG(msg);   
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
    robot = 1;
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
  LeftServo.write(93);
  RightServo.write(87);
}

int checkWall(int msg){
  int north = 0;
  int east = 0;
  int south = 0;
  int west = 0;
  boolean right = (analogRead(WALLRIGHT)+analogRead(WALLRIGHT)/2) > 200;
  boolean front = (analogRead(WALLFRONT)+analogRead(WALLFRONT)/2) > 200;
  boolean left = (analogRead(WALLLEFT)+analogRead(WALLLEFT)/2) > 200;
  if (right) {
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
  dir = (dir-1)%4; // update direction
  delay(300);

}
void sharpRight() {
  LeftServo.write(160);
  RightServo.write(95);
  dir = (dir+1)%4; // update direction
  delay(300);
}

void oneEighty() {
  LeftServo.write(100);
  RightServo.write(100);
  dir = (dir+2)%4; //update direction
  delay(660);
}

void fullStop() {
  LeftServo.write(90);
  RightServo.write(90);
  while (1);
}

int updateCoord(int msg) {
  Serial.println("UPDATE COORD");
  switch(dir) {
    case 0: location = location - 1;
            break;
    case 1: location = location + dim;
            break;
    case 2: location = location + 1;
            break;
    case 3: location = location - dim;
            break;
    default: break;   
  }
  int newCoord = djikstra();
  int goTo = newCoord-location;
  if(goTo == 1){
    faceEast();
  }
  else if(goTo == -1){
    faceWest();
  }
  else if(goTo == dim){
    faceNorth();
  }
  else if(goTo == -dim){
    faceSouth();
  }
  goStraight();

  location = newCoord;
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

    int wall_data = walls[start];
    settled[last_s] = location;
    last_s++;
    
    if(wall_data % 2 == 1) {
        frontier[last_f] = location-dim;
        last_f++;
    }
    if((wall_data >> 1) %2 == 1) {
        frontier[last_f] = location+dim;
        last_f++;
    }
    if((wall_data >> 2) %2 == 1) {
        frontier[last_f] = location+1;
        last_f++;
    }
    if((wall_data >> 3) %2 == 1) {
        frontier[last_f] = location-1;
        last_f++;
    }

    for(int i = 0; i<=last_s; i++ ){
        int point = settled[i];
        c = 1;
        while (c <= last_f)
            if (point == frontier[c]){
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
            else {
                c++; 
            }   
    }

    if (last_f == -1)
        return location;
    else{
        int min_dist = dim*dim+1;
        int dest = -1;
        int dest_ind = -1;
        for(int i=0;i<=last_f;i++){
          int dist = frontier[i]-location;
          if(dist < min_dist){
            min_dist = dist;
            dest = frontier[i];
            dest_ind = -1;
          }
        }
        int path[9][2];
        for(int i=0;i<dim*dim;i++)
          path[i][0]=-1;
        path[0][0] = location;
        findPath(dest,location,walls,path);
        int next_loc = path[1][0];
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
    }      
}

int findPath(int dest,int start,int walls[],int path[][2]){
    
    int wall_data = walls[start];
    if(wall_data == -1){
        path[0][1] = 100;
    }
    else{
        int last_fp = -1;
        int frontier_path[dim*dim];
        for(int i=0;i<dim*dim;i++){
          frontier_path[i] = -1;
        }
        if(wall_data % 2 == 1){
            last_fp++;
            frontier_path[last_fp+1] = start+dim;
        }
        if((wall_data >> 1) %2 == 1){
            last_fp++;
            frontier_path[last_fp+1] = start-dim;
        }
        if((wall_data >> 2) %2 == 1){
            last_fp++;
            frontier_path[last_fp+1] = start+1;
        }
        if((wall_data >> 3) %2 == 1){
            last_fp++;
            frontier_path[last_fp+1] = start-1;
        }

        int last_p = 0;
        while(path[last_p][0]>-1)
          last_p++;
        
        for(int i = 0; i<=last_p; i++){
            int point = path[i][0];
            c = 0;
            while (c <= last_fp){
                if (point == frontier[c]){
                    if (c == 0){ 
                        frontier[0] = frontier[last_fp];
                        last_fp--;
                    }
                    else if(c == last_fp){
                        frontier[last_fp] = -1;
                        last_fp--;
                    }
                    else{
                        frontier[c] = frontier[last_fp];
                        last_fp--;
                    }
                }
                else
                    c++;
            }
        }

        int i = 0;
        while(i <= last_fp){
            if( dest == frontier[last_fp]){
                path[last_p][0] = dest;
                path[last_p][1] = 1;
                break;
            }
        }

        int poss_path_best[9][2];
        int best_dist,poss_dist;
        for(i=0;i<=last_fp;i++){
                
                int new_start = frontier[c];
                path[last_p][0] = new_start;
                path[last_p][1] = 1;
                int new_path[9][2] = {path};
                findPath(dest,new_start,walls,new_path);
                if(i == 0){
                      for( int i = 0 ; i < 9 ; ++i ){
                        for(int j = 0 ; j < 2 ; ++j)
                          poss_path_best[i][j] = new_path[i][j];
                      }
                }
                else{
                    for(int k=0;k<9;k++){
                      best_dist += poss_path_best[k][1];
                      poss_dist += new_path[k][1];
                    }
                    if(poss_dist < best_dist)
                      for( int i = 0 ; i < 9 ; ++i ){
                        for(int j = 0 ; j < 2 ; ++j)
                          poss_path_best[i][j] = new_path[i][j];
                      }
                }
        }
        for( int i = 0 ; i < 9 ; ++i ){
           for(int j = 0 ; j < 2 ; ++j)
               path[i][j] = poss_path_best[i][j];
        }
    }

}


void sendMSG(int msg) {
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
  TIMSK0 = 0; // turn off timer0 for lower jitter
  ADCSRA = 0xe5; // set the adc to free running mode
  ADMUX = 0x40; // use adc0
  DIDR0 = 0x01; // turn off the digital input for adc0
  while(start == 0){
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
    if (fft_log_out[5] > 160) 
      start = 1;
    Serial.println(fft_log_out[5]);
  }
  TIMSK0 = prevTIMSK0;
  ADCSRA = prevADCSRA;
  ADMUX = prevADMUX;
  DIDR0 = prevDIDR0;

}
