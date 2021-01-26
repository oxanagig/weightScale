/*
  wiredSensor.cpp 
  Class to handle sensor board
  Created by Xiang Gao, November 27, 2020.
*/

#include "wiredSensor.h"

/*
* wiredSensor constructor
*/
wiredSensor::wiredSensor(adc30* adc,gVariables* variables)
{
    _adc = adc;
    _var = variables;
}

void wiredSensor::begin(void)
{
    Wire.begin();
}

uint16_t wiredSensor::getReading(void)
{
    uint16_t result;
    uint16_t memValue;
    _adc->singleConvertMode();
    result = _adc->getValue();
    return result;
}

bool wiredSensor::getConnection(void)
{
    uint16_t memValue = getSerial();
    if(memValue==0 ||memValue==65535)
        return false;
    
    return true;
}

/*
*Gets the current gain loaded in the ADC (for testing purposes or as a redundancy check)
*/
uint16_t wiredSensor::getAdcGain(void)
{
    return _adc->gainRead();
}
/*
*  Sets the gain on the ADC directly (for testing, debugging, or manual calibration mode)
*  - Returns: (Boolean) Confirmation that the gain was written to the ADC
*/
bool wiredSensor::setAdcGain(uint16_t adcGain)
{
    Serial.println("setGain");
    Serial.println(adcGain);
    _adc->reset();
    _adc->gainWrite(adcGain);    
    _adc->systemZeroCalib();
    while(_adc->isReadyAndSteady());
    _adc->singleConvertMode();
    while(_adc->isReadyAndSteady());
    _valueA = _adc->getValue();
    
    if(adcGain!=getAdcGain())
    {
        return false;
    }

    return true;
}

/*
*Display & Sensor initialization -- read/write memory on connect
*/
void wiredSensor::initFast(void)
{
    uint16_t calMonth,calYear;

    calMonth = readMemory(STORE_CAL_MONTH);
    Serial.print("month:");
    Serial.println(calMonth);
    calYear = readMemory(STORE_CAL_YEAR);
    Serial.print("year:");
    Serial.println(calYear);
    
    _calDue = ((calYear - 1999) * 100) + calMonth;  //cal due algorithm for old sensor
    _calDate = ((calYear - 2000) * 100) + calMonth;// date calibrated

    _sensorVersion = 17; //00017 ' 17g (legacy)
    _var->unitFamily = FAMILY_STANDARD_FORCE;
}

/*
* zero button (quick system zero)
*/
void wiredSensor::zeroFast(void)
{
    // _adc->reset();
    // _adc->gainWrite(getAdcGain());
    // _adc->systemZeroCalib();
    // delay(75); //Used to be 250, changed to 75 to speed up things (1507)s
    // _adc->singleConvertMode();
    // _valueA = _adc->getValue();
    // _adc->singleConvertMode();
    // _valueA = _adc->getValue();
    // _adc->singleConvertMode();
    // _valueA = _adc->getValue();
    setAdcGain(getAdcGain());

    _var->systemZero = false;
    _var->peakValue = 0;
    _valueA = 32768;
    _valueB = 32768;
    _var->isZeroRun = true;    
    delay(25); // Lowed by half (1507)
}


/*
*   Universal zero sensor command -- depending on the sensor connected, it will be set to zero
*     (Typically this command will be coupled with the ZERO MASTER command)
*/
void wiredSensor::initUnits(void)
{
    
    setAdcGain(getGain());

    _var->peakValue = 0;
    _valueA = 32768;
    _valueB = 32768;
    _var->isZeroRun = true;    
    delay(25); // Halfed and removed from before peakvalue (1507)
}

/*
*   Set the gain as found in the memory according to the units we want
*/
uint16_t wiredSensor::getGain(void)
{
    uint16_t gain;
    switch(_var->getUnits())
    {
        case UNIT_MV    :   gain = readMemory(STORE_MV_GAIN);break;
        case UNIT_N     :   gain = readMemory(STORE_N_GAIN);break;
        case UNIT_KGF   :   gain = readMemory(STORE_KGF_GAIN);break;
        case UNIT_KN    :   gain = readMemory(STORE_N_GAIN);break;
        default         :   gain = readMemory(STORE_LBF_GAIN);break;
    }

    delay(100);

    return gain;
}

/*
* Read serial for comparison in initial config packet (after its received)  
*/
uint16_t wiredSensor::getSerial(void)
{
    return readMemory(STORE_SERIAL_NR);
}

/*
* Read the sensor calibration date
*/
uint16_t wiredSensor::getCalDate(void)
{
    return _calDate;
}

/*
* Read the sensor calibration due date
*/

uint16_t wiredSensor::getCalDue(void)
{
    return _calDue;
}

/*
* Put sensor into "mV' mode (reset)
*/
void wiredSensor::resetAdc()
{
    _adc->reset();
}

uint16_t wiredSensor::readMemory(uint16_t memStore)
{
    uint8_t memValue[2];
    memStore+=1000;
    delay(21);
    Wire.beginTransmission(0x50); 
    Wire.write(byte(memStore>>8));            
    Wire.write(byte(memStore&0xFF));           
    Wire.endTransmission();     

    Serial.print("address:");
    Serial.print(memStore);
    Serial.print(" value:");

	Wire.requestFrom(0x50,2);
	Wire.available();
	memValue[0] = Wire.read();
    Wire.available();
	memValue[1] = Wire.read();

    Serial.print(memValue[0],HEX);
    Serial.print(" ");
    Serial.println(memValue[1],HEX);

    delay(52);

    return ((uint16_t)memValue[1]) | ((uint16_t)memValue[0]<<8);
}

uint16_t wiredSensor::getSensorVersion(void)
{
    return _sensorVersion;
}

void wiredSensor::setSystemZero(void)
{
    _var->systemZero = true;
}

void wiredSensor::writeMemory(uint16_t memStore, uint16_t memValue)
{
    memStore+=1000;
    delay(21);
    Wire.beginTransmission(0x50); 
    Wire.write(byte(memStore>>8));            
    Wire.write(byte(memStore&0xFF)); 
    Wire.write(byte(memValue>>8));
    Wire.write(byte(memValue&0xFF));           
    Wire.endTransmission();     
    
    delay(5);
}