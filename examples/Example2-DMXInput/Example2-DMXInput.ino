/*
  Reads DMX data from channel 1
  
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
  cable between the shield and another device that inputs DMX data. You can use
  a second board and shield running Example 1!
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
    Serial.println("SparkFun DMX Example 2 - Input");

    // Set pins for DMX serial port (may depend on platform)
    dmxSerial.setPins(rxPin, txPin);

    // Begin DMX
    dmx.begin(dmxSerial, rxPin, txPin, enPin, numChannels);

    // Set communicaiton direction, which can be changed on the fly as needed
    dmx.setComDir(DMX_READ_DIR);

    Serial.println("DMX initialized!");
}

void loop()
{
    // Wait until data has been received
    while(dmx.dataAvailable() == false)
    {
        // Update must be called to update received data
        dmx.update();
    }

    // Data has been received, read out channel 1
    uint8_t data = dmx.readByte(1);
    
    Serial.print("DMX: read value from channel 1: ");
    Serial.println(data);
}
