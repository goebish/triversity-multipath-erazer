#ifndef __DISPLAY_H_
#define __DISPLAY_H_

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 128

///////////////////
// dialog portions

enum e_MAIN_DIALOG{ // portions for updateMainDialog
    MAIN_INIT = 0,
    MAIN_BAND,
    MAIN_CHANNEL,
    MAIN_BATTERY,
    MAIN_MODE,
};

enum e_CALIB_DIALOG{
    CALIB_INIT = 0,
    CALIB_VALUES,
    CALIB_BARS,
    CALIB_HEADER,
};

enum e_MAIN_MENU_DIALOG{
    MAIN_MENU_INIT,
    MAIN_MENU_ITEMS,
};

/////////////////////
// Bands

enum {
    BAND_A = 0,
    BAND_B,
    BAND_E,
    BAND_IRC,
    BAND_RACER
};

PGM_P const short_band_name[] PROGMEM = {
    "A","B","E","F","R"
};

PGM_P const long_band_name[] PROGMEM = {
    "A     ",
    "B     ",
    "E     ",
    "IRC/FS",
    "Racer "
};

////////////////////
// dialog titles
// (must be in sync with e_STATES)

PGM_P const dialog_title[] PROGMEM = {
    "Channel",
    "Main menu",
    "RSSI",
    "Band scanner",
};

///////////////////
// menu items

PGM_P const main_menu_item[] PROGMEM = {
    "Exit",
    "RSSI Levels",
    "Band scanner",
    "Settings",
};

enum e_MAIN_MENU_ITEM{
    MAIN_MENU_EXIT,
    MAIN_MENU_LEVELS,
    MAIN_MENU_SCANNER,
    MAIN_MENU_SETTINGS,
};

#define MAIN_MENU_ITEMS (sizeof(main_menu_item)/sizeof(char *)) //array size

#endif