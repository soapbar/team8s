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


void setup(void)
{
  //
  // Print preamble
  //
  currentCell = 0;
  Serial.begin(57600);
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


  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();

  role = role_ping_out;
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
}

void loop(void)
{
  while (currentCell <= 21) {
    // First, stop listening so we can talk.
    radio.stopListening();
    
    // Take the time, and send it.  This will block until complete
    unsigned long time = millis();
    int msg = maze[currentCell];
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
        bool ok = radio.write( &msg, sizeof(byte) );
    
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
    

    // Wait 1 sec before sending next
    delay(1000);
    currentCell = currentCell+1;
  }
  while(1);
}
