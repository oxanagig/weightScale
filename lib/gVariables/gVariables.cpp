/*
  gVariables.cpp 
  Class to handle global variables to the application
  Created by Xiang Gao, November 28, 2020.
*/
#include "gVariables.h"
#include "Adafruit_LittleFS.h"

// variables used by the filesystem
lfs_t lfs;
lfs_file_t file;

// // configuration of the filesystem is provided by this struct
// const struct lfs_config cfg = {
//     // block device operations
//     .read  = user_provided_block_device_read,
//     .prog  = user_provided_block_device_prog,
//     .erase = user_provided_block_device_erase,
//     .sync  = user_provided_block_device_sync,

//     // block device configuration
//     .read_size = 16,
//     .prog_size = 16,
//     .block_size = 4096,
//     .block_count = 128,
//     .lookahead = 128,
// };

gVariables::gVariables(void)
{

}

/*
* Initialize the variables and load from flash
*/
void gVariables::begin(void)
{
// Default settings on first application run after flashing firmware
// DATA @5,   DISPLAY_FORCE' Display status store (force)
// DATA @6,   UNIT_N     ' Units status store (lbf)
// DATA @7,   TRUE         ' Auto off store (on)
// DATA @8,   LANG_EN      ' Language store (English)
// data @NUM_USES_STORE, 0 
// data @NUM_USES_STORE+1, 0
// Read DISPLAY_STATUS_STORE, displayStatus
// Read UNITS_STATUS_STORE, units
// Read AUTO_OFF_STORE, isAutoOff
// READ LANGUAGE_STORE, lang 

  // for test purpse
  _displayStatus = DISPLAY_FORCE;
  _units = UNIT_N;
  _isAutoOff = true;
  _lang = LANG_EN;
  _displayUses = 0;
}


enum DISPLAY_STATUS gVariables::getDisplayStatus(void)
{
  return _displayStatus;
}

void gVariables::setDisplayStatus(enum DISPLAY_STATUS displayStatus)
{
  _displayStatus = displayStatus;
}

uint8_t gVariables::getUnits(void)
{
  return _units;
}

void gVariables::setUnits(uint8_t units)
{
  _units = units;
}

bool gVariables::getIsAutoOff(void)
{
  return _isAutoOff;
}

void gVariables::setIsAutoOff(bool isAutoOff)
{
  _isAutoOff = isAutoOff;
}

uint8_t gVariables::getLang(void)
{
  return _lang;
}

void gVariables::setLang(uint8_t lang)
{
  _lang = lang;
}

uint16_t gVariables::getDisplayUses(void)
{
  return _displayUses;
}

void gVariables::setDispalyUses(uint16_t use)
{
  _displayUses = use;
}