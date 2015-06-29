#include "config.h"
#include "utils.h"
#include "pinout.h"
#include "display.h"
#include <PDQ_GFX.h>
#include "PDQ_ST7735_config.h"
#include <PDQ_ST7735.h>
PDQ_ST7735 tft;

#define NUMBER_OF_RECEIVER 3
#define VBAT_FACTOR 190

const unsigned char RSSI_Pin[3] = { A2, A1, A0}; //Analog Pin

// bands
enum {
    BAND_A = 0,
    BAND_B,
    BAND_E,
    BAND_IRC,
    BAND_RACER    
};

// Channels with their Mhz Values
const uint16_t channelFreqTable[] PROGMEM = {
    // Channel 1 - 8
    5865, 5845, 5825, 5805, 5785, 5765, 5745, 5725, // Band A  00 - 07
    5733, 5752, 5771, 5790, 5809, 5828, 5847, 5866, // Band B  08 - 15
    5705, 5685, 5665, 5645, 5885, 5905, 5925, 5945, // Band E  16 - 23
    5740, 5760, 5780, 5800, 5820, 5840, 5860, 5880, // IRC/FS  24 - 31
    5658, 5695, 5732, 5769, 5806, 5843, 5880, 5917  // Race    32 - 39
};

// All Channels of the above list ordered by Mhz
const uint8_t channelList[] PROGMEM = {
    19, 32, 18, 17, 33, 16,  7, 34,  8, 24, 
     6,  9, 25,  5, 35, 10, 26,  4, 11, 27,
     3, 36, 12, 28,  2, 13, 29, 37,  1, 14,
    30,  0, 15, 31, 38, 20, 21, 39, 22, 23
};

uint16_t RSSI_Value[NUMBER_OF_RECEIVER];
uint8_t SelectedSource = 0;
uint8_t vbat;
uint8_t MaxSource = 0; 

// timers
uint32_t saveSettings = 0;
uint32_t checkVbat = 0;

enum STATES{
    STATE_SPLASH,
    STATE_MAIN, // main diversity dialog
    STATE_CALIB // bar graph
};

static uint8_t state = STATE_MAIN;
    
void setup()
{
    // read settings from eeprom
    readConfig();
    // init vRXs
    SPI_vRX_init();
    // init display
    TFT_init_display();
    // set analog reference
    analogReference(EXTERNAL); 
    // setup leds
    DDRC |= 0b00111000; // output
    // setup pi5v331
    initSwitcher();
    // setup switches
    DDRD &= ~0b10000011; // input
    DDRB &= ~0b00010000;
    PORTD|=  0b10000011; // pull up
    PORTB|=  0b00010000;
    // setup buzzer
    DDRB |= 0b00000001; // output
    BUZZ_OFF;
    // setup 250khz adc (slightly more than recommended 200 khz max setting
    //setAdcPrescaler(ADC_PS_64);
    // splash screen
    displaySplash();
    // start with main dialog
    initState();
}

void loop()
{   
    switch(state) {
        case STATE_MAIN:
            processMainState();
            break;
        case STATE_CALIB:
            processCalibState();
            break;
    }
    
    // save settings to eeprom
    if(saveSettings && millis() > saveSettings)
    {
        writeConfig();
        saveSettings = 0;
    }
    
    // refresh vbat
    if(millis() > checkVbat) {
        checkVbat = millis() + 20000; // check every 20s
        vbat = map( analogRead(A6)*10, 0, 10240, 0, VBAT_FACTOR);
        updateMainDialog(_BV(MAIN_BATTERY));        
    }
    
    // todo: manage non blocking buzzer
}

void initState()
{
    switch(state) {
        case STATE_MAIN:
            updateMainDialog(_BV(MAIN_INIT) | _BV(MAIN_BAND) | _BV(MAIN_CHANNEL) );
            break;
        case STATE_CALIB:
            updateCalibDialog(_BV(CALIB_INIT)); 
            break;
    }
    checkVbat = 0;
}

void processCalibState()
{
    static uint32_t refresh;
    switchBestRSSI();
    if(BTN_UP) // return to main dialog
    {
        shortbeep();
        state = STATE_MAIN;
        initState();
        waitButtonsRelease();
        return;
    }
    if(millis() > refresh) {
        refresh = millis() + 1000/5; // x fps 
        updateCalibDialog(_BV(CALIB_VALUES) | _BV(CALIB_BARS));
    }
}

void processMainState()
{
    if(BTN_UP) { // basic calibration dialog
        shortbeep();
        state = STATE_CALIB;
        initState();
        waitButtonsRelease();
        return;
    } else 
    if(BTN_RIGHT || BTN_LEFT) { // change current channel
        shortbeep();
        if(BTN_RIGHT) {
            config.current_channel ++;
            if(config.current_channel > 39)
            config.current_channel = 0;
        }
        else if(BTN_LEFT) {
            config.current_channel --;
            if(config.current_channel < 0)
            config.current_channel = 39;
        }
        SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + config.current_channel));
        updateMainDialog(_BV(MAIN_BAND) | _BV(MAIN_CHANNEL));
        uint8_t autoRepeat = 7;
        while(BTN_ANY && autoRepeat--) {
            delay(20);
        }
        saveSettings = millis() + 3000; // save settings 3s after last change
    }
    switchBestRSSI();
}

void switchBestRSSI()
{
    for(uint8_t i = 0; i < NUMBER_OF_RECEIVER; i++) {
        RSSI_Value[i] = analogRead(RSSI_Pin[i]);
    }
    MaxSource = getMaxIndex(RSSI_Value, NUMBER_OF_RECEIVER);
    select(MaxSource);
}

uint16_t getMaxIndex(unsigned int Array[], unsigned int iSize)
{
    if(iSize <= 1)
    {
        return 0;
    }
    int16_t iMax = Array[0];
    uint8_t iIndexMax = 0;
    //int16_t hysteresis = analogRead(RSSI_Hysteresis_Pin) >> 5; // 0 - 3.125% hysteresis
    int16_t hysteresis = 0;
    static int16_t last_max = 0;
    static uint8_t last_index = 0;
    for(unsigned int i = 1; i < iSize; i++)
    {
        if(Array[i] > (uint16_t)iMax)
        {
            iMax = Array[i];
            iIndexMax = i;
        }
    }
    if((iMax < last_max - hysteresis) || (iMax > last_max + hysteresis)) {
        last_max = iMax;
        last_index = iIndexMax;
    }
    return last_index;
}

// short blocking beep for button press
void shortbeep()
{
    BUZZ_ON;
    delay(8);
    BUZZ_OFF;
}

// wait buttons release
void waitButtonsRelease()
{
    while(BTN_ANY) {
        delay(20);
        // todo: call non blocking buzzer manager
    }
}
