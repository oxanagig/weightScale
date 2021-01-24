/*
  wiredSensor.h 
  Class to handle sensor board
  Created by Xiang Gao, November 27, 2020.
*/
#ifndef WIREDSENSOR_h
#define WIREDSENSOR_h

#include "Arduino.h"
#include "adc30.h"
#include <Wire.h>
#include "display.h"
#include "gVariables.h"

/*
* This class will interface with sensor board, inlcuding ADC and Serial Flash
*/
class wiredSensor
{
  public:
    wiredSensor(adc30* adc = NULL,gVariables* variables = NULL);
    void begin(void);
    uint16_t getReading(void);
    bool getConnection(void);
    uint16_t getAdcGain(void);
    bool setAdcGain(uint16_t adcGain);
    void initFast(void);
    void zeroFast(void);
    void initUnits(void);
    uint16_t getGain(void);
    uint16_t getSerial(void);
    void resetAdc(void);
    uint16_t readMemory(uint16_t memStore);
    void writeMemory(uint16_t memStore, uint16_t memValue);
    uint16_t getSensorVersion(void);
    uint16_t getCalDate(void);
    uint16_t getCalDue(void);
    void setSystemZero(void);

  private:
    adc30* _adc;
    gVariables* _var;

    uint16_t _valueA; // not used
    uint16_t _valueB; // not used
    uint16_t _sensorVersion;
    uint16_t _calDue;
    uint16_t _calDate;
};

#endif