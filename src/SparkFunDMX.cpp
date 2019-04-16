// - - - - -
// ESPDMX - A Arduino library for sending and receiving DMX using the builtin serial hardware port.
// ESPDMX.cpp: Library implementation file
//
// Copyright (C) 2015  Rick <ricardogg95@gmail.com>
// This work is licensed under a GNU style license.
//
// Last change: Marcel Seerig <https://github.com/mseerig>
//
// Documentation and samples are available at https://github.com/Rickgg/ESP-Dmx
// - - - - -

/* ----- LIBRARIES ----- */
#include <Arduino.h>

#include "SparkFunDMX.h"
#include <HardwareSerial.h>

#define dmxMaxChannel  513
#define defaultMax 32

#define DMXSPEED       250000
#define DMXFORMAT      SERIAL_8N2

int enablePin = 21;		//dafault on ESP32
int rxPin = 16;
int txPin = 17;

uint32_t _lastTimeStamp = 0;

//DMX value array and size. Entry 0 will hold startbyte
uint8_t dmxData[dmxMaxChannel] = {};
int chanSize;
int currentChannel = 0;

HardwareSerial DMXSerial(2);

void SparkFunDMX::initRead(int chanQuant) {
  _READWRITE = _READ;
  if (chanQuant > dmxMaxChannel || chanQuant <= 0) 
  {
    chanQuant = defaultMax;
  }
  chanSize = chanQuant;
  pinMode(enablePin, OUTPUT);
  digitalWrite(enablePin, LOW);
  pinMode(rxPin, INPUT);
  _lastTimeStamp = micros();
  attachInterrupt(digitalPinToInterrupt(rxPin), startCode, CHANGE);
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
  return(dmxData[Channel]); //add one to account for start byte
}

// Function to send DMX data
void SparkFunDMX::write(int Channel, uint8_t value) {
  if (Channel < 0) Channel = 0;
  if (Channel > chanSize) chanSize = Channel;
  dmxData[0] = 0;
  dmxData[Channel] = value; //add one to account for start byte
}

void SparkFunDMX::startCode() {
  if ((micros() - _lastTimeStamp) >= 87)
  {
	currentChannel = 0;
	DMXSerial.begin(DMXSPEED, DMXFORMAT, rxPin, txPin);
  }
  _lastTimeStamp = micros();
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
    pinMatrixOutAttach(txPin, U2TXD_OUT_IDX, false, false);

    DMXSerial.write(dmxData, chanSize);
    DMXSerial.flush();
    DMXSerial.end();//clear our DMX array, end the Hardware Serial port
  }
  else if (_READWRITE == _READ)//In a perfect world, this function ends serial communication upon packet completion and attaches RX to a CHANGE interrupt so the start code can be read again
  { 
    while (DMXSerial.available())
	{
	  if (currentChannel == 0)
	  {
	  	DMXSerial.read();
	  }
	  dmxData[currentChannel++] = DMXSerial.read();
	}
	if (currentChannel > chanSize) //Set the channel counter back to 0 if we reach the known end size of our packet
	{
	  currentChannel = 0;
	}
  }
}

// Function to update the DMX bus
