#include <Arduino.h>
#include <U8g2lib.h>
#include <display.h>
#include <adc30.h>
#include <SPI.h>
#include <wiredSensor.h>
#include <button.h>
#include <menu.h>
#include <gVariables.h>
#include <avr/sleep.h>

#define DEBUG
#define INTERRUPT_PIN PIN_BUTTON1
#define CYCLE_TIME_MS 100
#define DEBOUNCE_TIME 100

#ifdef DEBUG
#define DEBUG_MSG(msg) (Serial1.println(msg))
#else
#define DEBUG_MSG(msg)
#endif

#define DISPLAY_SWITCH_PIN 3
#define MBAR_ONOFF 9 
#define SHIFT_EN 14

enum SYS_STATE
{
    NO_SENSOR,
    DISPLAY_MEASUREMENT,
    DISPLAY_MENU,
    DISPLAY_STATUS,
    DISPLAY_SECRET,
    SYSTEM_OFF
};

enum SYS_STATE systemState = NO_SENSOR;
gVariables variables;
adc30 adc;
button Button;
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R2, U8X8_PIN_NONE, PIN_WIRE_SCL, PIN_WIRE_SDA);
display Display(&u8g2, &variables);
wiredSensor sensor(&adc, &variables);
menu Menu(&Display, &Button, &sensor, &variables);

static bool firstPowerOn = true;
volatile int cycles = 0;

uint16_t gain;

void buttonONFFInterruptHandler(void);
void buttonFuncInterruptHandler(void);
void buttonZeroInterruptHandler(void);
void buttonModeInterruptHandler(void);
void buttonInterruptHandler(void);
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
    switch (button)
    {
    case BTN_FUNC:
        attachInterrupt(digitalPinToInterrupt(button), buttonFuncInterruptHandler, RISING);
        break;
    case BTN_MODE:
        attachInterrupt(digitalPinToInterrupt(button), buttonModeInterruptHandler, RISING);
        break;
    case BTN_ZERO:
        attachInterrupt(digitalPinToInterrupt(button), buttonZeroInterruptHandler, RISING);
        break;
    case BTN_ONOFF:
        attachInterrupt(digitalPinToInterrupt(button), buttonONFFInterruptHandler, RISING);
        break;
    default:
        break;
    }
}

void findSensor(void)
{
    static bool attachSensorDisplayed = false;
    // no sensor is connected (communication timeout)
    variables.isConnected = false;
    // isRpm = false; isRpm is not used in the oringal code
    // counter =0; not used
    variables.autoOffCount = 0;
    variables.autoOffMax = 900; //90 on wireless?

    enableButtonInterrupt(BTN_ONOFF);
    // delay(500);
    Display.clearBuffer();
    variables.isConnected = sensor.getConnection();
    while (variables.isConnected == false)
    {
        
        // DEBUG_MSG("FIND SENSOR\n");
        if(variables.sleepMode)
            break;
        if (attachSensorDisplayed == false)
        {
            Display.setFont(u8g2_font_t0_22b_mf);
            switch (variables.getLang())
            {
            case LANG_ES:
                Display.msg("Conectar", 40, 30);
                break;
            case LANG_DE:
                Display.msg("Anschl.", 40, 30);
                break;
            default:
                Display.msg("Attach", 35, 25);
                break;
            }

            Display.msg("Sensor", 35, 45);

            Display.update();
            attachSensorDisplayed = true;
        }
        //Check for wired serial connection -- look for serial signature(?)
        //Possible ways: wait for serial value (A), check if RX pin is high? (1)
        variables.isConnected = sensor.getConnection();
        delay(420);
        if (variables.getIsAutoOff())
        {
            autoOffCheck(0);
        }
    }

    if(variables.sleepMode)
        return;
    attachSensorDisplayed = false;
    enableAllButtonInterrupt();
    adc.begin();
    startSensor();
}

//Initialize a new sensor -- show serial, firmware, cal date
void startSensor(void)
{
    uint16_t storedCalDue;
    char calDate[9];
    variables.autoOffCount = 0;
    variables.autoOffMax = 3200;

    if (variables.getUnits() == UNIT_MV)
    {
        variables.setUnits(UNIT_N);
    }

    sensor.initFast();

    Display.setFont(u8g2_font_t0_12b_mf);
    Display.clearBuffer();

    switch (variables.getLang())
    {
    case LANG_ES:
        Display.msg("Cal. Deb", 35, 25);
        break;
    case LANG_DE:
        Display.msg("K.Fállig", 35, 25);
        break;
    default:
        Display.msg("Calibration Due", 25, 25);
        break;
    }
    storedCalDue = sensor.getCalDue();
    sprintf(calDate, "%4d-%02d", ((storedCalDue / 100) + 2000), storedCalDue % 100);
    Display.setFont(u8g2_font_t0_22b_mf);
    Display.msg(calDate, 30, 45);

    Display.update();
    sensor.initUnits();
    
}

