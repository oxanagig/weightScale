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
#define INTERRUPT_PIN PIN_BUTTON1
#define LOOP_CYCLES 100
#define CYCLE_TIME_MS 10
#define DEBOUNCE_TIME 100

#ifdef DEBUG
#define DEBUG_MSG(msg) (Serial.print(msg))
#else
#define DEBUG_MSG(msg)
#endif

#define DISPLAY_RESET_PIN 7
#define MBAR_ONOFF 5 // this a 5V output pin

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
U8G2_SSD1309_128X64_NONAME2_F_HW_I2C u8g2(U8G2_R2, DISPLAY_RESET_PIN, PIN_WIRE_SCL, PIN_WIRE_SDA);
display Display(&u8g2, &variables);
wiredSensor sensor(&adc, &variables);
menu Menu(&Display, &Button, &sensor, &variables);

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

    Display.clearBuffer();
    variables.isConnected = sensor.getConnection();
    while (variables.isConnected == false)
    {
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
    delay(1000);
}

void displayOn(void)
{
    variables.sleepMode = false;
    // release display and sensor reset pin
    digitalWrite(MBAR_ONOFF, HIGH);
    digitalWrite(DISPLAY_RESET_PIN, HIGH);
    // delay(350); //increased pause value to fix "while plugged in" connection issue (still says 'attach sensor') (1507)
    Display.begin();
    u8g2.setBusClock(100000L);
    u8g2.setFont(u8g2_font_ncenB14_tr);
    //Record number of uses
    variables.setDispalyUses(variables.getDisplayUses() + 1);
    DEBUG_MSG("display use:");
    DEBUG_MSG(variables.getDisplayUses());
    DEBUG_MSG('\n');
    enableButtonInterrupt(BTN_ONOFF);
    //delay(50);

    findSensor();
}

void displaySleep(void)
{
    // Hold both display and sensor in reset
    pinMode(MBAR_ONOFF, OUTPUT);
    pinMode(DISPLAY_RESET_PIN, OUTPUT);

    digitalWrite(MBAR_ONOFF, LOW);
    digitalWrite(DISPLAY_RESET_PIN, LOW);

    // initialize gobal variables
    variables.isConnected = false;
    variables.autoOffCount = 0;
    Menu.setMenuExit(false);
    // clear interrupt on change??
    variables.sleepMode = true;
    systemState = SYSTEM_OFF;
    disableAllButtonInterrupt();
    enableButtonInterrupt(BTN_ONOFF);
    sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
    __WFE();
}

//  Checks how long the display has been on with the SAME value.
//  If the same value has stayed for too long, turn unit(device) off.
void autoOffCheck(int value)
{
    static int oldValue;

    // value = (value & 0xff00) | (int)((value & 0xff) / 4);
    DEBUG_MSG(value);
    DEBUG_MSG("\n");

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

    DEBUG_MSG(variables.autoOffCount);
    DEBUG_MSG("\n");
}

void buttonONFFInterruptHandler(void)
{
    static uint32_t last_interrupt_time = 0;
    uint32_t interrupt_time = millis();

    // If interrupts come faster than debouce time, assume it's a bounce and ignore
    if (interrupt_time - last_interrupt_time > DEBOUNCE_TIME)
    {
        //if(Button.isPressed(BTN_ONOFF))
        while (Button.isPressed(BTN_ONOFF))
            ;
        DEBUG_MSG("BTN_ONOFF\n");
        if (variables.sleepMode)
        {
            systemState = NO_SENSOR;
            NVIC_SystemReset();
        }
        else
        {
            systemState = SYSTEM_OFF;
            displaySleep();
        }
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

/*
  START HERE
*/

void setup()
{

    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(PIN_BUTTON1, INPUT_PULLUP);
    pinMode(DISPLAY_RESET_PIN, OUTPUT);
    pinMode(MBAR_ONOFF, OUTPUT);
    digitalWrite(DISPLAY_RESET_PIN, LOW);
    digitalWrite(MBAR_ONOFF, LOW);

    Serial.begin(115200);
    sensor.begin();

    digitalWrite(DISPLAY_RESET_PIN, HIGH);
    digitalWrite(MBAR_ONOFF, HIGH);
}

void loop()
{
    uint16_t adcReading;

    switch (systemState)
    {
    case NO_SENSOR:
        // this a blocking code
        // it will loop inside the function until find the sensor
        // or it reaches the auto off time limit
        variables.begin();
        displayOn();
        systemState = DISPLAY_MEASUREMENT;
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

        DEBUG_MSG("read sensor value: ");
        adcReading = sensor.getReading();
        DEBUG_MSG(adcReading);
        DEBUG_MSG("\n");

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
        /* the system should be turned off inside interrupt */
    default:
        break;
    }

    //cycles++;
    delay(CYCLE_TIME_MS);
}
