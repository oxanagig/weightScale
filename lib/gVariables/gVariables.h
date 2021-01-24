/*
  gVariables.h 
  Class to handle global variables to the application
  Created by Xiang Gao, November 28, 2020.
*/
#ifndef NVM_h
#define NVM_h

#include "Arduino.h"

#define DISPLAY_VERSION_YEAR  21
#define DISPLAY_VERSION_MONTH  1


#define CCC_TOGGLE_SENSOR       1   // Turns sensor value output on
#define CCC_INIT_UNITS          2
#define CCC_ADC_RESET           10   // Resets ADC and writes configuration register
#define CCC_SYS_ZERO_CAL        11
#define CCC_INT_ZERO_CAL        12   // Not used -- use system zero scale cal instead 
#define CCC_FULLSCALE_CAL       13
#define CCC_SET_FULLSCALE       16   // Not used; values read from the sensor memory are primarily used
#define CCC_GET_FULLSCALE       17
#define CCC_SET_OFFSET          18
#define CCC_GET_OFFSET          19
#define CCC_SET_UNITS           22
#define CCC_GET_READING         23   // Doesn't serve a purpose; sensor is _always_ spewing adc values
#define CCC_READ_MEM            26                                                                   
#define CCC_WRITE_MEM           27
//CCC_CONFIG_RN           con 00032
#define CCC_DISPLAY_SLEEP       33

#define STORE_NUM_SENSORS       0   // Only on newer sensors
#define STORE_SENSOR_INFO       2   // Only on newer sensors
#define STORE_IS_RPM            4   // Overriding function for enabling RPM family (no need to look for "multiple sensors")
#define STORE_USE_LIMIT         6   // 65535 OR 0 == unlimited, anything else == number of times it can be turned on (demo mode)
#define STORE_NUM_USES          8   // Number of times this sensor has been connected to (if in demo mode, match with use limit)
#define STORE_MINUTE_LIFE       10   // Slow counter that measures how many minutes of active use 
#define STORE_CAL_DUE           12   // STORE_CAL_DUE_MONTH     CON 00012   ' Only on newer sensors
#define STORE_CAL_DATE		    14   // STORE_CAL_DUE_YEAR      CON 00014   ' Only on newer sensors
#define STORE_AUTO_OFF_TIME     16   // Automatically turns off after the predefined time interval
#define STORE_ON_DISPLAY_SLEEP  18   // If set, shutdowns after N ms if the display is shut down 
#define STORE_SERIAL_NR         24   // 1024-1025 Serial Number - same as v.1.7f (meas. bar)
#define STORE_KN_GAIN           26   // Units 2 (new sensors only) [ 1026-1027 Offset - not used - same as v.1.7f (meas. bar) ]
#define STORE_ADC_GAIN          28   // [ 1028-1029 ADC Gain - used with old software version 1.7f (meas. bar) ]
#define STORE_KGF_GAIN          34   // Units 4 1034-1035 kgf gain / scale
#define STORE_N_GAIN            36   // Units 1 1036-1037 N gain / scale - new to version 2.0 (meas. bar) 
#define STORE_LBF_GAIN          38   // Units 3 1038-1039 lbf gain / scale - new to version 2.0 (meas. bar) 
#define STORE_MV_GAIN           40   // 1040-1041 calc./factory ADC gain / scale - same as software v. 1.7f (meas. bar)     
#define STORE_CAL_MONTH         42   // Legacy v1.7g cal month 
#define STORE_CAL_YEAR          44   // Legacy v1.7g cal year                                                               
#define STORE_SENSOR_TYPE       48   // Stores the the ID from sensor types -- this also determines the family the sensor has (new include file?)
#define STORE_SENSOR_FIRMWARE   50   // Stores the current sensor firmeware (hard programmed on sensor) 
#define STORE_DISPLAY_LAST      52   // Stores the last display version it was connected to 
#define STORE_DISPLAY_USES      54   // Shows the number of times the last display has been used
#define STORE_MV_OFFSET         56   // Universal sensor/adc offset (zero point) -- now used on both legacy and new sensors (zero button writes to it now)
#define STORE_WAKE_INTERVAL     58   // If set, sensor wakes up ever N ms as defined (multiplier)
#define STORE_WAKE_OVER_MV      60   // mV difference/threshold to check for -- holds for 200 ms, then turns on if wake interval is set




enum FAMILY
{
    FAMILY_STANDARD_FORCE = 1,
    FAMILY_HIGH_FORCE = 2,
    FAMILY_DIMENSIONAL = 3,
    FAMILY_LOW_FORCE = 4,
    FAMILY_RPM = 10
};

enum UNIT
{
    UNIT_LBF = 0,
    UNIT_N = 1,
    UNIT_KGF = 2,
    UNIT_KN =3,
    UNIT_MM= 5,
    UNIT_PK = 6,
    UNIT_RPM = 8,
    UNIT_MV = 99
};

enum DISPLAY_STATUS
{
    DISPLAY_FORCE   = 0,
    DISPLAY_FORCE_PEAK = 1,
    DISPLAY_FORCE_RPM = 2,
    DISPLAY_FORCE_SN =3
};

enum LANG
{
    LANG_EN = 0,
    LANG_DE = 1,
    LANG_ES = 2
};



class gVariables
{
    public:
        gVariables(void);
        void begin(void);
        // the variables that stored in flash will be accessed 
        // through getter and setter function
        uint8_t getDisplayStatus(void);
        void setDisplayStatus(uint8_t displayStatus);
        
        uint8_t getUnits(void);
        void setUnits(uint8_t units);
        
        bool getIsAutoOff(void);
        void setIsAutoOff(bool isAutoOff);
        
        uint8_t getLang(void);
        void setLang(uint8_t lang);

        uint16_t getDisplayUses(void);
        void setDispalyUses(uint16_t use);

        uint8_t unitFamily;
        bool systemZero;
        bool isZeroRun;
        bool isRawMode;
        bool isConnected;
        bool sleepMode;
        uint16_t autoOffCount;
        uint16_t peakValue;
        uint16_t lcdBlinkWait;
        uint16_t lcdWait;
        uint16_t autoOffMax;
        uint16_t value;
        


    private:
        uint8_t _displayStatus;
        bool _isAutoOff;
        uint8_t _lang;
        uint8_t _displayUses;
        uint8_t _units;
};
#endif