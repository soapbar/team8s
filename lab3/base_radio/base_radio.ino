/*
 Copyright (C) 2011 J. Coliz <maniacbug@ymail.com>
 This program is free software; you can redistribute it and/or
 modify it under the terms of the GNU General Public License
 version 2 as published by the Free Software Foundation.
 */

/**
 * Example for Getting Started with nRF24L01+ radios.
 *
 * This is an example of how to use the RF24 class.  Write this sketch to two
 * different nodes.  Put one of the nodes into 'transmit' mode by connecting
 * with the serial monitor and sending a 'T'.  The ping node sends the current
 * time to the pong node, which responds by sending the value back.  The ping
 * node can then see how long the whole cycle took.
 */

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"

//
// Hardware configuration
//

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10

RF24 radio(9,10);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0x0000000016LL, 0x00000000017LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;

unsigned long got_time;

int currentCell;

int maze[22] = {
  0x00D0,//  B0000|0000|1101|0000,
  0x0150,//  B0000|0001|0101|0000,
  0x0230,//  B0000|0010|0011|0000,
  0x0660,//  B0000|0110|0110|0000,
  0x0550,//  B0000|0101|0101|0000,
  0x0490,//  B0000|0100|1001|0000,
  0x0880,//  B0000|1000|1000|0000,
  0x0950,//  B0000|1001|0101|0000,
  0x0A50,//  B0000|1010|0101|0000,
  0x0B20,//  B0000|1011|0010|0000,
  0x07A0,//  B0000|0111|1010|0000,
  0x03B0,//  B0000|0011|1011|0000,
  0x07A0,//  B0000|0111|1010|0000,
  0x0B20,//  B0000|1011|0010|0000,
  0x0FE0,//  B0000|1111|1110|0000,
  0x0B20,//  B0000|1011|0010|0000,
  0x0A50,//  B0000|1010|0101|0000,
  0x0950,//  B0000|1001|0101|0000,
  0x0880,//  B0000|1000|1000|0000,
  0x0CC0,//  B0000|1100|1100|0000,
  0x0D50,//  B0000|1101|0101|0000,
  0x0E70,//  B0000|1110|0111|0000
};

bool didZero = false;
int dim = 3;


void setup(void)
{
  //
  // Print preamble
  //
  currentCell = 0;
  Serial.begin(9600);
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

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

  // optionally, reduce the payload size.  seems to
  // improve reliability
  radio.setPayloadSize(2);

  //
  // Open pipes to other nodes for communication
  //

  role = role_pong_back;
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();

  role = role_pong_back;
  radio.openWritingPipe(pipes[1]);
  radio.openReadingPipe(1,pipes[0]);
}

void loop(void) {
  // if there is data ready
  if ( radio.available() )
  {
    // Dump the payloads until we've gotten everything
    int msg;
    bool done = false;
    while (!done)
    {
      // Fetch the payload, and see if this was the last one.
      done = radio.read( &msg, sizeof(int) );

      int location = (msg >> 8) & 0x003F;
      String x = String(location%dim);
      String y = String(location/dim);

      String north = "false";
      String east = "false";
      String south = "false";
      String west = "false";
      String shape;
      String color;

      if ((msg & B10000000) == B10000000) 
        north = "true";

      if ((msg & B01000000) == B01000000) 
        east = "true";

      if ((msg & B00100000) == B00100000) 
        south = "true";

      if ((msg & B00010000) == B00010000) 
        west = "true";

      switch (msg & B00001100) {
        case B00000000:
          shape = "none";
          break;
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
        case B00000000:
          color = "none";
          break;
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
      String results = x+","+y+",north="+north+",east="+east+",south="+south+",west="+west+",tshape="+shape+",tcolor="+color+",robot=false";
      if (!didZero || !(x.toInt() == 0 && y.toInt() == 0))
        Serial.println(results);

      if (x.toInt() == 0 && y.toInt() == 0) didZero = true;
      // Delay just a little bit to let the other unit
      // make the transition to receiver
      delay(20);

    }

    // First, stop listening so we can talk
    radio.stopListening();

    // Send the final one back.
    radio.write( &got_time, sizeof(unsigned long) );

    // Now, resume listening so we catch the next packets.
    radio.startListening();
  }

}
