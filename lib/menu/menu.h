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
    void functionMenu();
    void modeMenu();
    void secretMenu();
    void process();
    void setMenuExit(bool enabled);
    
  private:

    const char DS_TITLE_ES[9] = "Mostrar:";
    const char DS_OP0_ES[9] = "  Fuerza";
    const char DS_OP1_ES[9] = "  F/Pico";
    const char DS_TITLE_DE[9] = "Anzeige:";
    const char DS_OP0_DE[9] = "   Kraft";
    const char DS_OP1_DE[9] = "Kr|Spitz";
    const char DS_TITLE[9] = "Display:";
    const char DS_OP0[9] = "   Force";
    const char DS_OP1[9] = "  F/Peak";

    const char U_TITLE_ES[9] = "Medida: ";
    const char U_OP0_ES[9] = " Libra-F";
    const char U_OP2_ES[9] = "    kg-F";
    const char U_TITLE_DE[9] = "Einh.:  ";
    const char U_OP0_DE[9] = "Pound-Kr";
    const char U_OP2_DE[9] = "   kg-Kr";
    const char U_TITLE[9] = "Units:  ";
    const char U_OP0[9] = " Pound-F";
    const char U_OP2[9] = "    kg-F";

    const char AO_TITLE_ES[9] = "AutoApag";
    const char AO_OP0_ES[9] = "  Activo";
    const char AO_OP1_ES[9] = " Apagado";
    const char AO_TITLE_DE[9] = "AutoOff:";
    const char AO_OP0_DE[9] = "   Aktiv";
    const char AO_OP1_DE[9] = "     Aus";
    const char AO_TITLE[9] = "AutoOff:";
    const char AO_OP0[9] = "      On";
    const char AO_OP1[9] = "     Off";

    const char L_TITLE_ES[9] = "Idioma: ";
    const char L_TITLE_DE[9] = "Sprache:";
    const char L_TITLE[9] = "Lang.:  ";

    const char ENG [9] = "English";
    const char DE[9] = "Deutsch";
    const char ES[9] = "Español";

    const char KNEWTON[9]=" kNewton";
    const char NEWTON[9]="  Newton";

    const char SM_TITILE[9] = "mV mode:";
    const char SM_OP0[9] = "     Off";
    const char SM_OP1[9] = "      On";
  
    display* _display;
    button* _button;
    gVariables* _var;
    wiredSensor* _sensor;
    
    bool _menuExit;
    uint8_t _optionSelected;    
    const char* _option0;
    const char* _option1;
    const char* _option2;
    const char* _option3;
    const char* _optionTitle;

    uint8_t _processMenu;

    void processFunctionMenu();
    void processModeMenu();
    void processSecretMenu();
    void optionSelect(void);
    bool menuButtonProcess(void);
    void exitMenu(void);
};

#endif