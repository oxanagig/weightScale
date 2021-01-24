/*
  button.cpp 
  Library for button 
  Created by Xiang Gao, November 29, 2020.
*/
#include "button.h"

button::button()
{
    pinMode(BTN_FUNC,INPUT_PULLUP);
    pinMode(BTN_ONOFF,INPUT_PULLUP);
    pinMode(BTN_ZERO,INPUT_PULLUP);
    pinMode(BTN_MODE,INPUT_PULLUP);
}

bool button::isPressed(int button)
{
    if( digitalRead(button) == HIGH )
        return false;
    
    return true;
}

