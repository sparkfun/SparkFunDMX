/*
  Writes DMX data to channel 1

  By: Dryw Wade
  SparkFun Electronics
  Date: 10/3/2023
  License: GNU. See license file for more information but you can
  basically do whatever you want with this code.
  This example runs two servos and a number of LED's off of 5 DMX channels
  
  Feel like supporting open source hardware?
  Buy a board from SparkFun! https://www.sparkfun.com/products/15110
  
  Hardware Connections:
  Connect a Thing Plus board to the SparkFun DMX Shield, and connect a DMX XLR-3
  cable between the shield and another device that outputs DMX data. You can use
  a second board and shield running Example 2!
*/

// Inlcude DMX library
#include <SparkFunDMX.h>

// Create DMX object
SparkFunDMX dmx;

// Create serial port to be used for DMX interface. Exact implementation depends
// on platform, this example is for the ESP32
HardwareSerial dmxSerial(2);

// Pin definitions, required by DMX library
// These are for the SparkFun ESP32 Thing Plus (Micro-B)
uint8_t rxPin = 16;
uint8_t txPin = 17;
uint8_t enPin = 21;

// Number of DMX channels, can be up tp 512
uint16_t numChannels = 1;

// Create a counter for demonstration
uint8_t counter = 0;

void setup()
{
    Serial.begin(115200);
    Serial.println("SparkFun DMX Example 1 - Output");

    // Set pins for DMX serial port (may depend on platform)
    dmxSerial.setPins(rxPin, txPin);

    // Begin DMX
    dmx.begin(dmxSerial, rxPin, txPin, enPin, numChannels);

    // Set communicaiton direction, which can be changed on the fly as needed
    dmx.setComDir(DMX_WRITE_DIR);

    Serial.println("DMX initialized!");
}

void loop()
{
    // Write counter to channel 1
    dmx.writeByte(counter, 1);

    // Once all data has been written, update() must be called to actually send it
    dmx.update();

    Serial.print("DMX: sent value to channel 1: ");
    Serial.println(counter);

    // Increment counter (overflows back to 0 after 255)
    counter++;

    // Slow down communication for this example
    delay(100);
}
