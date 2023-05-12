#include <Arduino.h>
#include <TFT_eSPI.h>

#define SWINGING_TIME       20
#define COUNTING_MODE       1
#define RANDOM_COLOR_MODE   10

static TFT_eSPI tft = TFT_eSPI();
static hw_timer_t *Timer0_Cfg = NULL;

static bool flag_10ms = false;
static bool flag_100ms = false;
static bool flag_500ms = false;
static bool flag_1s = false;

// buttons
static bool button_left = false;
static bool button_right = false;

#define LEFT_KEY 0
#define RIGHT_KEY 35


static int timer_10ms = 0;
static int timer_100ms = 0;
static int timer_500ms = 0;
static int timer_1s = 0;

// **************************************************************************
void IRAM_ATTR Timer0_ISR()
{
    if (++timer_10ms >= 10)
    {
        timer_10ms = 0;
        flag_10ms = true;
    }

    if (++timer_100ms >= 100)           // for fast display color changeing
    {
        timer_100ms = 0;
        flag_100ms = true;
    }

    if (++timer_500ms >= 500)           // for blinking display
    {
        timer_500ms = 0;
        flag_500ms = true;
    }

    if (++timer_1s >= 1000)             // for decrementing the counter
    {
        timer_1s = 0;
        flag_1s = true;
    }

}   // Timer0_ISR


// **************************************************************************
void ButtonsProcess(void)
{
    static bool button_left_prev = false;
    static bool button_right_prev = false;

        if (digitalRead(LEFT_KEY) == 0)
        {
            if (button_left_prev == false)
                button_left = true;

            button_left_prev = true;
        }
        else
        {
            button_left_prev = false;
        }

        if (digitalRead(RIGHT_KEY) == 0)
        {
            if (button_right_prev == false)
                button_right = true;

            button_right_prev = true;
        }
        else
        {
            button_right_prev = false;
        }

}   // ButtonsProcess


// **************************************************************************
void DispNumber(uint16_t val)
{
    char str[5];
    utoa(val, str, 10);

    tft.fillScreen(TFT_BLACK);

    if (val < 10)
        tft.drawString(str, 45, 70, 8);
    else
        tft.drawString(str, 16, 70, 8);

}   // DispNumber


// **************************************************************************
void MultiColorScreen(bool reset)
{
    static uint8_t mode = 0;

    if (reset)
        mode = 0;

    switch (mode)
    {
    case 0:     tft.fillScreen(TFT_WHITE);       break;
    case 1:     tft.fillScreen(TFT_RED);         break;

    case 2:     tft.fillScreen(TFT_DARKGREEN);   break;
    case 3:     tft.fillScreen(TFT_DARKCYAN);    break;
    case 4:     tft.fillScreen(TFT_MAROON);      break;
    case 5:     tft.fillScreen(TFT_PURPLE);      break;
    case 6:     tft.fillScreen(TFT_OLIVE);       break;
    case 7:     tft.fillScreen(TFT_LIGHTGREY);   break;
    case 8:     tft.fillScreen(TFT_DARKGREY);    break;
    case 9:     tft.fillScreen(TFT_BLUE);        break;
    case 10:    tft.fillScreen(TFT_GREEN);       break;
    case 11:    tft.fillScreen(TFT_CYAN);        break;
    case 12:    tft.fillScreen(TFT_NAVY);        break;
    case 13:    tft.fillScreen(TFT_MAGENTA);     break;
    case 14:    tft.fillScreen(TFT_YELLOW);      break;
    case 15:    tft.fillScreen(TFT_BLACK);       break;
    case 16:    tft.fillScreen(TFT_ORANGE);      break;
    case 17:    tft.fillScreen(TFT_GREENYELLOW); break;
    case 18:    tft.fillScreen(TFT_PINK);        break;
    case 19:    tft.fillScreen(TFT_BROWN);       break;
    case 20:    tft.fillScreen(TFT_GOLD);        break;
    case 21:    tft.fillScreen(TFT_SILVER);      break;
    case 22:    tft.fillScreen(TFT_SKYBLUE);     break;
    case 23:    tft.fillScreen(TFT_VIOLET);      break;
    }

    if (++mode >= 2)
        mode = 0;

}   // MultiColorScreen


// **************************************************************************
// LCD: 135 x 240
void setup()
{
    pinMode(LEFT_KEY, INPUT_PULLUP);
    pinMode(RIGHT_KEY, INPUT_PULLUP);

    tft.init();
    // tft.setRotation(1);     // horizontal
    tft.setRotation(0);        // vertical ?
    tft.fillScreen(TFT_BLACK);

    // tft.drawString("Hello World", 30, 30, 2);
    // tft.drawString("10", 45, 80, 7);
    // tft.drawString("10", 16, 70, 8);

    // Timer0_Cfg = timerBegin(0, 80, true);
    Timer0_Cfg = timerBegin(0, 80, true);
    timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);
    timerAlarmWrite(Timer0_Cfg, 1000, true);         // 1ms sec
    timerAlarmEnable(Timer0_Cfg);

}   // setup


// **************************************************************************
void ReadButtons (uint8_t *mode, int *number, bool op)
{
    if (button_right)
    {
        button_right = false;

        *number = SWINGING_TIME;
        *mode = COUNTING_MODE;

        timer_1s = 0;
        flag_1s = true;
    }

    if (op)
    {
        if (button_left)
        {
            button_left = false;

            *mode = RANDOM_COLOR_MODE;
            timer_100ms = 0;
        }
    }
    else
        button_right = false;
}

// **************************************************************************
void loop()
{
    static uint8_t mode = 0;
    static int number = 0;
    static bool reset = 0;

    ButtonsProcess();

    switch (mode)
    {
        default:             // waiting for button
            ReadButtons(&mode, &number, true);
            break;

        case COUNTING_MODE:             // counting down

            if (flag_1s)
            {
                flag_1s = false;

                if (number >= 0)
                    DispNumber((uint8_t)number);

                if (--number == -2)
                {
                    ++mode;
                    reset = true;
                    flag_500ms = true;
                    timer_500ms = 0;

                    button_left = false;
                    button_right = false;
                }
            }
            break;

        case COUNTING_MODE + 1:         // blinking display
            if (flag_500ms)
            {
                flag_500ms = false;

                MultiColorScreen(reset);
                reset = false;
            }

            ReadButtons(&mode, &number, true);
            break;

        case RANDOM_COLOR_MODE:        // random display color
            if (flag_100ms)           // fast display color changeing
            {
                flag_100ms = false;
                uint32_t color = random(0xFFFF);

                if (digitalRead(LEFT_KEY) == 0)
                {
                    tft.fillScreen(color);
                }
            }

            ReadButtons(&mode, &number, false);

            break;
        case RANDOM_COLOR_MODE + 1:
            break;
    }


    if (flag_10ms)
    {
        flag_10ms = false;
    }

    if (flag_100ms)
    {
        flag_100ms = false;
        ButtonsProcess();
    }

}   // loop

