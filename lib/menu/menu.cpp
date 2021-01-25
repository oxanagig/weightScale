/*
  menu.h 
  Library for menu system
  Created by Xiang Gao, November 27, 2020.
*/

#include "menu.h"


/**
 * menu constructor
 */
 menu::menu(display* Display, button* Button,wiredSensor* sensor,gVariables* Variables)
 {
     _display = Display;
     _button = Button;
     _sensor = sensor;
     _var = Variables;
     _processMenu = NO_ACTION;
 }


 /*
 * Enters a loop, waiting for option selection (interrupts should be off)
 */ 

void menu::optionSelect(void)
{
    if(_menuExit) 
    {
        _option[2] = NULL;
        _option[3] = NULL;
        return;
    }

    while(1)
    {
        displayCurrentFunction();

        // Allow turning the power off
        if (_button->isPressed(BTN_ONOFF))
        {
            while(_button->isPressed(BTN_ONOFF)); // waits for release  
            //TODO:
            //goto displaySleep    ;  
            //break;      
        }

        //Allow exiting the menu entirely (return to main)
        if (_button->isPressed(BTN_ZERO))
        {
            while(_button->isPressed(BTN_ZERO)); // waits for release
            _menuExit = true;
            break; // save this selection, will skip to end
        }
        
        // Go to next option selection
        if (_button->isPressed(BTN_FUNC))
        {
            while(_button->isPressed(BTN_FUNC)); // waits for release
            break;//save this selection, go to next option
        }

        if (_button->isPressed(BTN_MODE))
        {
            while(_button->isPressed(BTN_MODE)); // waits for release
            _optionSelected = _optionSelected + 1 ;
            
            if(_currentOption!=1)
            {
                if ((_optionSelected == 2) && (_option[2] == NULL)) 
                {
                    _optionSelected = 0;
                }
                
                if ((_optionSelected == 3) && (_option[3] == NULL))
                {
                    _optionSelected = 0;
                }
            }
            else
            {
                if (_optionSelected == 4)
                {
                    _optionSelected = 0;
                }
            }
        }
    }
}


void menu::displayCurrentFunction(void)
{
    char secret[28];
    char peakForce[28];
    char units[28];
    char autoOff[28];

    _display->setFont(u8g2_font_t0_12b_mf);
    _display->clearBuffer();

    // secret menu
    if(_currentOption == 3)
    {
        _optionTitle = SM_TITILE;
        _option[0] = SM_OP0;
        _option[1] = SM_OP1;
        _option[2] = NULL;
        _option[3] = NULL;

        _display->setHighligthedFont();
        _display->msg(_optionTitle,5,20);
        _display->setnormalFont();
        sprintf(secret," %s",_option[_optionSelected]);
        _display->msg(secret);
        _display->update();
        return;
    }

    
    
    // Dispaly Status
    _optionTitle = DS_TITLE;
    _option[0] = DS_OP0;
    _option[1] = DS_OP1;
    _option[2] = NULL;
    _option[3] = NULL;

    if(_currentOption == 0)
    {
        _display->setHighligthedFont();
        _display->msg(_optionTitle,5,20);
        _display->setnormalFont();
        sprintf(peakForce,": %s",_option[_optionSelected]);
        _display->msg(peakForce);
    }
    else
    {
        sprintf(peakForce,"%s: %s",_optionTitle,_option[(uint8_t)_var->getDisplayStatus()]);
        _display->msg(peakForce,5,20);
    }
    

    //Unit Select
    _optionTitle = U_TITLE;
    _option[0] = U_OP0;
    _option[2] = U_OP2;
 
    if(_var->unitFamily!=FAMILY_LOW_FORCE)
    {
        _option[3] = KNEWTON;
    }
    _option[1] = NEWTON;

    if(_currentOption == 1)
    {
        _display->setHighligthedFont();
        _display->msg(_optionTitle,5,35);
        _display->setnormalFont();
        sprintf(units,": %s",_option[_optionSelected]);   
        _display->msg(units);
    }
    else
    {
        sprintf(units,"%s: %s",_optionTitle,_option[(uint8_t)_var->getUnits()]);
        _display->msg(units,5,35);
    }
    

    //autoOff select
    _optionTitle = AO_TITLE;
    _option[0] = AO_OP0;
    _option[1] = AO_OP1;
    _option[2] = NULL;
    _option[3] = NULL;

    if(_currentOption == 2)
    {
        _display->setHighligthedFont();
        _display->msg(_optionTitle,5,50);
        _display->setnormalFont();
        sprintf(autoOff,": %s",_option[_optionSelected]); 
        _display->msg(autoOff);
    }
    else
    {
        sprintf(autoOff,"%s: %s",_optionTitle,_option[(uint8_t)_var->getIsAutoOff()]); 
        _display->msg(autoOff,5,50);
    }
    _display->update();
}

