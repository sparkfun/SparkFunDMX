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
  delay(3000);
  Serial.println("starting...");
  dmx.initRead(TOTAL_CHANNELS);           // initialization for complete bus
  Serial.println("initialized...");

  FastLED.addLeds<APA102, DATA0, CLOCK, BGR>(matrix, NUM_LEDS);
  FastLED.setBrightness(16);
  pan.attach(DATA1);
  tilt.attach(DATA2);
  delay(200);               // wait a while (not necessary)
}

void loop()
{
  dmx.update();
  for (int led = 0; led < NUM_LEDS; led++)
  {
    matrix[led] = CHSV(dmx.read(HUE_CHANNEL), dmx.read(SATURATION_CHANNEL), dmx.read(VALUE_CHANNEL));
  }
  pan.write(map(dmx.read(PAN_CHANNEL), 0, 255, 0, 160));
  tilt.write(map(dmx.read(TILT_CHANNEL), 0, 255, 0, 160));
  FastLED.show();
}
