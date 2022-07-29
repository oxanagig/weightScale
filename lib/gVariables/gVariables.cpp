/*
  gVariables.cpp 
  Class to handle global variables to the application
  Created by Xiang Gao, November 28, 2020.
*/
#include "gVariables.h"


gVariables::gVariables(void)
{
}

/*
* Initialize the variables and load from flash
*/
void gVariables::begin(void)
{
  if ( EEPROM.read(INITIALIZED_FLAG) != 0x55)
  {
    for (int i = 0 ; i < EEPROM.length() ; i++) 
    {
      EEPROM.write(i, 0);
    }

    EEPROM.write(INITIALIZED_FLAG, 0x55);
    EEPROM.write(DISPLAY_STATUS_STORE, DISPLAY_FORCE);
    EEPROM.write(AUTO_OFF_STORE, 1);
    EEPROM.write(UNITS_STATUS_STORE,UNIT_N);
    EEPROM.write(LANGUAGE_STORE, LANG_EN);
    EEPROM.write(NUM_USES_STORE,0);
    EEPROM.write(NUM_USES_STORE+1,0);

  }
  else
  {
    // Do nothing because it is intialized
  }

  // Serial.print("selected Unit:");
  // Serial.println(_buffer[UNITS_STATUS_STORE]);
  // Serial.print("display status:");
  // Serial.println(_buffer[DISPLAY_STATUS_STORE]);
  // Serial.print("auto off status:");
  // Serial.println(_buffer[AUTO_OFF_STORE]);
  // Serial.print("flash flag:");
  // Serial.println(_buffer[INITIALIZED_FLAG]);

  // Serial.print("number of use:");
  // Serial.println((uint16_t)_buffer[NUM_USES_STORE]+((uint16_t)_buffer[NUM_USES_STORE+1]<<8));
  _displayStatus = (enum DISPLAY_STATUS)EEPROM.read(DISPLAY_STATUS_STORE);
  _units = EEPROM.read(UNITS_STATUS_STORE);
  _isAutoOff = EEPROM.read(AUTO_OFF_STORE);
  _lang = EEPROM.read(LANGUAGE_STORE);
  _displayUses = (uint16_t)EEPROM.read(NUM_USES_STORE) + ((uint16_t)EEPROM.read(NUM_USES_STORE + 1) << 8);
}

enum DISPLAY_STATUS gVariables::getDisplayStatus(void)
{
  return _displayStatus;
}

void gVariables::setDisplayStatus(enum DISPLAY_STATUS displayStatus)
{
  _displayStatus = displayStatus;
  EEPROM.write(DISPLAY_STATUS_STORE, _displayStatus);

}

uint8_t gVariables::getUnits(void)
{
  return _units;
}

void gVariables::setUnits(uint8_t units)
{
  _units = units;
  EEPROM.write(UNITS_STATUS_STORE,_units);
}

bool gVariables::getIsAutoOff(void)
{
  return _isAutoOff;
}

void gVariables::setIsAutoOff(bool isAutoOff)
{
  _isAutoOff = isAutoOff;
  EEPROM.write(AUTO_OFF_STORE,_isAutoOff);
  
}

uint8_t gVariables::getLang(void)
{
  return _lang;
}

void gVariables::setLang(uint8_t lang)
{
  _lang = lang;
  EEPROM.write(LANGUAGE_STORE,_lang);
 
}

uint16_t gVariables::getDisplayUses(void)
{
  return _displayUses;
}

void gVariables::setDispalyUses(uint16_t use)
{
  _displayUses = use;
  EEPROM.write(NUM_USES_STORE,(uint8_t)(_displayUses & 0xFF));
  EEPROM.write(NUM_USES_STORE + 1,(uint8_t)(_displayUses >> 8));
}

void gVariables::_readFlash(void)
{

}

void gVariables::_writeFlash(void)
{
 
}