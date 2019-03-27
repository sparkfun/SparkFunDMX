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

#include <inttypes.h>


#ifndef SparkFunDMX_h
#define SparkFunDMX_h

// ---- Methods ----

class SparkFunDMX {
public:
  void initRead(int maxChan);
  void initWrite(int maxChan);
  uint8_t read(int Channel);
  void write(int channel, uint8_t value);
  void update();
private:
  bool _READ = true;
  bool _WRITE = false;
  bool _READWRITE;
  static void startCode();
};

#endif