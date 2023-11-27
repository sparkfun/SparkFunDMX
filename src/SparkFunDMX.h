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

#ifndef SparkFunDMX_h
#define SparkFunDMX_h

#include <Arduino.h>
#include <HardwareSerial.h>

// DMX supports up to 512 channels, plus 1 for channel 0
#define DMX_MAX_CHANNELS 513

// DMX messages are started with a break signal of at least 88us
#define DMX_BREAK_DURATION_MICROS 88

// DMX operates at 250kbps with 2 parity bits
#define DMX_BAUD 250000
#define DMX_FORMAT SERIAL_8N2

// Macros for read or write direcion, DMX can't send and transmit at same time
#define DMX_WRITE_DIR 0
#define DMX_READ_DIR 1

class SparkFunDMX
{
    public:
        /// @brief Begins DMX class
        /// @param port Serial port for communication. This library will not
        /// begin the serial port, that must be done before calling dmx.begin()
        /// @param enPin Enable pin connected to bridge chip, used for direction
        /// @param numChannels Number of DMX channels, 512 max
        void begin(HardwareSerial& port, uint8_t enPin, uint16_t numChannels);
        
        /// @brief Set communication direction, either read or write
        /// @param comDir Either DMX_WRITE_DIR or DMX_READ_DIR
        void setComDir(bool comDir);
        
        /// @brief Copy data from a provided byte buffer
        /// @param data Buffer with data to be sent
        /// @param numBytes Number of bytes to copy from buffer
        /// @param startChannel Channel to start copying data to
        void writeBytes(uint8_t* data, uint16_t numBytes, uint16_t startChannel = 1);
        
        /// @brief Copy a single byte to a specified channel
        /// @param data Byte to copy
        /// @param channel Channel to copy data to
        void writeByte(uint8_t data, uint16_t channel);
        
        /// @brief Copy data to a provided byte buffer
        /// @param data Buffer with data to be read
        /// @param numBytes Number of bytes to copy to buffer
        /// @param startChannel Channel to start copying data from
        void readBytes(uint8_t* data, uint16_t numBytes, uint16_t startChannel = 1);
        
        /// @brief Copy a single byte from a specified channel
        /// @param channel Channel to copy data from
        /// @return Byte from specified channel
        uint8_t readByte(uint16_t channel);
        
        /// @brief When reading, returns whether data has been received
        /// @return True if data is available, else false
        bool dataAvailable();
        
        /// @brief When in read mode, will check to see if new data is available
        /// and ready to be read. When in write mode, will actually send out data
        /// @return True if successful, else false
        bool update();

    private:
        // Member variables
        HardwareSerial* _dmxSerial;
        uint8_t _dmxBuffer[DMX_MAX_CHANNELS];
        uint16_t _numChannels;
        uint8_t _enPin;
        bool _comDir;
        bool _dataAvailable;
};

#endif