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
#include <SparkFunDMX.h>
#include <ESP32Servo.h>

//Wifi settings
char ssid[] = "esp32devnet"; //Change these lines to an existing SSID and Password if you're trying to connect to an existing network
char password[] = "password";

// Artnet settings
ArtnetWifi artnet;
const int startUniverse = 0;
const int endUniverse = 0;//end Universe should be total channels/512

SparkFunDMX dmx;
WiFiUdp UdpSend;
bool sendFrame = 1;
int previousDataLength = 0;

//Channel and Peripheral Definitions
#define NUM_LEDS 64
#define NUM_LED_CHANNELS NUM_LEDS * 3 //Ends up being 192 channels for our 8x8 LED matrix
#define PAN_CHANNEL 193
#define TILT_CHANNEL 194
#define MAX_CHANNEL 513
CRGB matrix[NUM_LEDS];
Servo pan;
Servo tilt;

uint8_t hue = 0;

boolean connectWifi(void) //Sets our ESP32 device up as an access point
{
  boolean state = true;
  //WiFi.mode(WIFI_AP_STA);
  //state = WiFi.softAP(ssid, password);
  //Comment out the above two lines and uncomment the below line to connect to an existing network specified on lines 8 and 9
  state = WiFi.begin(ssid, password);
  return state;
}

void onDmxFrame(uint16_t universe, uint16_t length, uint8_t sequence, uint8_t* data)
{
  sendFrame = 1;
  //Read universe and put into the right part of the display buffer
  //DMX data should be sent with the first LED in the string on channel 0 of Universe 0
  Serial.println(length);
  for (int channel = 0; channel < length; channel++)
  {
    dmx.write(channel + 1, data[channel]);//Add one for offset
  }
  previousDataLength = length;
  if (universe == endUniverse) //Display our data if we have received all of our universes, prevents incomplete frames when more universes are concerned.
  {
    dmx.update();
	UdpSend.flush();
  }
}

void setup()
{
  Serial.begin(115200);
  //Fixture Hardware Declarations
  dmx.initWrite(MAX_CHANNEL);//Resolume sends full ArtNet packets so that's what we listen for

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
