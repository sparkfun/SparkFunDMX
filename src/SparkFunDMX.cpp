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

void SparkFunDMX::begin(HardwareSerial& port, uint8_t enPin, uint16_t numChannels)
{
    // Store serial stream port
    _dmxSerial = &port;

    // Store enable pin
    _enPin = enPin;

    // Store number of requested channels, plus 1 for channel 0
    _numChannels = numChannels + 1;

    // Ensure number of channels is not above the limit
    if(_numChannels > DMX_MAX_CHANNELS)
        _numChannels = DMX_MAX_CHANNELS;

    memset(_dmxBuffer, 0, _numChannels);

    // Configure enable pin, default to reading
    pinMode(_enPin, OUTPUT);

    _comDir = DMX_WRITE_DIR;
    setComDir(DMX_READ_DIR);

    _dataAvailable = false;
}

void SparkFunDMX::setComDir(bool comDir)
{
    // No need to do anything if this direction is already set
    if(_comDir == comDir)
        return;

    // Store communication direction
    _comDir = comDir;
    
    // Flush serial buffer contents
    _dmxSerial->flush(false);

    if(comDir == DMX_WRITE_DIR)
    {
        // Enable output
        digitalWrite(_enPin, HIGH);
    }
    else // DMX_READ_DIR
    {
        // Disable output
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
    // Store data
    _dmxBuffer[channel] = data;
}

void SparkFunDMX::readBytes(uint8_t* data, uint16_t numBytes, uint16_t startChannel)
{
    // Clear flag, this is now old data
    _dataAvailable = false;

    // Copy data into buffer
    uint8_t* startPtr = _dmxBuffer + startChannel;
    memcpy(data, startPtr, numBytes);
}

uint8_t SparkFunDMX::readByte(uint16_t channel)
{
    // Clear flag, this is now old data
    _dataAvailable = false;

    // Return requested data
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
        // this is a hacky solution: the baud rate is reduced such that sending
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
    // else _comDir == DMX_READ_DIR
    // Check if we've received the amount of data expected, +1 for break signal
    else if(_dmxSerial->available() >= (_numChannels + 1))
    {
        // We need to detect a break signal indicating the start of the message.
        // Arduino doesn't really have a way to actually read a break signal, it
        // will instead appear as an extra zero byte at the start. This has the
        // risk of not synchronizing properly, but it should be resolved by
        // calling update() freuently enough (ie. faster than messages are sent)
        // and flushing the RX buffer if not synced. We can do one other check:
        // after the break signal, channel 0 is sent, which should always have a
        // value of zero

        // Read out break signal (hopefully!) and peek at channel 0 (hopefully!)
        if((_dmxSerial->read() != 0) || (_dmxSerial->peek() != 0))
        {
            // If we get here, then we're not synced properly. We can try to
            // flush out all the data and hope the next time is synced
            _dmxSerial->flush(false);
            return false;
        }
        
        // We're probably synced! Read data into buffer
        _dmxSerial->read(_dmxBuffer, _numChannels);

        // Set flag indicating we have new data
        _dataAvailable = true;
        
        // Success
        return true;
    }

    // Something went wrong (most likely haven't received all the bytes yet)
    return false;
}