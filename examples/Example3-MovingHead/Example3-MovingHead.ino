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

#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArtnetWifi.h> //https://github.com/rstephan/ArtnetWifi
#include <FastLED.h>
#include <ESP32Servo.h>

//Wifi settings
char ssid[] = "myDMX"; //Change these lines to an existing SSID and Password if you're trying to connect to an existing network
char password[] = "artnetnode";

// Artnet settings
ArtnetWifi artnet;
WiFiUdp UdpSend;
const int startUniverse = 0;
const int endUniverse = 0;//end Universe should be total channels/512

bool sendFrame = 1;
int previousDataLength = 0;

//Pin Definitions for ESP32 WROOM
#define CLOCK 5
#define DATA0 19
#define DATA1 18
#define DATA2 27

//Channel and Peripheral Definitions
#define NUM_LEDS 64
#define NUM_LED_CHANNELS NUM_LEDS * 3 //Ends up being 192 channels for our 8x8 LED matrix
#define PAN_CHANNEL 193
#define TILT_CHANNEL 194
CRGB matrix[NUM_LEDS];
Servo pan;
Servo tilt;

uint8_t hue = 0;

boolean connectWifi(void) //Sets our ESP32 device up as an access point
{
  boolean state = true;
  WiFi.mode(WIFI_AP_STA);
  state = WiFi.softAP(ssid, password);
  //Comment out the above two lines and uncomment the below line to connect to an existing network specified on lines 8 and 9
  //state = WiFi.begin(ssid, password);
  return state;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  // read universe and put into the right part of the display buffer
  //DMX data should be sent with the first LED in the string on channel 0 of Universe 0
  for (int channel = 0; channel < length; channel++)
  {
    if (channel < NUM_LED_CHANNELS && channel % 3 == 0) //Only write on every 3rd piece of data so we correctly parse things into our RGB array
    {
      matrix[channel / 3] = CRGB(data[channel], data[channel + 1], data[channel + 2]);
    }
    else if (channel == PAN_CHANNEL - 1) //Subtract 1 due to the fact that we index at 0 and ignore the startcode
    {
      pan.write(map(data[channel], 0, 255, 0, 160));
    }
    else if (channel == TILT_CHANNEL - 1)
    {
      tilt.write(map(data[channel], 0, 255, 0, 160));
    }
  }
  previousDataLength = length;
  if (universe == endUniverse) //Display our data if we have received all of our universes, prevents incomplete frames when more universes are concerned.
  {
    FastLED.show();
	UdpSend.flush();
  }
}

void setup()
{
  Serial.begin(115200);
  //Fixture Hardware Declarations
  FastLED.addLeds<APA102, DATA0, CLOCK, BGR>(matrix, NUM_LEDS);
  pan.attach(DATA1);
  tilt.attach(DATA2);

  if (connectWifi())
  {
    Serial.println("Connected!");
  }
  artnet.begin();
  artnet.setArtDmxCallback(onDmxFrame);
}

void loop()
{
  artnet.read();
}
