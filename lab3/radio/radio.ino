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

int maze[21] = {
  B0000000011010000,
  B0000100001010000,
  B0001000000110000,
  B0001000101100000,
  B0000100101010000,
  B0000000110010000,
  B0000001010000000,
  B0000101001010000,
  B0001001001010000,
  B0001101000100000,
  B0001100110100000,
  B0001100010110000,
  B0001100110100000,
  B0001101000100000,
  B0001101111100000,
  B0001101000100000,
  B0001001001010000,
  B0000101001010000,
  B0000001010000000,
  B0000001111000000,
  B0000101101010000,
  B0001001101110000
}


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
}

void loop(void)
{
  //
  // Ping out role.  Repeatedly send the current time
  //

  if (role == role_ping_out)
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
      
  
      // Try again 1s later
      delay(1000);
      currentCell = currentCell+1;
    }
  }

  //
  // Pong back role.  Receive each packet, dump it out, and send it back
  //

  if ( role == role_pong_back )
  {
    // if there is data ready
    if ( radio.available() )
    {
      // Dump the payloads until we've gotten everything
      byte msg;
      bool done = false;
      while (!done)
      {
        // Fetch the payload, and see if this was the last one.
        done = radio.read( &msg, sizeof(byte) );

        String x = String(msg >> 11);
        String y = String(msg >> 8);
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
        String results = x+","+y+",north="+north+",east="+east+",south="+south+",west="+west+",tshape="+shape+",tcolor="+color;
//        results = results.concat(north);
//        results = results.concat(",east=");
//        results = results.concat(east);
//        results = results.concat(",south=");
//        results = results.concat(south);
//        results = results.concat(",west=");
//        results = results.concat(west);
//        results = results.concat(",tshape=");
//        results = results.concat(shape);
//        results = results.concat(",tcolor=");
//        results = results.concat(color);

        Serial.println(results);

        // Delay just a little bit to let the other unit
        // make the transition to receiver
        delay(20);

      }

      // First, stop listening so we can talk
      radio.stopListening();

      // Send the final one back.
      radio.write( &got_time, sizeof(unsigned long) );
      printf("Sent response.\n\r");

      // Now, resume listening so we catch the next packets.
      radio.startListening();
    }
  }

  //
  // Change roles
  //

  if ( Serial.available() )
  {
    char c = toupper(Serial.read());
    if ( c == 'T' && role == role_pong_back )
    {
      printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");

      // Become the primary transmitter (ping out)
      role = role_ping_out;
      radio.openWritingPipe(pipes[0]);
      radio.openReadingPipe(1,pipes[1]);
    }
    else if ( c == 'R' && role == role_ping_out )
    {
      printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");

      // Become the primary receiver (pong back)
      role = role_pong_back;
      radio.openWritingPipe(pipes[1]);
      radio.openReadingPipe(1,pipes[0]);
    }
  }
}
// vim:cin:ai:sts=2 sw=2 ft=cpp
