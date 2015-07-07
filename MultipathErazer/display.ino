#include <SPI.h>
#include "pinout.h"
#include "display.h"

#include <PDQ_GFX.h>			// PDQ: Core graphics library
#include <PDQ_ST7735.h>			// PDQ: Hardware-specific driver library

void TFT_init_display()
{
    tft.begin();						// initialize LCD
    tft.setRotation(1);
    tft.fillScreen(ST7735_BLACK);
}

void drawFrame()
{
    tft.drawRect(0,0,160,128,ST7735_WHITE);
    tft.drawFastHLine(0, 14, 160, ST7735_WHITE);
}

void clearFrame()
{
    tft.fillRect(1, 15, 158, 111, ST7735_BLACK);
}

void refreshTitle()
{
    tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
    tft.setTextSize(1);
    tft.setCursor(5,4);
    tft.println((char *)pgm_read_word(&(dialog_title[state])));
}

void locate(uint8_t x,uint8_t y, uint8_t text_size) {
    tft.setCursor( x*6*text_size, y*8*text_size);
}

void centerText(uint_farptr_t str, uint8_t y, uint8_t textSize)
{
    int8_t x = (SCREEN_WIDTH/2)-((strlen((PGM_P)str)*6*textSize)/2);
    tft.setTextSize(textSize);
    tft.setCursor(x,y);
    tft.print((char*)str);
}

void updateMainDialog(uint8_t portion)
{
    uint8_t i;
    tft.setTextColor(ST7735_WHITE);
    if(portion & _BV(MAIN_INIT)) { // frame & static labels
        drawFrame();
        clearFrame();
        tft.setTextSize(2);
        tft.setCursor(10,30);
        tft.print(F("Band: "));
        tft.setCursor(10, 45);
        for(i=0; i<8; i++) {
            tft.setCursor(12 + i*18 , 60);
            tft.print(i+1);
        }
    }

    if(portion & _BV(MAIN_BAND)) { // band name
        tft.setCursor(10+10+12*5, 30);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        switch(config.current_channel/8) {
            case BAND_A:
                tft.print(F("A     "));
                break;
            case BAND_B:
                tft.print(F("B     "));
                break;
            case BAND_E:
                tft.print(F("E     "));
                break;
            case BAND_IRC:
                tft.print(F("IRC/FS"));
                break;
            case BAND_RACER:
                tft.print(F("Racer "));
                break;
        }
    }

    if(portion & _BV(MAIN_CHANNEL)) { // channel # + freq
        // draw rectangle around current channel #
        for(i=0; i<8; i++) {
            tft.drawRect(10 + i*18, 58, 14, 18, ST7735_BLACK);
            tft.drawRect( 9 + i*18, 57, 16, 20, ST7735_BLACK);
        }
        tft.drawRect(10 + (config.current_channel%8)*18, 58, 14, 18, ST7735_GREEN);
        tft.drawRect( 9 + (config.current_channel%8)*18, 57, 16, 20, ST7735_GREEN);
        // frequency
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        tft.setCursor(35, 95);
        tft.print(pgm_read_word_near(channelFreqTable + config.current_channel));
        tft.print(F(" MHz"));
    }

    if(portion & _BV(MAIN_BATTERY)) { // battery voltage
        tft.fillRect(120, 4, 36, 7, ST7735_BLACK);
        tft.setCursor(120, 4);
        tft.setTextSize(1);
        if(vbat >= 114) { // > 3.8V / cell
            tft.setTextColor(ST7735_GREEN);
        }
        else if(vbat >= 111) { // > 3.7V / cell
            tft.setTextColor(ST7735_YELLOW);
        }
        else { // < 3.7V / cell
            tft.setTextColor(ST7735_RED);
        }
        tft.print((float)vbat/10, 1);
        tft.print(F(" V"));
    }
}

