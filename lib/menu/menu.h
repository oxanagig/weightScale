/*
  menu.h 
  Library for menu system
  Created by Xiang Gao, November 27, 2020.
*/
#ifndef MENU_h
#define MENU_h

#include "Arduino.h"
#include "display.h"
#include "gVariables.h"
#include "button.h"
#include "wiredSensor.h"

#define NO_ACTION     0xFF
#define FUNCTION_MENU 0
#define MODE_MENU     1
#define SECRET_MENU   2

class menu
{
  public:
    menu(display* Display = NULL, button* Button=NULL, wiredSensor* sensor=NULL, gVariables* variable=NULL);
    void processFunctionMenu();
    void processModeMenu();
    void processSecretMenu();
    void setMenuExit(bool enabled);
    
  private:

    const char DS_TITLE[12] = "Peak Force";
    const char DS_OP0[9] = "Off";
    const char DS_OP1[9] = "On";

    const char U_TITLE[9] = "Units";
    const char U_OP0[15] = "lb Force";
    const char U_OP2[15] = "Kg Force";

    const char AO_TITLE[9] = "Auto Off";
    const char AO_OP0[9] = "Enabled";
    const char AO_OP1[9] = "Disabled";

    const char KNEWTON[12]="kNewton";
    const char NEWTON[10]="N";

    const char SM_TITILE[9] = "mV mode";
    const char SM_OP0[9] = "Off";
    const char SM_OP1[9] = "On";
  
    display* _display;
    button* _button;
    gVariables* _var;
    wiredSensor* _sensor;
    
    bool _menuExit;
    uint8_t _optionSelected;    
    int _currentOption;
    const char* _option[4];
    const char* _optionTitle;

    uint8_t _processMenu;
    void optionSelect(void);
    bool menuButtonProcess(void);
    void exitMenu(void);
    void displayCurrentFunction();
};

#endif