void menu::processFunctionMenu(void)
{
    _currentOption = 0;
    _optionSelected = (uint8_t)_var->getDisplayStatus();
    optionSelect();

    //write only if a change was made
    if(_optionSelected!=(uint8_t)_var->getDisplayStatus())
    {
        _var->setDisplayStatus((enum DISPLAY_STATUS)_optionSelected);
    }

    _currentOption = 1;
    _optionSelected = _var->getUnits();
    optionSelect();

    //Immediately setup sensor
    if(_optionSelected !=_var->getUnits())
    {
        _var->setUnits(_optionSelected);
        _sensor->initUnits();
    }

    _currentOption = 2;
    _optionSelected = _var->getIsAutoOff();
    optionSelect();
    _var->setIsAutoOff(_optionSelected);

    // TODO re-enable interrupt on change for the buttons

    _menuExit = false;
}

void menu::processModeMenu(void)
{

    char serial[28];
    char version[28];
    char calDate[28];
    char calDue[28];
    uint16_t storedCalDate;
    uint16_t storedCalDue;
    uint8_t counter = 0;
    //TODO: turn off the all the button interrupt except the onOff button

    _display->setFont(u8g2_font_helvB08_tf);
    _display->clearBuffer();
    
    // show sensor serial
    sprintf(serial,"Sensor Serial Nr.: %5d",_sensor->getSerial());
    _display->msg(serial,5,9);

     // show calibrated data
    storedCalDate = _sensor->getCalDate();
    sprintf(calDate,"Calibrated: %4d-%02d",((storedCalDate / 100) + 2000),storedCalDate%100);
    _display->msg(calDate,5,18);

    // show cliabration due date
    storedCalDue = _sensor->getCalDue();
    sprintf(calDue,"Calibration Due: %4d-%02d",((storedCalDue / 100) + 2000),storedCalDue%100);
    _display->msg(calDue,5,27);

    //show sensor firmware version
    sprintf(version,"Sensor Firmware: %4d",_sensor->getSensorVersion());
    _display->msg(version,5,36);

    // show display firmware version
    sprintf(version,"Display Firmware: %02d%02d",DISPLAY_VERSION_YEAR,DISPLAY_VERSION_MONTH);
    _display->msg(version,5,45);
    _display->update();
    delay(500);
    // if(menuButtonProcess())
    // {
    //     exitMenu();
    //     return; 
    // }
    while(!_button->isPressed(BTN_MODE))
    {
        if(counter>153)
        {
            break;
        }
        counter++;
        delay(10);
        Serial.println(counter);
    }
    exitMenu();
    Serial.println("exit display");
}


void menu::processSecretMenu(void)
{
    
    char adcGain[9];
    //TODO: turn off interrupt for buttons except onff button
    
    _currentOption = 3;
    _optionSelected = _var->isRawMode;
    optionSelect();
    _var->isRawMode = _optionSelected;

    if(_var->isRawMode)
    {
        _var->setUnits(UNIT_MV);
        _sensor->resetAdc();
    }
    else
    {
        _var->setUnits(UNIT_N);
        _sensor->initFast();
        _sensor->initUnits();
    }

    delay(300);
    _display->clearBuffer();
    // show current ADC gain
    sprintf(adcGain,"   %5d",_sensor->getAdcGain());
    _display->msg("FScale: ",adcGain,1500,10,true);
    
    delay(100);

    //TODO: turn on the button interupt

    _menuExit = false;
    _var->systemZero = true;
    _var->autoOffCount = 0;
    _sensor->zeroFast();
}


bool menu::menuButtonProcess(void)
{
    uint8_t counter = 0;
    while(!_button->isPressed(BTN_MODE))
    {
        if(_button->isPressed(BTN_ZERO)
         ||_button->isPressed(BTN_FUNC))
        {
            return true;
        }
        if(counter>153)
        {
            break;
        }
        counter++;
        delay(10);
    }

    return false;
}

void menu::exitMenu(void)
{
    delay(570);
    // TODO: Turn on all the interrupt for the buttons
}

void menu::setMenuExit(bool enabled)
{
    _menuExit = enabled;
}
