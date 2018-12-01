#include <Servo.h>

#define LOG_OUT 1 // use the log output function
#define FFT_N 128 // set to 256 point fft
#include <FFT.h> // include the library
#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//-----------------------------------------------------------
// Adjustable parameters

int start = 0;
int dir   = 1; //0-North, 1-East, 2-South, 3-West

//-----------------------------------------------------------

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

void waitForSignal();
/* The direction that the robot is currently facing:
    - 0-North
    - 1-East
    - 2-South
    - 3-West
   The coordinates of the robot are encoded as a single number
    - x=location%dim
    - y=location/dim
*/

int dim = 9; // dimentions of the maze
int location;

int settled [81];
int frontier [81];
int walls [81]; //TEMP FIX FIX THIS FIX THIS

int last_s = 0;
int last_f = -1;

void setup() {
  // IR Setup
  Serial.begin(9600); // use the serial port
  pinMode(MS0, OUTPUT); //MUX
  pinMode(MS1, OUTPUT);
  pinMode(MS2, OUTPUT);
  
  pinMode(0, INPUT); // Backup Start Button

  waitForSignal();

  // Servo Setup
  LeftServo.attach(OUTLEFT);
  RightServo.attach(OUTRIGHT);
  location = start;
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
    int msg = 0;
    LeftServo.write(90);
    RightServo.write(90);
    msg = checkIR(msg);
    delay(100);
    msg = checkWall(msg);
    LeftServo.write(90);
    RightServo.write(90);
    msg = updateCoord(msg);  
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
  while (fft_log_out[21] > 160) {
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
  
  // update direction
  dir = dir-1;
  if(dir < 0){
    dir = dir+4;   
  }
  dir = dir%4;
  
  delay(400);

}
void sharpRight() {
  LeftServo.write(160);
  RightServo.write(95);
  dir = (dir+1)%4; // update direction
  delay(400);
}

void oneEighty() {
  LeftServo.write(160);
  RightServo.write(95);
  dir = (dir+2)%4; //update direction
  delay(1000);
}

void fullStop() {
  LeftServo.write(90);
  RightServo.write(90);
  while (1);
}

//Finds and Goes to new coordinate
int updateCoord(int msg) {
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
  else{
    fullStop();
  }
  goStraight();
  delay(300);  
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

    int wall_data = walls[location];
    settled[last_s] = location;
    last_s++;
    
    if(wall_data % 2 != 1) {
        last_f++;
        frontier[last_f] = location+dim;
    }
    if((wall_data >> 1) %2 != 1) {
        last_f++;
        frontier[last_f] = location-dim;        
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
        int point = settled[i];
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
        int min_dist = dim*dim+1;
        int dest = -1;
        int dest_ind = -1;
        for(int i=0;i<=last_f;i++){
          int xdist = abs(frontier[i]-location)%dim;
          int ydist = abs(frontier[i]-location)/dim;
          int dist = xdist+ydist;
          if(dist < min_dist){
            min_dist = dist;
            dest = frontier[i];
            dest_ind = i;
          }
        }
        int path[9][2];
        for( int i = 0 ; i < 81 ; i++ ){
             for(int j = 0 ; j < 2 ; j++)
                 path[i][j] = -1;
          }
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
        return next_loc;
    }      
}

void findPath(int dest,int start,int walls[],int path[][2]){
    
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
        
        if(wall_data % 2 != 1){
            last_fp++;
            frontier_path[last_fp] = start+dim;
        }
        if((wall_data >> 1) %2 != 1){
            last_fp++;
            frontier_path[last_fp] = start-dim;
        }
        if((wall_data >> 2) %2 != 1){
            last_fp++;
            frontier_path[last_fp] = start+1;
        }
        if((wall_data >> 3) %2 != 1){
            last_fp++;
            frontier_path[last_fp] = start-1;
        }
        
        int last_p = 0;
        while(path[last_p][0]>-1){
          last_p++;
        }
        
        for(int i = 0; i<last_p; i++){
            int point = path[i][0];
            c = 0;
            while (c <= last_fp){
                if (point == frontier_path[c] || frontier_path[c] < 0 || frontier_path[c] >= dim*dim){
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
        while(i <= last_fp && found == 0){
            if( dest == frontier_path[i]){
                found = 1;
                path[last_p][0] = dest;
                path[last_p][1] = 1;
            }
            else
              i++;
        }
        
        if(found == 0){
          int poss_path_best[9][2];
          for(int i=0;i<dim*dim;i++){
            for(int j=0;j<2;j++)
              poss_path_best[i][j]=-1;
          }
          
          int best_dist,poss_dist;
          
          for(i=0;i<=last_fp;i++){
                  
                  int new_start = frontier_path[i];
                  path[last_p][0] = new_start;
                  path[last_p][1] = 1;
                  int new_path[9][2];
                  for( int i = 0 ; i < dim*dim ; i++ ){
                    for(int j = 0 ; j < 2 ; j++)
                        new_path[i][j] = path[i][j];
                  }
                  
                  findPath(dest,new_start,walls,new_path);
                  if(i == 0){
                        for( int i = 0 ; i < dim*dim ; ++i ){
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
                        for( int i = 0 ; i < dim*dim ; i++ ){
                          for(int j = 0 ; j < 2 ; j++)
                            poss_path_best[i][j] = new_path[i][j];
                        }
                  }
          }
          
          for( int i = 0 ; i < dim*dim ; i++ ){
             for(int j = 0 ; j < 2 ; j++){
                 path[i][j] = poss_path_best[i][j];
             }
          }
        }
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