void displayOn(void)
{
    variables.sleepMode = false;
    // release display and sensor reset pin
    digitalWrite(MBAR_ONOFF, HIGH);
    digitalWrite(DISPLAY_SWITCH_PIN, HIGH);
    digitalWrite(SHIFT_EN,HIGH);
    // delay(350); //increased pause value to fix "while plugged in" connection issue (still says 'attach sensor') (1507)
    Display.begin();
    u8g2.setBusClock(100000L);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    //Record number of uses
    variables.setDispalyUses(variables.getDisplayUses() + 1);
    DEBUG_MSG("display use:");
    DEBUG_MSG(variables.getDisplayUses());
    DEBUG_MSG('\n');
    enableAllButtonInterrupt();
    delay(50);

    findSensor();
}

void displaySleep(void)
{
    // Hold both display and sensor in reset
    // pinMode(MBAR_ONOFF, OUTPUT);
    // pinMode(DISPLAY_SWITCH_PIN, OUTPUT);
    // pinMode(SHIFT_EN,OUTPUT);

    digitalWrite(MBAR_ONOFF, LOW);
    digitalWrite(DISPLAY_SWITCH_PIN, LOW);
    digitalWrite(SHIFT_EN,LOW);
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    pinMode(13, INPUT);

    variables.isConnected = false;
    variables.autoOffCount = 0;
    Menu.setMenuExit(false);
    variables.sleepMode = true;
    systemState = SYSTEM_OFF;
    disableAllButtonInterrupt();
    enableButtonInterrupt(BTN_ONOFF);
    set_sleep_mode(SLEEP_MODE_STANDBY);
    sleep_enable();
    sleep_cpu();
    sleep_disable();
}

//  Checks how long the display has been on with the SAME value.
//  If the same value has stayed for too long, turn unit(device) off.
void autoOffCheck(int value)
{
    static int oldValue;

    // value = (value & 0xff00) | (int)((value & 0xff) / 4);
    // DEBUG_MSG(value);
    // DEBUG_MSG("\n");

    if ((abs(oldValue - value) / 3) == 0)
    {
        variables.autoOffCount = variables.autoOffCount + 1;

        if (variables.autoOffCount >= variables.autoOffMax)
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

    oldValue = value;

    // DEBUG_MSG(variables.autoOffCount);
    // DEBUG_MSG("\n");
}

void buttonONFFInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    // If interrupts come faster than debouce time, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > 2 * DEBOUNCE_TIME)
    {
        //if(Button.isPressed(BTN_ONOFF))
        while (Button.isPressed(BTN_ONOFF))
            ;
        DEBUG_MSG("BTN_ONOFF\n");
        Button.setPressed(BTN_ONOFF);
        variables.sleepMode ^=1;
        DEBUG_MSG(variables.sleepMode);
        
    }
    last_interrupt_time = interrupt_time;
}

void buttonFuncInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    if (variables.sleepMode)
        displaySleep();
    else
    {
        if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME)
        {
            while (Button.isPressed(BTN_FUNC))
                ;
            DEBUG_MSG("func pressed\n");
            Button.setPressed(BTN_FUNC);
        }
    }
}
void buttonZeroInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    if (variables.sleepMode)
        displaySleep();
    else
    {
        if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME)
        {
            while (Button.isPressed(BTN_ZERO))
                ;
            DEBUG_MSG("zero pressed\n");
            Button.setPressed(BTN_ZERO);
        }
        last_interrupt_time = interrupt_time;
    }
}
void buttonModeInterruptHandler(void)
{
    disableAllButtonInterrupt();
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    if (variables.sleepMode)
        displaySleep();
    else
    {
        if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME)
        {
            while (Button.isPressed(BTN_MODE))
                ;
            DEBUG_MSG("mode presssed\n");
            Button.setPressed(BTN_MODE);
        }
        last_interrupt_time = interrupt_time;
    }
    enableAllButtonInterrupt();
}



