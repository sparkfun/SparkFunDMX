/*
  Read the 5 Channels of DMX Data coming from an ESP32 shield running Example 1
  By: Andy England
  SparkFun Electronics
  Date: , 2018
  License: GNU. See license file for more information but you can
  basically do whatever you want with this code.
  This example runs two servos and a number of LED's off of 5 DMX channels
  
  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15110
  Hardware Connections:
  Connect pan/tilt servos to pins DATA1 and DATA2, connect LEDs to CLOCK and DATA0. Connect a DMX XLR-3 Cable in between the Output and Input shields
*/

#include <SparkFunDMX.h>

SparkFunDMX dmx;

//Channel Definitions
#define TOTAL_CHANNELS 5

#define HUE_CHANNEL 1
#define SATURATION_CHANNEL 2
#define VALUE_CHANNEL 3
#define PAN_CHANNEL 4
#define TILT_CHANNEL 5

uint8_t x = 0;

void setup() {
  Serial.begin(115200);
  delay(3000);
  Serial.println("starting...");
  dmx.initWrite(TOTAL_CHANNELS);           // initialization for complete bus
  Serial.println("initialized...");
  delay(200);               // wait a while (not necessary)
}

void loop() {
  for (int channel = 1; channel <= TOTAL_CHANNELS; channel++) //We don't (and can't) write to channel 0 as it contains 0x00, the DMX start code
  {
    if (channel == SATURATION_CHANNEL || channel == VALUE_CHANNEL) //Write the same value (190/255) = 74.5% to both saturation and value.
    {
      dmx.write(channel, 190);
    }
    else if (channel == HUE_CHANNEL || channel == PAN_CHANNEL || channel == TILT_CHANNEL) //Sweep across our servos as well as a rainbow cycle.
    {
      dmx.write(channel, x);
    }
  }
  Serial.print(x++);//Print and increment x. Since x is an unsigned 8 bit integer it will loop back to 0 if we try to increment it past 255
  dmx.update(); // update the DMX bus witht he values that we have written
  Serial.println(": updated!");
  delay(100);
}
