#include "nRF24L01.h"
#include "RF24.h"

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000016LL, 0x00000000017LL }

void setup() {
  Serial.begin(9600);
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

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  if ( role == role_ping_out )
  {
    radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  else
  {
    radio.openWritingPipe(pipes[1]);
    radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  radio.begin();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
}

void loop() {

  // CELL (0,0)
  byte msg = B11010000;
  int x = 0;
  int y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,0)
  msg = B01010000;
  x = 1;
  y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,0)
  msg = B00110000;
  x = 2;
  y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,1)
  msg = B01100000;
  x = 2;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,1)
  msg = B01010000;
  x = 1;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,1)
  msg = B10010000;
  x = 0;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,2)
  msg = B10000000;
  x = 0;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,2)
  msg = B01010000;
  x = 1;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,2)
  msg = B01010000;
  x = 2;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,2)
  msg = B00100000;
  x = 3;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,1)
  msg = B10100000;
  x = 3;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,0)
  msg = B10110000;
  x = 3;
  y = 0;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,1)
  msg = B10100000;
  x = 3;
  y = 1;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,2)
  msg = B00100000;
  x = 3;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,3)
  msg = B11100000;
  x = 3;
  y = 3;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (3,2)
  msg = B00100000;
  x = 3;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,2)
  msg = B01010000;
  x = 2;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,2)
  msg = B01010000;
  x = 1;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,2)
  msg = B10000000;
  x = 0;
  y = 2;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (0,3)
  msg = B11000000;
  x = 0;
  y = 3;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (1,3)
  msg = B01010000;
  x = 1;
  y = 3;
  updateGUI(msg, x, y);
  delay(1000);

  // CELL (2,3)
  msg = B01110000;
  x = 2;
  y = 3;
  updateGUI(msg, x, y);
  while(1);
}

void updateGUI(byte msg, int x, int y) {
  String north="false";
  String east="false";
  String south="false";
  String west="false";
  String shape;
  String color;

  String xcoord = String(x);
  String ycoord = String(y);

  if ((msg & B10000000) == B10000000) 
    north = "true";

  if ((msg & B01000000) == B01000000) 
    east = "true";

  if ((msg & B00100000) == B00100000) 
    south = "true";

  if ((msg & B00010000) == B00010000) 
    west = "true";

  switch (msg & B00001100) {
    case B00000100:
      shape = "circle";
      break;
    case B00001000:
      shape = "triangle";
      break;
    case B00001100:
      shape = "square";
      break;
    default:
      shape = "none";
      break;
  }

  switch (msg & B00000011) {
    case B00000001:
      color = "red";
      break;
    case B00000010:
      color = "green";
      break;
    case B00000011:
      color = "blue";
      break;
    default:
      color = "none";
      break;
  }

  //Serial.println(xcoord+","+ycoord+","+"north="+north+",east="+east+",south="+south+",west="+west+",tshape="+shape+",tcolor="+color);
  Serial.println(xcoord+","+ycoord+","+"north="+north+",east="+east+",south="+south+",west="+west+",robot=false");
}
