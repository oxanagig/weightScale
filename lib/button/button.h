/*
  button.h 
  Library for button 
  Created by Xiang Gao, November 29, 2020.
*/

#include <arduino.h>

#ifndef BUTTON_h
#define BUTTON_h

#define BTN_FUNC    5
#define BTN_ONOFF   6
#define BTN_ZERO    7
#define BTN_MODE    8 

class button
{
    public:
        button();
        bool isPressed(int button);
        bool hasPressed(int button);
        void setPressed(int button);
        void clearPressState(void);

    private:
        bool _hasPressed[4];
};

#endif
