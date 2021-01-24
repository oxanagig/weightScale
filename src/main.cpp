#include <Arduino.h>
#include <U8g2lib.h>
#include <display.h>
#include <adc30.h>
#include <SPI.h>
#include <wiredSensor.h>
#include <button.h>
#include <menu.h>
#include <gVariables.h>

#define DEBUG 
#define INTERRUPT_PIN   PIN_BUTTON1
#define LOOP_CYCLES     100
#define CYCLE_TIME_MS   10

#ifdef DEBUG
#define DEBUG_MSG(msg) (Serial.print(msg))
#else
#define DEBUG_MSG(msg) 
#endif


#define DISPLAY_RESET_PIN 7
#define MBAR_ONOFF        5 // this a 5V output pin

enum SYS_STATE 
{
    NO_SENSOR,
    DISPLAY_MEASUREMENT,
    DISPLAY_MENU,
    DISPLAY_STATUS,
    DISPLAY_SECRET
};

enum SYS_STATE systemState = NO_SENSOR;
gVariables variables;
adc30 adc;
button Button;
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R2,DISPLAY_RESET_PIN,PIN_WIRE_SCL,PIN_WIRE_SDA);
display Display(&u8g2,&variables);
wiredSensor sensor(&adc,&variables);
menu Menu(&Display,&Button,&sensor,&variables);


volatile int cycles = 0;

uint16_t gain;

void buttonONFFInterruptHandler(void);
void buttonFuncInterruptHandler(void);
void buttonZeroInterruptHandler(void);
void buttonModeInterruptHandler(void);
//void buttonInterruptHandler(void);
void startSensor(void);
void autoOffCheck(int value);


void enableAllButtonInterrupt(void)
{
  attachInterrupt(digitalPinToInterrupt(BTN_FUNC), buttonFuncInterruptHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_MODE), buttonModeInterruptHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_ZERO), buttonZeroInterruptHandler, RISING);
  attachInterrupt(digitalPinToInterrupt(BTN_ONOFF), buttonONFFInterruptHandler, RISING);
}

void disableAllButtonInterrupt(void)
{
    detachInterrupt(digitalPinToInterrupt(BTN_FUNC));
    detachInterrupt(digitalPinToInterrupt(BTN_MODE));
    detachInterrupt(digitalPinToInterrupt(BTN_ZERO));
    detachInterrupt(digitalPinToInterrupt(BTN_ONOFF));
}

void enableButtonInterrupt(int button)
{
    switch(button)
    {
        case BTN_FUNC:
            attachInterrupt(digitalPinToInterrupt(button),buttonFuncInterruptHandler, RISING);
            break;
        case BTN_MODE:
            attachInterrupt(digitalPinToInterrupt(button),buttonModeInterruptHandler, RISING);
            break;
        case BTN_ZERO:
            attachInterrupt(digitalPinToInterrupt(button),buttonZeroInterruptHandler, RISING);
            break;
        case BTN_ONOFF:
            attachInterrupt(digitalPinToInterrupt(button),buttonONFFInterruptHandler, RISING);
            break;
        default:
            break;
    }
    
}

void findSensor(void)
{
    // no sensor is connected (communication timeout)
    variables.isConnected = false;
    // isRpm = false; isRpm is not used in the oringal code
    // counter =0; not used
    variables.autoOffCount = 0;
    variables.autoOffMax = 900; //90 on wireless?

    enableButtonInterrupt(BTN_ONOFF);

    Display.clearBuffer();

    while(variables.isConnected==false)
    {
        switch(variables.getLang())
        {
            case LANG_ES : Display.msgFirstLine("Conectar");break;
            case LANG_DE : Display.msgFirstLine("Anschl. ");break;
            default:       Display.msgFirstLine("Attach  ");break;
        }

        Display.msgSecondLine("Sensor  ");

        //Check for wired serial connection -- look for serial signature(?)
        //Possible ways: wait for serial value (A), check if RX pin is high? (1) 
        variables.isConnected = sensor.getConnection();
        //TODO: Bypass for now
        variables.isConnected = true;
        Display.update();
        delay(350);
        Display.clearDisplay();
        delay(70);

        if(variables.getIsAutoOff())
        {
            autoOffCheck(0);
        }
    }

    enableAllButtonInterrupt();
    delay(100);
    startSensor();
}


