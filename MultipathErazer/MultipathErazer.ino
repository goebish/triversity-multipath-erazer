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
#define RSSI_STABILIZATION_TIME 50

const unsigned char RSSI_Pin[3] = { A2, A1, A0}; //Analog Pin

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
int16_t max_rssi = 0;
int8_t max_rssi_scan_index;
bool show_active_leds = true;
uint8_t current_main_menu_item;
boolean scan_first_pass = true;
uint16_t scan_max_found = 0;
uint16_t scan_last_max_found = 0;
uint8_t scan_channel;
uint8_t channelIndex;
uint8_t scan_best_index=0;
uint16_t anim_count;
bool searching=false;

// timers
uint32_t saveSettings = 0;
uint32_t checkVbat = 0;

enum e_STATES{
    STATE_MAIN,      // channel selection
    STATE_MAIN_MENU, // main menu
    STATE_CALIB,     // bar graph
    STATE_SCANNER,   // band scanner
    STATE_SETTINGS_MENU,  // settings menu
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
        case STATE_MAIN_MENU:
            processMainMenu();
            break;
        case STATE_SCANNER:
            processScanner();
            break;
        case STATE_SETTINGS_MENU:
            processSettingsMenu();
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
        vbat = map( analogRead(VBAT)*10, 0, 10240, 0, VBAT_FACTOR);
        updateMainDialog(_BV(MAIN_BATTERY));
    }

    alarmBeep();
    switchBestRSSI();
}

void initState()
{
    switch(state) {
        case STATE_MAIN:
            updateMainDialog(_BV(MAIN_INIT) | _BV(MAIN_BAND) | _BV(MAIN_CHANNEL));
            SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + config.current_channel));
            show_active_leds=true;
            break;
        case STATE_CALIB:
            updateCalibDialog(_BV(CALIB_INIT) | _BV(CALIB_HEADER));
            show_active_leds=true;
            break;
        case STATE_MAIN_MENU:
            current_main_menu_item = MAIN_MENU_EXIT;
            updateMainMenu(_BV(MAIN_MENU_INIT) | _BV(MAIN_MENU_ITEMS));
            show_active_leds=true;
            break;
        case STATE_SCANNER:
            scan_first_pass = true;
            scan_max_found = 0;
            scan_last_max_found = 0;
            searching = true;
            updateScannerDialog(_BV(SCANNER_INIT));
            show_active_leds=false;
            scan_channel = 0;
            channelIndex = pgm_read_byte_near(channelList + scan_channel);
            SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + channelIndex));
            delay(100);
            break;
        case STATE_SETTINGS_MENU:
            updateSettingsMenuDialog(_BV(SETTINGS_MENU_INIT));
            show_active_leds=true;
            break;
    }
}

void processSettingsMenu()
{
    if(BTN_ANY) {
        shortbeep();
        state = STATE_MAIN;
        initState();
        waitButtonsRelease();
        return;
    }
}

void processScanner()
{
    if(BTN_ANY) {
        if(scan_first_pass) {
            searching = false;
            shortbeep();
            state = STATE_MAIN;
            initState();
            waitButtonsRelease();
            return;
        }
        if(searching) {
            shortbeep();
            show_active_leds = true;
            searching = false;
            updateScannerDialog(_BV(SCANNER_CHOICE));
            SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + scan_best_index));
            waitButtonsRelease();
            return;
        }
        if(BTN_LEFT) { // No
            shortbeep();
            state = STATE_MAIN;
            initState();
            waitButtonsRelease();
            return;
        } else
        if(BTN_RIGHT) { // Yes
            shortbeep();
            config.current_channel = scan_best_index;
            saveSettings = millis()+500;
            state = STATE_MAIN;
            initState();
            waitButtonsRelease();
            return;
        }
    }
    if(searching) {
        channelIndex = pgm_read_byte_near(channelList + scan_channel);
        PORTC = (PORTC & ~0b111000) | (0b1000 << ((anim_count++/2) % NUMBER_OF_RECEIVER));
        SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + channelIndex));
        updateScannerDialog(_BV(SCANNER_MARKER));
        wait(RSSI_STABILIZATION_TIME);
        switchBestRSSI();
        updateScannerDialog(_BV(SCANNER_GRAPH));
        if(max_rssi > scan_max_found) {
            scan_max_found = max_rssi;
            max_rssi_scan_index = scan_channel;
            if(scan_max_found > scan_last_max_found && !scan_first_pass) {
                updateScannerDialog(_BV(SCANNER_BEST));
                scan_best_index = pgm_read_byte_near(channelList + max_rssi_scan_index);
                scan_last_max_found = scan_max_found;
            }
        }
        scan_channel ++;
        if(scan_channel > 39) {
            scan_first_pass = false;
            updateScannerDialog(_BV(SCANNER_BEST));
            scan_best_index = pgm_read_byte_near(channelList + max_rssi_scan_index);
            scan_channel = 0;
            scan_last_max_found = scan_max_found;
            scan_max_found = 0;
        }
    }        
}

