/*
  adc30.cpp 
  Library for interface with ADI AD7730 ADC
  Created by Xiang Gao, November 17, 2020.
*/
#include "adc30.h"

/*
* adc30 class constructor
*/
adc30::adc30()
{


}

void adc30::begin(void)
{
    /* Use the default setting for the SPI interface 
    *  note: the configuration will change based on
    *  the hardware you selected.
    */
    SPI.begin();
    SPI.setDataMode(SPI_MODE3);
    SPI.setBitOrder(MSBFIRST); 
    SPI.setClockDivider(SPI_CLOCK_DIV512); // due to the long cable, the speed must be low
}

void adc30::reset()
{
    delay(100);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    SPI.transfer(0xff);
    delay(100);
}
/*
* read the ADC value from the file register
* this function assume that it's using the 
* default 16-bit mode 
*/
uint16_t adc30::getValue()
{
    uint16_t adc_value;
    SPI.transfer(CR_SINGLE_READ|CR_DATA_REGISTER);
    adc_value  = ((uint16_t)SPI.transfer(READ_ONLY))<<8;
    adc_value += (uint16_t)SPI.transfer(READ_ONLY);

    return adc_value;
}

/*
* Set single conversion mode with high reference setting
*/
void adc30::singleConvertMode()
{
    modeWrite();
    SPI.transfer(MR1_MODE_SINGLE);
    SPI.transfer(MR0_HIREF_5V);
}


/**
 * Set continous conversion mode with high reference setting
 */
 void adc30::contConvertMode()
 {
    modeWrite();
    SPI.transfer(MR1_MODE_CONTINUOUS);
    SPI.transfer(MR0_HIREF_5V);
 }


/**
 * Set Enter sync/idle mode
 */
void adc30::idleMode()
{
    modeWrite();
    SPI.transfer(MR1_MODE_IDLE);
    SPI.transfer(MR0_HIREF_5V);
}


/**
 * Set internal full-scale calibration mode
 */
void adc30::internalFullCalib()
{
    modeWrite();
    SPI.transfer(MR1_MODE_INTERNAL_FULL_CALIBRATION);
    SPI.transfer(MR0_HIREF_5V|MR0_RANGE_80MV);
}

/**
 * Set zero-scale  calibration mode
 */
void adc30::internalZeroCalib()
{
    modeWrite();
    SPI.transfer(MR1_MODE_INTERNAL_ZERO_CALIBRATION);
    SPI.transfer(MR0_HIREF_5V);
}

/**
 * System zero-scale calibration mode
 */
void adc30::systemZeroCalib()
{
    modeWrite();
    delay(300);
    SPI.transfer(MR1_MODE_SYSTEM_ZERO_CALIBRATION);
    SPI.transfer(MR0_HIREF_5V);
    delay(300);
}

/**
 *  Write gain value to ADC
 */
void adc30::gainWrite(uint16_t gain)
{
    delay(100);
    SPI.transfer(CR_SINGLE_WRITE|CR_GAIN_REGISTER);
    delay(100);
    SPI.transfer((uint8_t)(gain>>8));
    SPI.transfer((uint8_t)(gain&0xFF));
    SPI.transfer(0);    // don't care the LSB 8bits
    delay(100);
}

/**
 *  Read gain calibration value from ADC
 */
uint16_t adc30::gainRead(void)
{
    uint16_t gain;
    delay(100);
    SPI.transfer(CR_SINGLE_READ|CR_GAIN_REGISTER);
    gain = (uint16_t)SPI.transfer(READ_ONLY)<<8;
    gain += (uint16_t)SPI.transfer(READ_ONLY);
    SPI.transfer(READ_ONLY);// don't care the LSB 8bits
    delay(100);
    return gain;
}

/**
 *  Write offset calibration value to ADC
 */
void adc30::offsetWrite(uint16_t offset)
{
    SPI.transfer(CR_SINGLE_WRITE|CR_OFFSET_REGISTER);
    SPI.transfer((uint8_t)(offset>>8));
    SPI.transfer((uint8_t)(offset&0xFF));
}

/*
* Prepare write to mode reigster
*/
void adc30::modeWrite()
{
    SPI.transfer(CR_SINGLE_WRITE | CR_MODE_REGISTER);
    delay(100);
}



/*
* Check if the data is ready and steady to read
*/
bool adc30::isReadyAndSteady(void)
{
    uint8_t status;
    SPI.transfer(CR_SINGLE_READ | CR_STATUS_REGISTER);
    status = SPI.transfer(READ_ONLY);

    Serial.println((char)(status&0b11000000),BIN);
    // check /RDY bit and /STDY bit
    if((status&0b11000000)==0)
    {
        return true;
    }

    // prevent execcisve read
    delay(5);
    return false;
}