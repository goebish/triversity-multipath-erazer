#include <SPI.h>
#include "pinout.h"
#include "display.h"
#include "logo.h"

#include <PDQ_GFX.h>			// PDQ: Core graphics library
#include <PDQ_ST7735.h>			// PDQ: Hardware-specific driver library

char stringBuffer[30];

void TFT_init_display()
{
    tft.begin();						// initialize LCD
    tft.setRotation(1);
    tft.fillScreen(ST7735_WHITE);
}

inline uint16_t convertColor(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
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
    tft.fillRect( 5, 4, 110, 8, ST7735_BLACK);
    tft.setTextColor(ST7735_WHITE);
    tft.setTextSize(1);
    tft.setCursor(5,4);
    printText(&dialog_title[state]);
}

void locate(uint8_t x,uint8_t y, uint8_t text_size) {
    tft.setCursor( x*6*text_size, y*8*text_size);
}

// print PROGMEM string
void printText(const void* P_str) {
    strcpy_P(stringBuffer, (char*)pgm_read_word(P_str));
    tft.print((char*)stringBuffer);
}

// center PROGMEM string
void centerText(const void* P_str, uint8_t y, uint8_t textSize)
{
    strcpy_P(stringBuffer, (char*)pgm_read_word(P_str));
    int8_t x = (SCREEN_WIDTH/2)-((strlen(stringBuffer)*6*textSize)/2);
    tft.setTextSize(textSize);
    tft.setCursor(x,y);
    tft.print((char*)stringBuffer);
}

void updateMainDialog(uint8_t portion)
{
    uint8_t i;
    tft.setTextColor(ST7735_WHITE);
    if(portion & _BV(MAIN_INIT)) { // frame & static labels
        drawFrame();
        refreshTitle();
        clearFrame();
        tft.setTextSize(2);
        tft.setCursor(10,30);
        printText(&misc_string[STRING_BAND]);
        for(i=0; i<8; i++) {
            tft.setCursor(12 + i*18 , 60);
            tft.print(i+1);
        }
    }

    if(portion & _BV(MAIN_BAND)) { // band name
        tft.setCursor(10+10+12*5, 30);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        printText(&long_band_name[config.current_channel/8]);
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
        printText(&misc_string[STRING_MHZ]);
    }

    if(portion & _BV(MAIN_BATTERY)) { // battery voltage
        static uint8_t last_vbat=0;
        if(vbat != last_vbat || (portion & _BV(MAIN_FORCE_BATTERY_REDRAW))) {
            tft.setCursor(120, 4);
            tft.setTextSize(1);
            if(vbat <= config.vbat_alarm) { 
                tft.setTextColor(ST7735_RED, ST7735_BLACK);
            }
            else if(vbat <= config.vbat_alarm + 4) { 
                tft.setTextColor(ST7735_YELLOW, ST7735_BLACK);
            }
            else { 
                tft.setTextColor(ST7735_GREEN, ST7735_BLACK);
            }
            tft.print((float)vbat/10, 1);
            tft.print(" V");
            last_vbat = vbat;
        }            
    }
}

void updateCalibDialog(uint8_t portion)
{
    #define BARGRAPH_TOP 40
    #define BARGRAPH_HEIGHT 60
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
        portion |= _BV(CALIB_SCALES);
    }
    
    if(portion & _BV(CALIB_SCALES)) {
        for(i=0; i<NUMBER_OF_RECEIVER; i++) {
            tft.drawFastVLine( 16 + i*50, BARGRAPH_TOP, BARGRAPH_HEIGHT, ST7735_WHITE);
            for(uint8_t tick=0; tick<6; tick++) {
                tft.drawFastHLine(15+i*50, (BARGRAPH_TOP-1) + (BARGRAPH_HEIGHT/5)*tick,3,ST7735_WHITE);
            }
        }            
    }
    
    if(portion & _BV(CALIB_RESET_BARS)) {
        for(i=0; i<NUMBER_OF_RECEIVER; i++) {
            RSSI_Value[i] = 0;
        }            
    }
    
    if(portion & _BV(CALIB_HEADER)) {
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(1);
        tft.setCursor(5,4);
        printText(&misc_string[STRING_RSSI]);
        tft.print("    ");
        tft.print((char)short_band_name[config.current_channel/8]);
        tft.print((config.current_channel%8)+1);
        tft.print("  ");
        tft.print(pgm_read_word_near(channelFreqTable + config.current_channel));
    }
    
    if(portion & _BV(CALIB_BARS)) {
        for(i=0; i<NUMBER_OF_RECEIVER; i++) {
            uint8_t height = map(RSSI_Value[i], 0, 1023, 0, BARGRAPH_HEIGHT);
            if(searching == true) { // avoid breaking "searching ..." label
                height = constrain(height,0, BARGRAPH_HEIGHT-20);
            }
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
                 tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
                 tft.setCursor(22+ i*50, 110);
                 tft.print(RSSI_Value[i]);
                 if(RSSI_Value[i] < 100) {
                     tft.print(" ");
                 }
                 if(RSSI_Value[i] < 1000) {
                     tft.print(" ");
                 }
                 RSSI_Previous[i] = RSSI_Value[i];
             }
         }
    }
}