void processCalibState()
{
    static uint32_t refresh;
    if(BTN_RIGHT || BTN_LEFT) { // change current channel
        changeChannel();
    } else
    if(BTN_DOWN || BTN_UP) // return to main dialog
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

void processMainMenu()
{
    if(BTN_UP || BTN_DOWN) {
        if(BTN_UP && current_main_menu_item > 0) {
            shortbeep();
            current_main_menu_item --;
            updateMainMenu(_BV(MAIN_MENU_ITEMS));
        } else
        if(BTN_DOWN && current_main_menu_item < MAIN_MENU_ITEMS-1) {
            shortbeep();
            current_main_menu_item ++;
            updateMainMenu(_BV(MAIN_MENU_ITEMS));
        }
        waitButtonsRelease();
    } else
    if(BTN_LEFT || BTN_RIGHT) {
        shortbeep();
        switch(current_main_menu_item) {
            case MAIN_MENU_EXIT:
                state = STATE_MAIN;
                break;
            case MAIN_MENU_LEVELS:
                state = STATE_CALIB;
                break;
            case MAIN_MENU_SCANNER:
                state = STATE_SCANNER;
                break;
            case MAIN_MENU_SETTINGS:
                state = STATE_SETTINGS_MENU;
                break;
        }
        waitButtonsRelease();
        initState();
    }
}

// previous / next channel, long press = auto search
void changeChannel()
{
        shortbeep();
        if(BTN_RIGHT) {
            config.current_channel ++;
            if(config.current_channel > 39) {
                config.current_channel = 0;
            }
        }
        else if(BTN_LEFT) {
            config.current_channel --;
            if(config.current_channel < 0) {
                config.current_channel = 39;
            }
        }
        SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + config.current_channel));
        switch(state) {
            case STATE_MAIN:
                updateMainDialog(_BV(MAIN_BAND) | _BV(MAIN_CHANNEL));
                break;
            case STATE_CALIB:
                updateCalibDialog(_BV(CALIB_HEADER));
                break;
        }
        uint8_t autoRepeat = 4;
        while(BTN_ANY && autoRepeat--) {
            delay(40);
            switchBestRSSI();
        }
        // long press = auto scan
        if(BTN_ANY && max_rssi < config.auto_threshold) {
            searching = true;
            tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
            tft.setTextSize(1);
            switch(state) {
                case STATE_MAIN:
                    tft.setCursor(5,4);
                    break;
                case STATE_CALIB:
                    updateCalibDialog( _BV(CALIB_RESET_BARS) | _BV(CALIB_BARS));
                    tft.fillRect(36, 39, 84 , 16, ST7735_BLACK);
                    tft.drawRect(36, 39, 84 , 16, ST7735_WHITE);
                    tft.setCursor(40, 43);
                    break;
            }
            printText(&misc_string[STRING_SEARCHING]);
            int8_t direction=1;
            if(BTN_LEFT) {
                direction = -1;
            }
            bool button_released=false;
            show_active_leds=false;
            while(!BTN_ANY || !button_released) {
                config.current_channel += direction;
                if(config.current_channel > 39) {
                    config.current_channel = 0;
                }
                if(config.current_channel < 0) {
                    config.current_channel = 39;
                }
                SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + config.current_channel));
                // LEDs animation
                anim_count += direction;
                PORTC = (PORTC & ~0b111000) | (0b1000 << ((anim_count/2) % NUMBER_OF_RECEIVER));
                switch(state) {
                    case STATE_MAIN:
                        updateMainDialog(_BV(MAIN_BAND) | _BV(MAIN_CHANNEL));
                        break;
                    case STATE_CALIB:
                        updateCalibDialog(_BV(CALIB_HEADER));
                        break;
                }
                // let rx stabilize on new frequency
                if((direction == 1 && config.current_channel % 8 == 0) ||
                    direction ==-1 && config.current_channel % 8 == 7) {
                    shortbeep(); // beep on band change
                    wait(RSSI_STABILIZATION_TIME-8);
                    } else {
                    wait(RSSI_STABILIZATION_TIME);
                }
                switchBestRSSI();
                if(state == STATE_CALIB) {
                    updateCalibDialog(_BV(CALIB_BARS) | _BV(CALIB_VALUES));
                }
                // signal found ?
                if(max_rssi >= config.auto_threshold) {
                    // try to avoid false positive
                    wait(RSSI_STABILIZATION_TIME*3);
                    switchBestRSSI();
                    if(max_rssi >= config.auto_threshold) {
                        shortbeep();
                        delay(100);
                        shortbeep();
                        delay(100);
                        break;
                    }                        
                }
                if(!BTN_ANY) {
                    button_released = true;
                }
                alarmBeep();
            }
            if(state == STATE_MAIN) {
                refreshTitle();
            }
            else if(state == STATE_CALIB) {
                tft.fillRect(36, 39, 84 , 16, ST7735_BLACK);
                updateCalibDialog(_BV(CALIB_SCALES));
            }
            shortbeep();
        }
        searching = false;
        waitButtonsRelease();
        saveSettings = millis() + 3000; // save settings 3s after last change
        show_active_leds = true;
}

