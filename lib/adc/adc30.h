/*
  adc30.h 
  Library for interface with ADI AD7730 ADC
  Created by Xiang Gao, November 17, 2020.
*/
#ifndef ADC30_h
#define ADC30_h

#include "Arduino.h"
#include <SPI.h>

#define READ_ONLY 0xFF
//Communication Register Values
#define CR_SINGLE_WRITE 0x00
#define CR_SINGLE_READ 0x10
#define CR_CONTINUOUS_READ_START 0x20
#define CR_CONTINUOUS_READ_STOP 0x30

#define CR_COMMUNICATION_REGISTER 0x00 //Write only
#define CR_STATUS_REGISTER 0x00        //Read only
#define CR_DATA_REGISTER 0x01
#define CR_MODE_REGISTER 0x02
#define CR_FILTER_REGISTER 0x03
#define CR_DAC_REGISTER 0x04
#define CR_OFFSET_REGISTER 0x05
#define CR_GAIN_REGISTER 0x06
#define CR_TEST_REGISTER 0x07

//Mode Register Values
#define MR1_MODE_IDLE 0x00
#define MR1_MODE_CONTINUOUS 0x20 //Standard Operation
#define MR1_MODE_SINGLE 0x40
#define MR1_MODE_STANDBY 0x60
#define MR1_MODE_INTERNAL_ZERO_CALIBRATION 0x80
#define MR1_MODE_INTERNAL_FULL_CALIBRATION 0xA0
#define MR1_MODE_SYSTEM_ZERO_CALIBRATION 0xC0
#define MR1_MODE_SYSTEM_FULL_CALIBRATION 0xE0
#define MR1_BU_BIPOLAR 0x00  //+- voltage defined by MR0_RANGE
#define MR1_BU_UNIPOLAR 0x10 //0 to voltage deifined by MRO_RANGE
#define MR1_WL_24_BIT 0x01
#define MR1_WL_16_BIT 0x00

#define MR0_HIREF_5V 0x80
#define MR0_HIREF_2P5V 0x00
#define MR0_RANGE_10MV 0x00
#define MR0_RANGE_20MV 0x01
#define MR0_RANGE_40MV 0x02
#define MR0_RANGE_80MV 0x03
#define MR0_CHANNEL_1 0x00
#define MR0_CHANNEL_2 0x01
#define MR0_CHANNEL_SHORT_1 0x02      //Used for internal noise check
#define MR0_CHANNEL_NEGATIVE_1_2 0x03 //Unknown use
#define MRO_BURNOUT_ON 0x04           //Advanced, to check if loadcell is burnt out

//Filter Register Values
#define FR2_SINC_AVERAGING_2048 0x80 //Base sample rate of 50 Hz
#define FR2_SINC_AVERAGING_1024 0x40 //Base sample rate of 100 Hz
#define FR2_SINC_AVERAGING_512 0x20  //Base sample rate of 200 Hz
#define FR2_SINC_AVERAGING_256 0x10  //Base sample rate of 400 Hz

#define FR1_SKIP_ON 0x02 //the FIR filter on the part is bypassed
#define FR1_SKIP_OFF 0x00
#define FR1_FAST_ON 0x01 //FIR is replaced with moving average on large step, sinc filter averages are used to compensate
#define FR1_FAST_OFF 0x00

#define FR0_CHOP_ON 0x10  //When the chop mode is enabled, the part is effectively chopped at its input and output to remove all offset and offset drift errors on the part.
#define FR0_CHOP_OFF 0x00 //Increases sample rate by x3

//DAC Register Values
#define DACR_OFFSET_SIGN_POSITIVE 0x00
#define DACR_OFFSET_SIGN_NEGATIVE 0x20
#define DACR_OFFSET_40MV 0x10
#define DACR_OFFSET_20MV 0x08
#define DACR_OFFSET_10MV 0x04
#define DACR_OFFSET_5MV 0x02
#define DACR_OFFSET_2P5MV 0x01
#define DACR_OFFSET_NONE 0x00

class adc30
{
public:
  adc30();
  void reset();
  void begin(void);
  void idleMode();
  void singleConvertMode();
  void contConvertMode();
  void standbyMode();
  void internalFullCalib();
  void internalZeroCalib();
  void systemFullCalib();
  void systemZeroCalib();
  uint16_t gainRead();
  void gainWrite(uint16_t gain);
  void offsetWrite(uint16_t offset);
  uint16_t getValue();
  bool isReadyAndSteady(void);

private:
  void modeWrite();
};

#endif