void updateMainMenuDialog(uint8_t portion)
{
    uint8_t i;
    static uint8_t previous_selection = MAIN_MENU_EXIT;
    if(portion & _BV(MAIN_MENU_INIT)) {
        refreshTitle();
        clearFrame();
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(2);
        for (i=0; i<MAIN_MENU_NB_ITEMS; i++) {
            centerText(&main_menu_item[i], 24+i*26, 2);
        }
    }
    
    if(portion & _BV(MAIN_MENU_ITEMS)) {
        tft.setTextSize(2);
        tft.setTextColor(ST7735_WHITE);
        tft.fillRect(6, 22 + previous_selection*26, 147, 19, ST7735_BLACK);
        centerText(&main_menu_item[previous_selection], 24+previous_selection*26, 2);
        tft.setTextColor(ST7735_BLACK);
        tft.fillRect(6, 22 + current_menu_item*26, 147, 19, ST7735_WHITE);
        centerText(&main_menu_item[current_menu_item], 24+current_menu_item*26, 2);
        previous_selection = current_menu_item;
    }
}

void printSettingValue(uint8_t settings_item)
{
    switch(settings_item) {
        case SETTINGS_MENU_VBAT_ALARM:
            tft.print((float)config.vbat_alarm/10,1);
            tft.print(" V ");
            break;
        case SETTINGS_MENU_BEEP_VOLUME:
            printText(&beep_volume_item[config.beep_volume]);
            break;
        case SETTINGS_MENU_PERIOD:
            tft.print(config.switch_period);
            tft.print(" ms ");
            break;
    }
}

void updateSettingsMenuDialog(uint8_t portion)
{
    uint8_t i;
    static uint8_t previous_selection = SETTINGS_MENU_EXIT;
    if(portion & _BV(SETTINGS_MENU_INIT)) {
        refreshTitle();
        clearFrame();
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setTextSize(1);
        centerText(&settings_menu_item[SETTINGS_MENU_EXIT], 24, 1);
        for (i=1; i<SETTINGS_MENU_NB_ITEMS; i++) {
            tft.setCursor(10, 24+i*18);
            printText(&settings_menu_item[i]);
            tft.setCursor(110, 24+i*18);
            printSettingValue(i);
        }
    }
    
    if(portion & _BV(SETTINGS_MENU_ITEMS)) {
        tft.setTextSize(1);
        tft.setTextColor(ST7735_WHITE);
        tft.setCursor(10, 24+previous_selection*18);
        tft.fillRect(8, 22 + previous_selection*18, 144, 11, ST7735_BLACK);
        if(previous_selection == SETTINGS_MENU_EXIT) {
            centerText(&settings_menu_item[SETTINGS_MENU_EXIT], 24, 1);
        } else {
            printText(&settings_menu_item[previous_selection]);
            tft.setCursor(110, 24+previous_selection*18);
            printSettingValue(previous_selection);
        }
        
        tft.setTextColor(ST7735_BLACK);
        tft.setCursor(10, 24 + current_menu_item*18);
        tft.fillRect(8, 22 + current_menu_item*18, 144, 11, ST7735_WHITE);
        if(current_menu_item == SETTINGS_MENU_EXIT) {
            centerText(&settings_menu_item[SETTINGS_MENU_EXIT], 24, 1);
            } else {
            printText(&settings_menu_item[current_menu_item]);
            tft.setCursor(110, 24 + current_menu_item*18);
            printSettingValue(current_menu_item);
        }       
        previous_selection = current_menu_item;
    }
    
    if(portion & _BV(SETTINGS_MENU_CHANGE_SETTING)) {
        tft.setTextColor(ST7735_BLACK, ST7735_WHITE);
        tft.setCursor(110, 24 + current_menu_item*18);
        printSettingValue(current_menu_item);
    }
}