//Initialize a new sensor -- show serial, firmware, cal date
void startSensor(void)
{
    uint16_t storedCalDue;
    char calDate[9];
    variables.autoOffCount = 0;
    variables.autoOffMax = 3200;

    if(variables.getUnits() == UNIT_MV)
    {
        variables.setUnits(UNIT_N);
    }

    sensor.initFast();
    sensor.initUnits();

    switch(variables.getLang())
    {
        case LANG_ES : Display.msgFirstLine("Cal. Deb");break;
        case LANG_DE : Display.msgFirstLine("K.Fállig");break;
        default:       Display.msgFirstLine("Cal. Due");break;
    }
    storedCalDue = sensor.getCalDue();
    sprintf(calDate," %4d-%02d",((storedCalDue / 100) + 2000),storedCalDue%100);
    Display.msgSecondLine(calDate);

    Display.update();
    delay(1700);
}


void displayOn(void)
{
    variables.sleepMode = false; 
    // release display and sensor reset pin
    digitalWrite(MBAR_ONOFF,HIGH);
    digitalWrite(DISPLAY_RESET_PIN,HIGH); 
    delay(350); //increased pause value to fix "while plugged in" connection issue (still says 'attach sensor') (1507)
    
    //Record number of uses
    variables.setDispalyUses(variables.getDisplayUses()+1);
    
    enableButtonInterrupt(BTN_ONOFF);
    delay(50);

    findSensor();
}

void displaySleep(void)
{
    // Hold both display and sensor in reset
    pinMode(MBAR_ONOFF,OUTPUT);
    pinMode(DISPLAY_RESET_PIN, OUTPUT);

    digitalWrite(MBAR_ONOFF,LOW);
    digitalWrite(DISPLAY_RESET_PIN,LOW);

    // initialize gobal variables
    variables.isConnected = false;
    //lcdBlinkWait = 10;
    //lcdWait = 350;
    variables.autoOffCount = 0;
    Menu.setMenuExit(false);
    // clear interrupt on change??
    variables.sleepMode = true;

    //enableButtonInterrupt(BTN_ONOFF);
    // nRF5x_lowPower.enableWakeupByInterrupt(BTN_ONOFF, HIGH);
    // nRF5x_lowPower.powerMode(POWER_MODE_OFF);
}

//  Checks how long the display has been on with the SAME value.
//  If the same value has stayed for too long, turn unit(device) off.
void autoOffCheck(int value)
{
    static int oldValue;

    value = (value & 0xff00) | (uint8_t)((value & 0xff)/3);
    if( oldValue == (value & 0xff))
    {
        variables.autoOffCount = variables.autoOffCount + 1;
  
        if( variables.autoOffCount >= variables.autoOffMax)
        {
            Display.clearBuffer();   
            Display.msgFirstLine("Auto Off");
            Display.update();
            delay(1700);
            displaySleep();
        }
    }
    else
        variables.autoOffCount = 0;
    
    oldValue = value & 0xff;

}

void buttonONFFInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    
    if(variables.sleepMode)
    {
        // If interrupts come faster than 200ms, assume it's a bounce and ignore
        if (interrupt_time - last_interrupt_time > 200)
        {
            //if(Button.isPressed(BTN_ONOFF))
            while(Button.isPressed(BTN_ONOFF));
            DEBUG_MSG("BTN_OFF\n");
            displayOn();
        }
        last_interrupt_time = interrupt_time;
    }
    
}

void buttonFuncInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    
    if(variables.sleepMode)
        displaySleep();
    else
    {
        if (interrupt_time - last_interrupt_time > 200)
        {
            while(Button.isPressed(BTN_FUNC));
            DEBUG_MSG("func pressed\n");
            Button.setPressed(BTN_FUNC);
            //Menu.functionMenu();
        }
    }
}
void buttonZeroInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    
    if(variables.sleepMode)
        displaySleep();
    else
    {
        if (interrupt_time - last_interrupt_time > 200)
        {
            while(Button.isPressed(BTN_ZERO));
            DEBUG_MSG("zero pressed\n");
            Button.setPressed(BTN_ZERO);
            //sensor.setSystemZero();
        }
        last_interrupt_time = interrupt_time;
    }
}
void buttonModeInterruptHandler(void)
{
    disableAllButtonInterrupt();
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();
    
    if(variables.sleepMode)
        displaySleep();
    else
    {
        if (interrupt_time - last_interrupt_time > 200)
        {
            while(Button.isPressed(BTN_MODE));
            DEBUG_MSG("mode presssed\n");
            Button.setPressed(BTN_MODE);
            //Menu.modeMenu();
        }
        last_interrupt_time = interrupt_time;
    }
    enableAllButtonInterrupt();
}

/*
  START HERE
*/

void setup() {
  
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode(PIN_BUTTON1,INPUT_PULLUP);
  pinMode(DISPLAY_RESET_PIN,OUTPUT);
  pinMode(MBAR_ONOFF,OUTPUT);
  digitalWrite(DISPLAY_RESET_PIN,LOW);
  digitalWrite(MBAR_ONOFF,LOW);

  variables.begin();

  Display.begin();

  Serial.begin(9600);
  
  adc.begin();
  sensor.begin();
  u8g2.setBusClock(100000L);
  u8g2.setFont(u8g2_font_ncenB14_tr);


  //nrf_gpio_cfg_sense_input(29, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
  //attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), intHandler, RISING);
  //detachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN));

  digitalWrite(DISPLAY_RESET_PIN,HIGH);
  digitalWrite(MBAR_ONOFF,HIGH);
}

void loop() 
{
    uint16_t adcReading;

    switch(systemState)
    {
        case NO_SENSOR:
                // this a blocking code
                // it will loop inside the function until find the sensor
                // or it reaches the auto off time limit
                displayOn();
                systemState = DISPLAY_MEASUREMENT;
                break;

        case DISPLAY_MEASUREMENT:

            DEBUG_MSG(Button.hasPressed(BTN_ZERO));
            DEBUG_MSG("\n");

            if(Button.hasPressed(BTN_ZERO)
             &&Button.hasPressed(BTN_MODE))
            {
                systemState = DISPLAY_SECRET;
            }
            else if(Button.hasPressed(BTN_ZERO))
            {
                sensor.setSystemZero();
                DEBUG_MSG("SystemZero\n");
                variables.autoOffCount = 0;
                sensor.zeroFast();
            }
            else if(Button.hasPressed(BTN_MODE))
            {
                systemState = DISPLAY_STATUS;
            }
            else if(Button.hasPressed(BTN_FUNC))
            {
                systemState = DISPLAY_MENU;
            }
                Button.clearPressState();

            //DEBUG_MSG("read sensor value: ");
            adcReading = sensor.getReading(); 
            //DEBUG_MSG(adcReading);
            //DEBUG_MSG("\n");

            //if value is 0 or 65535, check the memory to make sure we're still connected
            if(adcReading==0 || adcReading==65535)
            {
                if(!sensor.getConnection())
                {
                    // sensor not found
                    findSensor();
                }
                
            }
            if(variables.getIsAutoOff())
            {
                autoOffCheck(adcReading);
            }
            
            Display.clearBuffer();   
            Display.setSensorValue(adcReading);
            Display.setValueFormat(FAMILY_STANDARD_FORCE);
            //DEBUG_MSG("display first line\n");
            Display.displaySensorValue(0);
            Display.setDisplayStatus(variables.getDisplayStatus());
            //DEBUG_MSG("display second line\n");
            Display.displaySensorValue(1);
            Display.update();
            break;

        case DISPLAY_STATUS:
            Menu.processModeMenu();
            Button.clearPressState();
            systemState = DISPLAY_MEASUREMENT;
            break;
        
        case DISPLAY_MENU:
            Menu.processFunctionMenu();
            Button.clearPressState();
            systemState = DISPLAY_MEASUREMENT;
            break;

        case DISPLAY_SECRET:
            Menu.processSecretMenu();
            Button.clearPressState();
            systemState = DISPLAY_MEASUREMENT;
            break;

        default:
            break;
    }


    //cycles++;
    delay(CYCLE_TIME_MS);
}