void updateCalibDialog(uint8_t portion)
{
    static uint8_t previous_height[NUMBER_OF_RECEIVER];
    static uint16_t RSSI_Previous[NUMBER_OF_RECEIVER];
    uint8_t i;
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    if(portion & _BV(CALIB_INIT)) {
        clearFrame();
        for(i=0; i<NUMBER_OF_RECEIVER; i++) {
           previous_height[i] = 0;
           RSSI_Previous[i] = 0;
           tft.setCursor( 18 + i*50, 27);
           tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
           tft.print("vRX");
           tft.print(i+1);
        }
    }
    #define BARGRAPH_TOP 40
    #define BARGRAPH_HEIGHT 60
    if(portion & _BV(CALIB_BARS)) {
        for(i=0; i<NUMBER_OF_RECEIVER; i++) {
            uint8_t height = map(RSSI_Value[i], 0, 1023, 0, BARGRAPH_HEIGHT);
            if(height != previous_height[i]) {
                if(height > previous_height[i]) {
                    tft.fillRect(20 + i*50, BARGRAPH_TOP + BARGRAPH_HEIGHT-height, 20, height - previous_height[i], ST7735_WHITE);
                } else {
                    tft.fillRect(20 + i*50, BARGRAPH_TOP + BARGRAPH_HEIGHT-previous_height[i], 20, previous_height[i] - height, ST7735_BLACK);
                }
                previous_height[i] = height;
            }
        }
    }
    if(portion & _BV(CALIB_VALUES)) {
         for(i=0; i<NUMBER_OF_RECEIVER; i++) {
             if( RSSI_Value[i] != RSSI_Previous[i]) {
                 tft.fillRect(22 + i*50, 110, 24, 7, ST7735_BLACK);
                 tft.setCursor(22+ i*50, 110);
                 tft.print(RSSI_Value[i]);
                 RSSI_Previous[i] = RSSI_Value[i];
             }
         }
    }
}

void updateMainMenu(uint8_t portion)
{
    uint8_t i;
    static uint8_t previous_selection = MAIN_MENU_EXIT;
    if(portion & _BV(MAIN_MENU_INIT)) {
        clearFrame();
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        for (i=0; i<MAIN_MENU_ITEMS; i++) {
            centerText(pgm_read_word(&(main_menu_item[i])), 24+i*26, 2);
        }
    }
    if(portion & _BV(MAIN_MENU_ITEMS)) {
        tft.setTextSize(2);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.fillRect(6, 22 + previous_selection*26, 147, 19, ST7735_BLACK);
        centerText(pgm_read_word(&(main_menu_item[previous_selection])), 24+previous_selection*26, 2);
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
        tft.fillRect(6, 22 + current_main_menu_item*26, 147, 19, ST7735_WHITE);
        centerText(pgm_read_word(&(main_menu_item[current_main_menu_item])), 24+current_main_menu_item*26, 2);
        previous_selection = current_main_menu_item;
    }
}

void displaySplash()
{
    tft.fillScreen(ST7735_BLACK);
    tft.setCursor(40, 25);
    tft.print(F("Goebish"));
    tft.setCursor(60, 40);
    tft.print("&");
    #define LOGO_OFFSET 15
    tft.drawCircle(66, 64+LOGO_OFFSET, 22, ST7735_WHITE);
    tft.drawCircle(66, 64+LOGO_OFFSET, 17, ST7735_WHITE);
    tft.fillRect(0, 58+LOGO_OFFSET , 128, 11, ST7735_BLACK);
    tft.drawFastHLine(15, 57+LOGO_OFFSET, 131, ST7735_WHITE);
    tft.drawFastHLine(15, 69+LOGO_OFFSET, 131, ST7735_WHITE);
    tft.setCursor(15,60+LOGO_OFFSET);
    tft.setTextSize(1);
    tft.print(F("LA FABRIQUE CIRCULAIRE"));
    delay(3000);
    tft.fillScreen(ST7735_BLACK);
    tft.setTextSize(2);
    tft.setTextColor(ST7735_WHITE);
    tft.setCursor((160-12*10)/2, 15);
    tft.print(F("TRIVERSITY"));
    tft.setCursor((160-12*9)/2, 80);
    tft.print(F("Multipath"));
    tft.setCursor((160-12*6)/2, 105);
    tft.print(F("Erazer"));
    for(uint8_t i=0; i<30; i++) {
        PORTC = (PORTC & ~0b111000) | (0b1000 << (i % 3));
        if(i>26) {
            shortbeep();
        }
        delay(80);
    }
}
