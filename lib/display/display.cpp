/*
  display.h 
  Class to handle display
  Created by Xiang Gao, November 22, 2020.
*/

#include "display.h"


/*
* display constructor
*/
display::display(u8g2_display_t*  u8g2,gVariables* varaibles)
{
    _u8g2 = u8g2;
    _var = varaibles;
}

/*
* display setup
*/
void display::begin(void)
{
    _u8g2->begin();
    _u8g2->clear();
    _u8g2->setFont(u8g2_font_helvR24_tr);
    _skipDisplay = false;
    _isRawValue = false;
}


/*
*  Set display Font
*/
void display::setFont(const uint8_t *font)
{
    _u8g2->setFont(font);
}

/*
* display 2 lines of message on the OLED, with specified wait and blink wait
*/
void display::msg(const char* firstLine, const char* secondLine,uint16_t waitTime, uint16_t blinkWait,bool clear)
{
    clearBuffer();
    _u8g2->setCursor(5, 15);
    _u8g2->print(firstLine);
    _u8g2->setCursor(5, 30);
    _u8g2->print(secondLine);
    update();

    delay(waitTime);

    if(clear)
    {
        _u8g2->clear();
    }

    delay(blinkWait);
}

/*
* display text on OLED in first line
*/
void display::msgFirstLine(const char* inputMessage)
{
    _u8g2->setFont(u8g2_font_t0_22b_mf);
    _u8g2->setCursor(5, 20);
    _u8g2->print(inputMessage);
}

/*
* display text on OLED in second line
*/
void display::msgSecondLine(const char* inputMessage)
{
    _u8g2->setFont(u8g2_font_t0_22b_mf);
    _u8g2->setCursor(5, 60);
    _u8g2->print(inputMessage);
}

/*
* display text with specific starting position
*/
void display::msg(const char* inputMessage,uint16_t x, uint16_t y)
{
    _u8g2->setCursor(x, y);
    _u8g2->print(inputMessage);
}

/*
* display text with the current cursor position
*/
void display::msg(const char* inputMessage)
{
    _u8g2->print(inputMessage);
}

/**
 *  send the buffer content to display
 */ 
void display::update(void)
{
    _u8g2->sendBuffer();
}

/*
* clear display buffer
*/
void display::clearBuffer()
{
    _u8g2->clearBuffer();
}

/*
* clear display buffer and current display on OLED
*/
void display::clearDisplay()
{
    _u8g2->clearDisplay();
}

/*
* Set any variables for character positions 12345, 0 = not active
*/
void display::setValueFormat(enum FAMILY family)
{
    _valueUnits = _var->getUnits();
    switch(family)
    {
        // Family ID 00002 -- [999.9 kN] 
        case FAMILY_HIGH_FORCE:
            _valueDec = 4;
            _valueZeroPad = 0;
            _valueRound = 0;
            _valueDivide = 0; // We're assuming that raw values are calibrated differently here...
            _maxValue = 3000;// [300.0 kN] -- bug 17-07: this was not working until now; said "99.9 kN" instead
            _minValue = 3000;// [-300.0kN] -- changed: move over to read 
            _maxMoveOver = 3000; //If same as maxValue, then it doesn't ever "move over"
            _minMoveOver = 999; //when display is less than this value, it removes space and moves over [-100.0kN] = 1000 > 999             
            break;

        // Family ID 00003 -- [99.99 mm]
        case FAMILY_DIMENSIONAL:
            _valueDec = 2;
            _valueZeroPad = 0;
            _valueRound = 0;
            _valueDivide = 0;
            _maxValue = 9999;
            _minValue = 9999;
            _maxMoveOver = 9999;
            _minMoveOver = 999;
            //TODO: can't be negative? or move decimal down?     
            break;
        //RPM is a special case (already set by isRpm$)? ' Family ID 00100
        //CASE FAMILY_RPM
        
        //Family ID 00004 -- [30000 N ]
        case FAMILY_LOW_FORCE:
            _valueDec = 0;
            _valueZeroPad = 0 ;
            _valueRound = 0 ;
            _valueDivide = 0;
            _maxValue = 30000; //[30000 N ]
            _minValue = 30000; //[-30000N ]
            _maxMoveOver = 30000;
            _minMoveOver = 9999;  //[-9999 N ] < [-10000N ]
            break;
        
        default:
            _maxValue = 30000;  //[300000N ]  last zero is fake
            _minValue = 9995;   //[-99950N ]    '  '
            _maxMoveOver = 9990;//[99900 N ] < [ 99950N ] < [650000N ]
            _minMoveOver = 990; // [-9900 N ] < [- 9950N ] < [-99500N ] 
            _valueRound = 0;    //No rounding is performed by the display; mbar is calibrated to 1/10 of actual force readings.
            _valueDec = 0;
            _valueDivide = 0;
            _valueZeroPad = 1;  // The last zero is fake 
            
            //If we're using the original ADC protocol, the value is rounded to nearest 10
            if(_var->getUnits() == UNIT_KN)
            {
                //Units are kN, display as 999.9/-99.9
                _valueDec = 4;    
                _valueZeroPad = 0; 
                _valueRound = 0; 
                _valueDivide = 10; //Bug fix: we needed to actually divide the value by a factor of 10 since it was too big 
                _maxValue = 30000; //Bug fix #2 (17-07): Because we divided, we forgot to change the min/max values!  
                _minValue = 30000;
                _maxMoveOver = 30000;
                _minMoveOver = 9999; 
            }                                                     
            // Units are N, lbf or kgf in standard format -- [99990 N ][-9990 N ]
            else if(_var->getUnits() == UNIT_N )     
            {
                _valueRound = 5; //round to nearest 50 for newtons only
            }  
            else if(_var->getUnits() == UNIT_MV)
            {
                _valueZeroPad = 0;
                _maxValue = 30000;
                _minValue = 9999;  
            }          
            break;
    }
}

