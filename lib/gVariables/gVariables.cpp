/*
  gVariables.cpp 
  Class to handle global variables to the application
  Created by Xiang Gao, November 28, 2020.
*/
#include "gVariables.h"
#include <Adafruit_SPIFlash.h>

Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash onboardFlash(&flashTransport);

gVariables::gVariables(void)
{

}

/*
* Initialize the variables and load from flash
*/
void gVariables::begin(void)
{
  Serial.print("Starting up onboard QSPI Flash...");
  onboardFlash.begin();
  Serial.println("Done");
  Serial.println("Onboard Flash information");
  Serial.print("JEDEC ID: 0x");
  Serial.println(onboardFlash.getJEDECID(), HEX);
  Serial.print("Flash size: ");
  Serial.print(onboardFlash.size() / 1024);
  Serial.println(" KB");

  _pagesize = onboardFlash.pageSize();
  _readFlash();
  
  if(_buffer[INITIALIZED_FLAG]!=0x55)
  {
    Serial.println("initializing flash");
    memset(_buffer,0,_pagesize);

    _buffer[INITIALIZED_FLAG] = 0x55;
    _buffer[DISPLAY_STATUS_STORE]=DISPLAY_FORCE;
    _buffer[UNITS_STATUS_STORE]=UNIT_N;
    _buffer[AUTO_OFF_STORE]=1;
    _buffer[LANGUAGE_STORE]=LANG_EN;
    _buffer[NUM_USES_STORE]=0;
    _buffer[NUM_USES_STORE+1]=0;

    _writeFlash();
  }
  else
  {
    Serial.println("flash initialized");
  }

  Serial.print("selected Unit:");
  Serial.println(_buffer[UNITS_STATUS_STORE]);
  Serial.print("display status:");
  Serial.println(_buffer[DISPLAY_STATUS_STORE]);
  Serial.print("auto off status:");
  Serial.println(_buffer[AUTO_OFF_STORE]);
  Serial.print("flash flag:");
  Serial.println(_buffer[INITIALIZED_FLAG]);

  Serial.print("number of use:");
  Serial.println((uint16_t)_buffer[NUM_USES_STORE]+((uint16_t)_buffer[NUM_USES_STORE+1]<<8));
  _displayStatus = (enum DISPLAY_STATUS)_buffer[DISPLAY_STATUS_STORE];
  _units = _buffer[UNITS_STATUS_STORE];
  _isAutoOff = _buffer[AUTO_OFF_STORE];
  _lang = _buffer[LANGUAGE_STORE];
  _displayUses = (uint16_t)_buffer[NUM_USES_STORE]+((uint16_t)_buffer[NUM_USES_STORE+1]<<8);

}


enum DISPLAY_STATUS gVariables::getDisplayStatus(void)
{
  return _displayStatus;
}

void gVariables::setDisplayStatus(enum DISPLAY_STATUS displayStatus)
{
  _displayStatus = displayStatus;
  _buffer[DISPLAY_STATUS_STORE] = _displayStatus;
  _writeFlash();
  _readFlash(); // read back the flash to updat the cache buffer
}

uint8_t gVariables::getUnits(void)
{
  return _units;
}

void gVariables::setUnits(uint8_t units)
{
  _units = units;
  _buffer[UNITS_STATUS_STORE] = _units;
  _writeFlash();
  _readFlash(); // read back the flash to updat the cache buffer
}

bool gVariables::getIsAutoOff(void)
{
  return _isAutoOff;
}

void gVariables::setIsAutoOff(bool isAutoOff)
{
  _isAutoOff = isAutoOff;
  _buffer[AUTO_OFF_STORE] = _isAutoOff;
  _writeFlash();
  _readFlash(); // read back the flash to updat the cache buffer
}

uint8_t gVariables::getLang(void)
{
  return _lang;
}

void gVariables::setLang(uint8_t lang)
{
  _lang = lang;
  _buffer[LANGUAGE_STORE] = _lang;
  _writeFlash();
  _readFlash(); // read back the flash to updat the cache buffer
}

uint16_t gVariables::getDisplayUses(void)
{
  return _displayUses;
}

void gVariables::setDispalyUses(uint16_t use)
{
  _displayUses = use;
  _buffer[NUM_USES_STORE] = (uint8_t)(_displayUses&0xFF);
  _buffer[NUM_USES_STORE+1] = (uint8_t)(_displayUses>>8);
  _writeFlash();
  _readFlash(); // read back the flash to updat the cache buffer
}

void gVariables::_readFlash(void)
{
  onboardFlash.readBuffer(0, _buffer, _pagesize);
}

void gVariables::_writeFlash(void)
{
  onboardFlash.eraseSector(0);
  onboardFlash.writeBuffer(0,_buffer,_pagesize);
}