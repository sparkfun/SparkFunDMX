/******************************************************************************
SparkFunDMX.h
Arduino Library for the SparkFun ESP32 LED to DMX Shield
Dryw Wade @ SparkFun Electronics
10/3/2023

Development environment specifics:
Arduino IDE 2.2.1

This code is released under the [MIT License](http://opensource.org/licenses/MIT).
Please review the LICENSE.md file included with this example. If you have any questions 
or concerns with licensing, please contact techsupport@sparkfun.com.
Distributed as-is; no warranty is given.
******************************************************************************/

/* ----- LIBRARIES ----- */
#include "SparkFunDMX.h"

// Static member definitions
uint8_t SparkFunDMX::_rxPin;
uint8_t SparkFunDMX::_txPin;
uint8_t SparkFunDMX::_enPin;
uint16_t SparkFunDMX::_numChannels;
uint8_t SparkFunDMX::_dmxBuffer[DMX_MAX_CHANNELS];
HardwareSerial*SparkFunDMX:: _dmxSerial;
bool SparkFunDMX::_comDir = DMX_READ_DIR;
bool SparkFunDMX::_dataAvailable = false;
bool SparkFunDMX::_synced = false;
uint32_t SparkFunDMX::_tStartBreak = 0;

void SparkFunDMX::_rxISR()
{
    // If communication direction is writing, skip
    if(_comDir == DMX_WRITE_DIR)
        return;
    
    // If already synced, skip
    if(_synced == true)
        return;
    
    // Check if pin went low or high
    if(digitalRead(_rxPin) == LOW)
    {
        // Pin just went low, start timer
        _tStartBreak = micros();
    }
    else
    {
        // Pin just went high, check if low pulse was long enough to be the
        // syncronization break signal
        if((micros() - _tStartBreak) >= (DMX_BREAK_DURATION_MICROS - DMX_BREAK_DURATION_MARGIN))
        {
            // This must have been the sync signal, update flag and flush serial
            _synced = true;
            _dmxSerial->flush(false);
        }
    }
}

void SparkFunDMX::begin(HardwareSerial& port, uint8_t rxPin, uint8_t txPin, uint8_t enPin, uint16_t numChannels)
{
    // Store serial stream port
    _dmxSerial = &port;

    // Store pin assignments
    _rxPin = rxPin;
    _txPin = txPin;
    _enPin = enPin;

    // Store number of requested channels, ensuring it's not above the limit
    _numChannels = numChannels+1;
    if(_numChannels > DMX_MAX_CHANNELS)
        _numChannels = DMX_MAX_CHANNELS;

    // Begin serial
    _dmxSerial->begin(DMX_BAUD, DMX_FORMAT);

    // Attach interrupt to RX pin for synchronization
    attachInterrupt(digitalPinToInterrupt(_rxPin), _rxISR, CHANGE);

    // Configure enable pin
    pinMode(_enPin, OUTPUT);
    setComDir(DMX_READ_DIR);
}

void SparkFunDMX::setComDir(bool comDir)
{
    // Store communication direction
    _comDir = comDir;
    
    // Flush serial buffer contents
    _dmxSerial->flush(false);

    if(comDir == DMX_WRITE_DIR)
    {
        digitalWrite(_enPin, HIGH);
    }
    else // DMX_READ_DIR
    {
        digitalWrite(_enPin, LOW);
    }
}

void SparkFunDMX::writeBytes(uint8_t* data, uint16_t numBytes, uint16_t startChannel)
{
    // Copy data into buffer
    uint8_t* startPtr = _dmxBuffer + startChannel;
    memcpy(startPtr, data, numBytes);
}

void SparkFunDMX::writeByte(uint8_t data, uint16_t channel)
{
    _dmxBuffer[channel] = data;
}

void SparkFunDMX::readBytes(uint8_t* data, uint16_t numBytes, uint16_t startChannel)
{
    // Copy data into buffer
    uint8_t* startPtr = _dmxBuffer + startChannel;
    memcpy(data, startPtr, numBytes);

    _dataAvailable = false;
}

uint8_t SparkFunDMX::readByte(uint16_t channel)
{
    _dataAvailable = false;
    return _dmxBuffer[channel];
}

bool SparkFunDMX::dataAvailable()
{
    return _dataAvailable;
}

bool SparkFunDMX::update()
{
    if(_comDir == DMX_WRITE_DIR)
    {
        // We need to send a break signal to indicate the start of the message.
        // Arduino doesn't really have a way to actually send a break signal, so
        // this is a hacky solution. The baud rate is reduced such that sending
        // a zero byte creates a sufficiently long pulse to simulate a break
        
        // Reduce baud rate
        uint32_t breakBaud = 1000000 * 8 / DMX_BREAK_DURATION_MICROS;
        _dmxSerial->updateBaudRate(breakBaud);

        // Send a zero at this new baud rate
        _dmxSerial->write(0);
        _dmxSerial->flush();

        // Return baud rate to original value
        _dmxSerial->updateBaudRate(DMX_BAUD);

        // Send message
        _dmxSerial->write(_dmxBuffer, _numChannels);

        // Success
        return true;
    }
    else // DMX_READ_DIR
    {
        // Check if the break signal has been synced, and we've received the
        // amount of data expected
        if(_synced && (_dmxSerial->available() >= _numChannels))
        {
            // Read bytes into buffer
            _dmxSerial->read(_dmxBuffer, _numChannels);
            
            // Reset flags
            _dataAvailable = true;
            _synced = false;
            
            // Success
            return true;
        }
    }

    // Something went wrong (most likely haven't received all the bytes yet)
    return false;
}