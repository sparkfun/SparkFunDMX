/******************************************************************************
SparkFunDMX.h
Arduino Library for the SparkFun ESP32 LED to DMX Shield
Andy England @ SparkFun Electronics
7/22/2019

Development environment specifics:
Arduino IDE 1.6.4

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

// Verify this is a valid platform (SparkFun ESP32 Thing Plus boards only)
#if !(defined(ARDUINO_ESP32_THING_PLUS) \
  || defined(ARDUINO_ESP32_THING_PLUS_C) \
  || defined(ARDUINO_ESP32S2_THING_PLUS))
#error SparkFun DMX library can only be used with ESP32 Thing Plus boards!
#endif

/* ----- LIBRARIES ----- */
#include <Arduino.h>

#include "SparkFunDMX.h"
#include <HardwareSerial.h>

#define dmxMaxChannel  513
#define defaultMax 32

#define DMXSPEED       250000
#define DMXFORMAT      SERIAL_8N2

#if defined(ARDUINO_ESP32_THING_PLUS)
int enablePin = 21;
int rxPin = 16;
int txPin = 17;
#elif defined(ARDUINO_ESP32_THING_PLUS_C)
int enablePin = 4;
int rxPin = 16;
int txPin = 17;
#elif defined(ARDUINO_ESP32S2_THING_PLUS)
int enablePin = 3;
int rxPin = 33;
int txPin = 34;
#endif

//DMX value array and size. Entry 0 will hold startbyte
uint8_t dmxData[dmxMaxChannel] = {};
int chanSize;
int currentChannel = 0;

#if defined(ARDUINO_ESP32_THING_PLUS) || defined(ARDUINO_ESP32_THING_PLUS_C)
HardwareSerial DMXSerial(2);
#elif defined(ARDUINO_ESP32S2_THING_PLUS)
HardwareSerial DMXSerial(1);
#endif

/* Interrupt Timer for DMX Receive */
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

volatile int _interruptCounter;
volatile bool _startCodeDetected = false;

volatile bool ledPin = false;

/* Start Code is detected by 21 low interrupts */
void IRAM_ATTR onTimer() {
	if (digitalRead(rxPin) == 1)
	{
		_interruptCounter = 0; //If the RX Pin is high, we are not in an interrupt
	}
	else
	{
		_interruptCounter++;
	}
	if ((_interruptCounter > 9) && (_startCodeDetected == false))
	{	
		portENTER_CRITICAL_ISR(&timerMux);
		_startCodeDetected = true;
		DMXSerial.begin(DMXSPEED, DMXFORMAT, rxPin, txPin);
		portEXIT_CRITICAL_ISR(&timerMux);
		_interruptCounter = 0;
	}
}

void SparkFunDMX::initRead(int chanQuant) {
	
  timer = timerBegin(0, 2, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 320, true);
  timerAlarmEnable(timer);
  _READWRITE = _READ;
  if (chanQuant > dmxMaxChannel || chanQuant <= 0) 
  {
    chanQuant = defaultMax;
  }
  chanSize = chanQuant;
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW);
  pinMode(rxPin, INPUT);
}

// Set up the DMX-Protocol
void SparkFunDMX::initWrite (int chanQuant) {

  _READWRITE = _WRITE;
  if (chanQuant > dmxMaxChannel || chanQuant <= 0) {
    chanQuant = defaultMax;
  }

  chanSize = chanQuant + 1; //Add 1 for start code

  DMXSerial.begin(DMXSPEED, DMXFORMAT, rxPin, txPin);
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, HIGH);
}

// Function to read DMX data
uint8_t SparkFunDMX::read(int Channel) {
  if (Channel > chanSize) Channel = chanSize;
  return(dmxData[Channel - 1]); //subtract one to account for start byte
}

// Function to send DMX data
void SparkFunDMX::write(int Channel, uint8_t value) {
  if (Channel < 0) Channel = 0;
  if (Channel > chanSize) chanSize = Channel;
  dmxData[0] = 0;
  dmxData[Channel] = value; //add one to account for start byte
}



void SparkFunDMX::update() {
  if (_READWRITE == _WRITE)
  {
	DMXSerial.begin(DMXSPEED, DMXFORMAT, rxPin, txPin);//Begin the Serial port
    pinMatrixOutDetach(txPin, false, false); //Detach our
    pinMode(txPin, OUTPUT); 
    digitalWrite(txPin, LOW); //88 uS break
    delayMicroseconds(88);  
    digitalWrite(txPin, HIGH); //4 Us Mark After Break
    delayMicroseconds(1);
    #ifdef U2TXD_OUT_IDX // Some ESP32 boards only have 2 UARTs
    pinMatrixOutAttach(txPin, U2TXD_OUT_IDX, false, false);
    #else
    pinMatrixOutAttach(txPin, U1TXD_OUT_IDX, false, false);
    #endif

    DMXSerial.write(dmxData, chanSize);
    DMXSerial.flush();
    DMXSerial.end();//clear our DMX array, end the Hardware Serial port
  }
  else if (_READWRITE == _READ)//In a perfect world, this function ends serial communication upon packet completion and attaches RX to a CHANGE interrupt so the start code can be read again
  { 
	if (_startCodeDetected == true)
	{
    // If there are extra bytes in serial buffer, remove them
    while(DMXSerial.available() > chanSize)
    {
      DMXSerial.read();
    }
		while (DMXSerial.available())
		{
			dmxData[currentChannel++] = DMXSerial.read();
		}
	if (currentChannel > chanSize) //Set the channel counter back to 0 if we reach the known end size of our packet
	{
		
      portENTER_CRITICAL(&timerMux);
	  _startCodeDetected = false;
	  DMXSerial.flush();
	  DMXSerial.end();
      portEXIT_CRITICAL(&timerMux);
	  currentChannel = 0;
	}
	}
  }
}

// Function to update the DMX bus