void systemInit(void)
{

     while (RTC.STATUS > 0) { /* Wait for all register to be synchronized */
	}

	RTC.CTRLA = RTC_PRESCALER_DIV1_gc   /* 1 */
	            | 0 << RTC_RTCEN_bp     /* Enable: enabled */
	            | 0 << RTC_RUNSTDBY_bp; /* Run In Standby: enabled */

	RTC.CLKSEL = RTC_CLKSEL_INT1K_gc; /* 32KHz divided by 32 */


	RTC.INTCTRL = 0 << RTC_CMP_bp    /* Compare Match Interrupt enable: enabled */
	              | 0 << RTC_OVF_bp; /* Overflow Interrupt enable: disabled */


    for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTA + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTB + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTC + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}
  
    for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTD + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTE + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}

	for (uint8_t i = 0; i < 8; i++) {
		*((uint8_t *)&PORTF + 0x10 + i) |= 1 << PORT_PULLUPEN_bp;
	}
    /* DIR Registers Initialization */
    PORTA_DIR = 0xF1;
    PORTB_DIR = 0x3B;
    PORTC_DIR = 0xFF;
    PORTD_DIR = 0xFF;
    PORTE_DIR = 0x07;
    PORTF_DIR = 0x6F;


    BOD.INTCTRL = 0 << BOD_VLMIE_bp      /* voltage level monitor interrrupt enable: disabled */
                | BOD_VLMCFG_ABOVE_gc; /* Interrupt when supply goes above VLM level */

    WDT.CTRLA = 0x00;

    //VLMCFG BELOW; VLMIE disabled; 
	BOD.INTCTRL = 0x00;

    //VLMLVL 5ABOVE; 
	BOD.VLMCTRLA = 0x00;
}

/*
  START HERE
*/
void setup()
{
    systemInit();
    digitalWrite(DISPLAY_SWITCH_PIN, LOW);
    digitalWrite(MBAR_ONOFF, LOW);
    digitalWrite(SHIFT_EN, LOW);

    pinMode(DISPLAY_SWITCH_PIN, OUTPUT);
    pinMode(MBAR_ONOFF, OUTPUT);
    pinMode(SHIFT_EN, OUTPUT);
  
    Serial1.begin(9600);
    sensor.begin();
    // delay(1000);
    DEBUG_MSG("RESET register:");
    DEBUG_MSG(RSTCTRL_RSTFR);
    if(!(RSTCTRL_RSTFR & RSTCTRL_SWRF_bm))
    {
        systemState = SYSTEM_OFF;
        variables.sleepMode = 1;
        RSTCTRL_RSTFR = 0;
    }
}

void loop()
{
    uint16_t adcReading;
    DEBUG_MSG("power mode:");
    DEBUG_MSG(systemState);

    if(variables.sleepMode)
        systemState = SYSTEM_OFF;
    
    switch (systemState)
    {
        case NO_SENSOR:
            // this a blocking code
            // it will loop inside the function until find the sensor
            // or it reaches the auto off time limit
            variables.begin();
            displayOn();
            
            if(variables.sleepMode)
                systemState = SYSTEM_OFF;
            else
                systemState =DISPLAY_MEASUREMENT;
            Button.clearPressState();
            break;

        case DISPLAY_MEASUREMENT:

            if (Button.hasPressed(BTN_ZERO) && Button.hasPressed(BTN_MODE))
            {
                systemState = DISPLAY_SECRET;
            }
            else if (Button.hasPressed(BTN_ZERO))
            {
                sensor.setSystemZero();
                DEBUG_MSG("SystemZero\n");
                variables.autoOffCount = 0;
                sensor.zeroFast();
            }
            else if (Button.hasPressed(BTN_MODE))
            {
                systemState = DISPLAY_STATUS;
            }
            else if (Button.hasPressed(BTN_FUNC))
            {
                systemState = DISPLAY_MENU;
            }
            Button.clearPressState();

            //DEBUG_MSG("read sensor value: ");
            adcReading = sensor.getReading();
            //DEBUG_MSG(adcReading);
            //DEBUG_MSG("\n");

            //if value is 0 or 65535, check the memory to make sure we're still connected
            if (adcReading == 0 || adcReading == 65535)
            {
                if (!sensor.getConnection())
                {
                    // sensor not found
                    findSensor();
                    break;
                }
            }
            if (variables.getIsAutoOff())
            {
                autoOffCheck(adcReading);
            }

            Display.clearBuffer();
            Display.setSensorValue(adcReading);
            Display.setValueFormat(FAMILY_STANDARD_FORCE);
            Display.displaySensorValue(0);
            Display.setDisplayStatus(variables.getDisplayStatus());
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

        case SYSTEM_OFF:
            DEBUG_MSG("going to sleep\n");
            // delay(1000);
            displaySleep();
             _PROTECTED_WRITE(RSTCTRL.SWRR,RSTCTRL_SWRE_bm);
            systemState = NO_SENSOR;
        default:
            break;
    }

    cycles++;
    delay(CYCLE_TIME_MS);
}
