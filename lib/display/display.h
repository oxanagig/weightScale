/*
  display.h 
  Class to handle display
  Created by Xiang Gao, November 22, 2020.
*/
#ifndef DISPLAY_h
#define DISPLAY_h

#include "Arduino.h"
#include <Wire.h>
#include <U8g2lib.h>
#include "gVariables.h"

#define u8g2_display_t U8G2_SSD1309_128X64_NONAME2_F_HW_I2C 

/*
* This class will interface U8g2 library.
*/

class display
{
  public:
    display(u8g2_display_t*  u8g2 = NULL, gVariables* variables = NULL);
    void begin(void);
    void setFont(const uint8_t *font);
    void msgFirstLine(const char* inputMessage);
    void msgSecondLine(const char* inputMessage);
    void msg(const char* firstLine, const char*secondLine,uint16_t holdTime, uint16_t blinkWait,bool clear);
    void msg(const char* inputMessage,uint16_t x, uint16_t y);
    void msg(const char* inputMessage);
    void setValueFormat(enum FAMILY family);
    void setDisplayStatus(enum DISPLAY_STATUS display_status);
    void displaySensorValue(int valueDisplayLine);
    void update(void);
    void clearBuffer(void);
    void clearDisplay(void);
    void setSensorValue(uint16_t value);
    void setHighligthedFont(void);
    void setnormalFont(void);
    void drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h);
    
  private:
    u8g2_display_t* _u8g2;
    gVariables* _var;

    bool _isRawValue;
    bool _skipDisplay;
    uint8_t _valueUnits;
    uint8_t _valueDec;
    uint8_t _valueZeroPad;
    uint8_t _valueZeroPadPos;
    uint16_t _value;
    uint16_t _valueDivide;
    uint16_t _valueRound;
    uint16_t _maxValue;
    uint16_t _minValue;
    uint16_t _maxMoveOver;
    uint16_t _minMoveOver;

    uint8_t getDigit(uint16_t number, uint8_t position);
};

#endif