#ifndef __EEPROM_PUT_GET_H
#define __EEPROM_PUT_GET_H

#include <Wire.h>
#define DEVICE 0x50 //EEPROM device ID

template <class T>
int EEPROM_put(int addr, const T& value) {
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  Wire.beginTransmission(DEVICE);
  Wire.write((int)(addr >> 8));    // MSB
  Wire.write((int)(addr & 0xFF));  // LSB
  for (i = 0; i < sizeof(value); i++) Wire.write(*p++);
  Wire.endTransmission();
  return i;
}

template <class T>
int EEPROM_get(int addr, T& value) {
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  Wire.beginTransmission(DEVICE);
  Wire.write((int)(addr >> 8));    // MSB
  Wire.write((int)(addr & 0xFF));  // LSB
  Wire.endTransmission();
  Wire.requestFrom(DEVICE, sizeof(value));
  for (i = 0; i < sizeof(value); i++) {
    if (Wire.available()) *p++ = Wire.read();
  }
  return i;
}

#endif