/*
* Set display status 
*/
void display::setDisplayStatus(enum DISPLAY_STATUS display_status)
{
    switch(display_status)
    {
        case DISPLAY_FORCE_PEAK:
            //Use valueA as the original value?
            
            //if value (valueA - 32768) is greater than peak value, it becomes the new peak value
            if(_value > _var->peakValue) 
            {
                _var->peakValue = _value;
            }
            _value = _var->peakValue;
            _isRawValue = true;
            _valueDivide = 0; //don't do any dividing here.. we should have already done it previously if needed
            //_valueUnits = UNIT_PK; // don't reuse this variable to print "Peak" in the second line
            break;

        default:
            _skipDisplay = true;
            break;
    }
    
}

/*
*   Displays the sensor values on the LCD according to the value formatting
*
*   Set "value" variable first before running, as well as display line var (1st or second line)
*/
void display::displaySensorValue(int valueDisplayLine)
{
    bool _isNegValue = false;
    bool _isValuePrinted;
    uint8_t _valueDigit;
    uint8_t _valueModulus;
    uint8_t _valueMaxLength;
    uint8_t _valueLength;

    // get the current Unit
    char unitStr[10];
    switch(_var->getLang())
    {
        case LANG_ES:
            switch(_valueUnits)
            {
                case UNIT_MV :  sprintf(unitStr,"mV" );break;
                case UNIT_N :   sprintf(unitStr,"N " );break;
                case UNIT_KN :  sprintf(unitStr,"kN" );break;
                case UNIT_KGF : sprintf(unitStr,"kg-F" );break;
                //case UNIT_PK :  sprintf(unitStr,"Pc" );break;
                default :       sprintf(unitStr,"lb-F" );break;
            }
            break;                               
        case LANG_DE:
            switch(_valueUnits)
            {
                case UNIT_MV :  sprintf(unitStr,"mV");break;
                case UNIT_N :   sprintf(unitStr,"N ");break;
                case UNIT_KN :  sprintf(unitStr,"kN");break;
                case UNIT_KGF : sprintf(unitStr,"kg");break;
                //case UNIT_PK :  sprintf(unitStr,"Sp");break;
                default:        sprintf(unitStr,"Kr");break;
            }
        default:
            switch(_valueUnits)
            {
                case UNIT_MV :  sprintf(unitStr,"mV");break;
                case UNIT_N :   sprintf(unitStr,"N ");break;
                case UNIT_KN :  sprintf(unitStr,"kN");break;
                case UNIT_KGF : sprintf(unitStr,"kg-F");break;
                //case UNIT_PK :  sprintf(unitStr,"Pk");break;
                default:        sprintf(unitStr,"lb-F");break;
            }
    }

    // Select which LCD line to use before running
    if(valueDisplayLine == 0)
    {
        // first line for text
        _u8g2->setCursor(1, 35);
        _u8g2->setFont(u8g2_font_inb30_mf);
    }
    else
    {
        // second line for text
        _u8g2->setFont(u8g2_font_helvB12_tr); 
        _u8g2->setCursor(3, 55);
    }   

    if(_skipDisplay == true)
    { 
        _skipDisplay = false;
        if(_valueUnits==UNIT_N)
        {
            _u8g2->setCursor(LCDWidth - 15, 55);
        }
        else if(_valueUnits==UNIT_KN)
        {
            _u8g2->setCursor(LCDWidth - 25, 55);
        }
        else
        {
            _u8g2->setCursor(LCDWidth - 35, 55);
        }
        _u8g2->print(unitStr);
        return;
    }         
    
    // Raw values (such as RPM) do not come from the ADC, and also cannot be negative(?)
    if(_isRawValue != true)
    {
       // Get value from sensor bar data and always take out 32768
        if(_value < 32768)
        {          
            _value = 32768 - _value;    //' -
            _isNegValue = true;
            if(_value > _minValue)
            {
                _value = _minValue; 
            }
        }      
        else
        {
            _value = _value - 32768;    //' +
            _isNegValue = false;
            if(_value > _maxValue)
            { 
                _value = _maxValue;
            }
        }
    }
    else
    {
        // Reset back to default
        _isRawValue = false;
    }
  
    if (_valueDivide > 0)
    {//Raw value to be divided (typically only kN and high force where the 10th place is insignificant)
        _value = _value / _valueDivide ;
    }
  
    //Rounding factor
    if(_valueRound > 0)
    {
        _valueModulus = _value % _valueRound; // 1 (no round anyway)/10/100/1000
        _value = _value / _valueRound; // Trim value
        
        // Use remainder (from modulus) to determine rounding factor
        if (_valueModulus > (_valueRound * 5) )
        {
            _value = _value + 1;
        }
        
        //Put value back together as the same value length
        _value = _value * _valueRound;
        
    }

    //Value is never negative if it is rounded down to zero
    if(_value < 1)
    {
        _isNegValue = false;
    } 
    
    // Nothing has been printed yet
    if ((_value > _maxMoveOver && _isNegValue == false) || (_value > _minMoveOver && _isNegValue))
    {
        _valueMaxLength = 6; //may need to print UP TO 6 digits, but shift over anyway. 
        _valueLength = 6;
    }
    else
    {
        _valueMaxLength = 5; //a constant during the iteration through digits
        _valueLength = 5; // default; there is a space after the number (before the unit)
    }
    _isValuePrinted = false; 

    //If the value is zero, no decimals are used, just use 1 fake zero
    if(_valueDec == 0 && _value == 0)
    { 
        _valueZeroPadPos = 1;
    }
    else
    {
       _valueZeroPadPos = _valueZeroPad;
    }
    
    // Remove spaces if zero padding set
    if( _valueZeroPadPos > 0 )
    {
        _valueLength = _valueLength - _valueZeroPadPos;
    }
    
    //If a decimal is in the display value, reduce the value length
    if( _valueDec > 0 )
    {
        _valueLength = _valueLength - 1; //TODO: since this is across the board, this should be a temp. constant in the set() function!
    }

    //Do 5 iterations for each character
    for(uint8_t i=1;i<=_valueMaxLength;i++) //= max length of characters for value 
    {   
        //If the value is negative, print a negative sign, then disable negative value marker
        if(_isNegValue == true)
        {
            _u8g2->print("-");
            _isNegValue = false;
            _valueLength = _valueLength - 1;
            continue;
        }
        
        
        // Print a fake zero and skip IF zp is GREATER than current digit position
        if(_valueZeroPadPos > (_valueMaxLength - i))
        {
            _u8g2->print("0");
            _valueLength = _valueLength - 1;
            continue;
        }
        
        //Get the first digit; DIG is reversed (43210) <-- the 5 digits positions
        _valueDigit = getDigit(_value, _valueLength - 1);

        // If the value is empty so far...print a space, or a 0 if there is a decimal in the next position
        if(_isValuePrinted == false && _valueDigit == 0 )
        {
            if(_valueDec == (1 + i))
            {
                _u8g2->print("0");
                _isValuePrinted = true;
            }
            else
            {
                if(valueDisplayLine == 0)
                    _u8g2->print(" ");
            }                          
            _valueLength = _valueLength - 1;
        }
        else// Value is not zero, stop printing spaces  
        {
            //If the decimal is in the current display position 
            //TODO: should we move this further up the loop??? 
            if(_valueDec == i) 
            {
                _u8g2->print(".");
            }
            else
            {
                char buffer[2];
                sprintf(buffer,"%d",_valueDigit);
                _u8g2->print(buffer);
                _valueLength = _valueLength - 1;
            }
            
            _isValuePrinted = true;                     
        }
        
    }
   
    //Determine whether to show a space before the unit
    if (_valueMaxLength < 6)
    {
        _u8g2->print(" ");
    }

    if(valueDisplayLine==1)
    {
        _u8g2->print(" Peak");
        if(_valueUnits==UNIT_N)
        {
            _u8g2->setCursor(LCDWidth - 15, 55);
        }
        else if(_valueUnits==UNIT_KN)
        {
            _u8g2->setCursor(LCDWidth - 25, 55);
        }
        else
        {
            _u8g2->setCursor(LCDWidth - 35, 55);
        }
        _u8g2->print(unitStr);
    }
}


uint8_t display::getDigit(uint16_t number, uint8_t position)
{
    uint8_t digit;

    switch(position)
    {
        case 0: digit = number%10;      break;
        case 1: digit = (number/10)%10; break;
        case 2: digit = (number/100)%10;break;
        case 3: digit = (number/1000)%10;break;
        case 4: digit = (number/10000)%10;break;
        case 5: digit = (number/100000)%10;break;
        default: digit = 0;break;
    }

    return digit;
}

void display::setSensorValue(uint16_t value)
{
    _value = value;
}

void display::setHighligthedFont(void)
{
    _u8g2->setFontMode(0);
    _u8g2->setDrawColor(0);
}

void display::setnormalFont(void)
{
    _u8g2->setFontMode(0);
    _u8g2->setDrawColor(1);
}

void display::drawBox(uint8_t x, uint8_t y, uint8_t w, uint8_t h)
{
    _u8g2->drawBox(x,y,w,h);
}

// void display::msgRightJustified(const char* input, uint32_t length,uint8_t x, uint8_t y,uint8_t fontWidth)
// {
//     // uint32_t i;
//     // _u8g2->setCursor(x,y);

//     // for(i=0;i<length;i++)
//     // {

//     // }
// }