void processMainState()
{
    if(BTN_UP || BTN_DOWN) { // main menu
        shortbeep();
        state = STATE_MAIN_MENU; // STATE_CALIB;
        initState();
        waitButtonsRelease();
        return;
    } else
    if(BTN_RIGHT || BTN_LEFT) { // change current channel
        changeChannel();
    }
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
    max_rssi = Array[0];
    uint8_t iIndexMax = 0;
    //int16_t hysteresis = analogRead(RSSI_Hysteresis_Pin) >> 5; // 0 - 3.125% hysteresis
    int16_t hysteresis = 0;
    static int16_t last_max = 0;
    static uint8_t last_index = 0;
    for(unsigned int i = 1; i < iSize; i++)
    {
        if(Array[i] > (uint16_t)max_rssi)
        {
            max_rssi = Array[i];
            iIndexMax = i;
        }
    }
    if((max_rssi < last_max - hysteresis) || (max_rssi > last_max + hysteresis)) {
        last_max = max_rssi;
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

// alarm beep management, must be called regularly
void alarmBeep()
{
    if(vbat < config.vbat_alarm) {
        // every 10 seconds
        uint32_t now = millis() % 10000;
        // for 2 seconds
        if(now < 2000) {
            // beep 250ms every 500ms
            if(now%500 < 250) {
                BUZZ_ON;
            } else {
                BUZZ_OFF;
            }
        }
    } else
        BUZZ_OFF;
}

// wait buttons release
void waitButtonsRelease()
{
    uint32_t timeout;
    while(BTN_ANY) {
        timeout = millis()+20;
        while(millis() < timeout)
            switchBestRSSI();
        alarmBeep();
    }
}

// wait or exit if button press
void wait(uint8_t msec)
{
    uint32_t timeout = millis() + msec;
    while(!BTN_ANY && millis() < timeout) {
        ;
    }
}
