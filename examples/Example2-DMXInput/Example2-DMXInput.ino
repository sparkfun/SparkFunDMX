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
#include <FastLED.h>
#include <ESP32Servo.h>

SparkFunDMX dmx;

//Pin Definitions for ESP32 WROOM
#define CLOCK 5
#define DATA0 19
#define DATA1 18
#define DATA2 27

//Channel Defintions
#define TOTAL_CHANNELS 5

#define HUE_CHANNEL 1
#define SATURATION_CHANNEL 2
#define VALUE_CHANNEL 3
#define PAN_CHANNEL 4
#define TILT_CHANNEL 5

//Fixture Hardware Definitinos
#define NUM_LEDS 64
CRGB matrix[NUM_LEDS];
Servo pan;
Servo tilt;

uint8_t x = 0;
int myDirection = 1;

void setup()
{
  Serial.begin(115200);
  dmx.initRead(TOTAL_CHANNELS);           // initialization for complete bus
  Serial.println("initialized...");

  FastLED.addLeds<APA102, DATA0, CLOCK, BGR>(matrix, NUM_LEDS);
  FastLED.setBrightness(16);
  pan.attach(DATA1);
  tilt.attach(DATA2);
}

void loop()
{
  dmx.update();
  for (int led = 0; led < NUM_LEDS; led++)
  {
    matrix[led] = CHSV(dmx.read(HUE_CHANNEL), dmx.read(SATURATION_CHANNEL), dmx.read(VALUE_CHANNEL)); //Read the same value into all of our LED's
  }
  pan.write(map(dmx.read(PAN_CHANNEL), 0, 255, 0, 160));
  tilt.write(map(dmx.read(TILT_CHANNEL), 0, 255, 0, 160));
  FastLED.show();
}