void updateScannerDialog(uint8_t portion) {
    static uint8_t previous_height[40];
    uint8_t i;
    #define BAR_TOP 45
    #define BAR_HEIGHT 60
    #define BAR_WIDTH 2
    #define GRAPH_X 20
    #define GRAPH_WIDTH ((BAR_WIDTH+1)*40)

    if(portion & _BV(SCANNER_INIT)) {
        refreshTitle();
        clearFrame();
        // Y axis
        for(i=0; i<40; i++) {
            previous_height[i] = 0;
        }
        tft.drawFastVLine(GRAPH_X-3, BAR_TOP-1, BAR_HEIGHT, ST7735_WHITE);
        for(i=0; i<6; i++) {
            tft.drawFastHLine(GRAPH_X-4, (BAR_TOP-1) + (BAR_HEIGHT/5)*i,3,ST7735_WHITE);
            tft.drawFastHLine(GRAPH_X-2, (BAR_TOP-1) + (BAR_HEIGHT/5)*i,GRAPH_WIDTH+2,convertColor(80,80,80));
        }
        // X axis
        tft.drawFastHLine(GRAPH_X, BAR_TOP+BAR_HEIGHT+5, GRAPH_WIDTH, ST7735_WHITE);
        for(i=0; i<11; i++) {
            tft.drawFastVLine(GRAPH_X + (GRAPH_WIDTH/10)*i, BAR_TOP+BAR_HEIGHT+4, 3, ST7735_WHITE);
        }
        tft.setTextSize(1);
        tft.setTextColor(ST7735_WHITE);
        tft.setCursor(GRAPH_X-12, BAR_TOP+BAR_HEIGHT+10);
        tft.print(F("5645      5800      5945"));
    }

    if(portion & _BV(SCANNER_MARKER)) {
        // remove previous marker
        tft.fillRect(GRAPH_X + (scan_channel !=0 ? scan_channel-1 : 39)*(BAR_WIDTH+1), BAR_TOP+BAR_HEIGHT+1, 2, BAR_WIDTH,ST7735_BLACK);
        // draw new marker
        tft.fillRect(GRAPH_X + (scan_channel)*(BAR_WIDTH+1), BAR_TOP+BAR_HEIGHT+1,BAR_WIDTH, 2, ST7735_WHITE);
    }

    if(portion & _BV(SCANNER_GRAPH)) {
        uint8_t height = constrain(map(max_rssi, 0, 1000, 0, BAR_HEIGHT)-2,0,BAR_HEIGHT);
        if(height != previous_height[scan_channel]) {
            if(height > previous_height[scan_channel]) {
                tft.fillRect(GRAPH_X + scan_channel*(BAR_WIDTH+1), BAR_TOP + BAR_HEIGHT-height, BAR_WIDTH, height - previous_height[scan_channel], ST7735_WHITE);
            } else {
                tft.fillRect(GRAPH_X + scan_channel*(BAR_WIDTH+1), BAR_TOP + BAR_HEIGHT-previous_height[scan_channel], BAR_WIDTH, previous_height[scan_channel] - height, ST7735_BLACK);
                for(i=0; i<5; i++) {
                    if(BAR_TOP + (BAR_HEIGHT - height) > (BAR_TOP-1) + (BAR_HEIGHT/5)*i) {
                        tft.drawFastHLine(GRAPH_X + scan_channel*(BAR_WIDTH+1), (BAR_TOP-1) + (BAR_HEIGHT/5)*i,BAR_WIDTH,convertColor(80,80,80));
                    }
                }
            }
            previous_height[scan_channel] = height;
        }
    }

    if(portion & _BV(SCANNER_BEST)) {
        tft.setTextSize(1);
        tft.setTextColor(ST7735_WHITE, ST7735_BLACK);
        tft.setCursor(17, 29);
        printText(&misc_string[STRING_BEST]);
        tft.setTextSize(2);
        tft.setCursor(56, 22);
        uint8_t index = pgm_read_byte_near(channelList + max_rssi_scan_index);
        tft.print((char)short_band_name[index/8]);
        tft.print((index%8)+1);
        tft.print(" ");
        tft.print(pgm_read_word_near(channelFreqTable + index));
    }
    
    if(portion & _BV(SCANNER_CHOICE)) {
        tft.fillRect(1, BAR_TOP-1, 158, BAR_HEIGHT+21, ST7735_BLACK);
        tft.drawRect(GRAPH_X-8, BAR_TOP+10, GRAPH_WIDTH+16, 50, ST7735_WHITE);
        tft.drawFastHLine(GRAPH_X-8, BAR_TOP+27, GRAPH_WIDTH+16, ST7735_WHITE);
        tft.setTextColor(ST7735_WHITE);
        centerText(&misc_string[STRING_SELECT_BEST], BAR_TOP + 16, 1);
        centerText(&misc_string[STRING_NO_YES], BAR_TOP + 36, 2);
    }
}

// display 8 bit grayscale RLE compressed bitmap
void showBitmap(const uint8_t* bitmap) {
    uint8_t x=0, y=SCREEN_HEIGHT-1;
    uint8_t* cursor= (uint8_t*)bitmap;
    for(;;) {
        uint8_t len = pgm_read_byte_near(cursor++);
        uint8_t command = pgm_read_byte_near(cursor++);
        if(len != 0) { // draw x consecutive pixels of the same color
            tft.drawFastHLine(x,y,len,convertColor(command,command,command));
            x += len;
        } else {
            // escape command
            if(command == 0x00) { // end of line
                y--;
                x=0;
            } else
            if(command == 0x01) { // end of bitmap
                return;
            } else {
                // single pixels run
                len = command;
                while(len--) {
                    uint8_t color = pgm_read_byte_near(cursor++);
                    tft.drawPixel(x++, y, convertColor(color,color,color));
                }
                if(command & 0x01) { // sequence not aligned on word boundary
                    cursor++; // skip 0 padding
                }
            }
        }
    }
}

void displaySplash()
{
    showBitmap(splash_logo);
    for(uint8_t i=0; i<30; i++) {
        PORTC = (PORTC & ~0b111000) | (0b1000 << (i % 3));
        if(i>26) {
            shortbeep();
        }
        delay(80);
    }
    tft.fillScreen(ST7735_BLACK);
}
