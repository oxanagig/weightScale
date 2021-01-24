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

void button::setPressed(int button)
{
    switch(button)
    {
        case BTN_FUNC: 
            _hasPressed[0] = true;
            break;
        case BTN_ONOFF:
            _hasPressed[1] = true;
            break;
        case BTN_ZERO:
            _hasPressed[2] = true;
            break;
        case BTN_MODE:
            _hasPressed[3] = true;
            break;
        default:
        break;
    }
}

bool button::hasPressed(int button)
{
    bool pressed = false;
    switch(button)
    {
        case BTN_FUNC: 
            pressed = _hasPressed[0];
            _hasPressed[0] = false;
            break;
        case BTN_ONOFF:
            pressed = _hasPressed[1];
            break;
        case BTN_ZERO:
            pressed = _hasPressed[2];
            break;
        case BTN_MODE:
            pressed = _hasPressed[3];
            break;
        default:
            break;
    }
    return pressed;
}

void button::clearPressState(void)
{
    uint8_t i;

    for(i=0;i<4;i++)
    {
        _hasPressed[i] = false;
    }
}