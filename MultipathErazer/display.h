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
    MAIN_FORCE_BATTERY_REDRAW,
};

enum e_CALIB_DIALOG{
    CALIB_INIT = 0,
    CALIB_VALUES,
    CALIB_BARS,
    CALIB_HEADER,
    CALIB_SCALES,
    CALIB_RESET_BARS,
};

enum e_MAIN_MENU_DIALOG{
    MAIN_MENU_INIT,
    MAIN_MENU_ITEMS,
};

enum e_SETTINGS_MENU_DIALOG{
    SETTINGS_MENU_INIT,
    SETTINGS_MENU_ITEMS,
    SETTINGS_MENU_CHANGE_SETTING,
};

enum e_SCANNER_DIALOG{
    SCANNER_INIT,
    SCANNER_GRAPH,
    SCANNER_MARKER,
    SCANNER_BEST,
    SCANNER_CHOICE,
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

const char short_band_name[] = {
    'A','B','E','F','R'
};

const char long_band_name_str1[] PROGMEM = "A     ";
const char long_band_name_str2[] PROGMEM = "B     ";
const char long_band_name_str3[] PROGMEM = "E     ";
const char long_band_name_str4[] PROGMEM = "IRC/FS";
const char long_band_name_str5[] PROGMEM = "Racer ";

PGM_P const long_band_name[] PROGMEM = {
    long_band_name_str1,
    long_band_name_str2,
    long_band_name_str3,
    long_band_name_str4,
    long_band_name_str5,
};

////////////////////
// dialog titles
// (must be in sync with e_STATES)

const char dialog_title_str1[] PROGMEM = "Channel";
const char dialog_title_str2[] PROGMEM = "Main Menu";
const char dialog_title_str3[] PROGMEM = "RSSI";
const char dialog_title_str4[] PROGMEM = "Band Scanner";
const char dialog_title_str5[] PROGMEM = "Settings";

PGM_P const dialog_title[] PROGMEM = {
    dialog_title_str1,
    dialog_title_str2,
    dialog_title_str3,
    dialog_title_str4,
    dialog_title_str5,
};

///////////////////
// main menu items

const char main_menu_item_str1[] PROGMEM = "Exit";
const char main_menu_item_str2[] PROGMEM = "RSSI Levels";
const char main_menu_item_str3[] PROGMEM = "Band Scanner";
const char main_menu_item_str4[] PROGMEM = "Settings";

PGM_P const main_menu_item[] PROGMEM = {
    main_menu_item_str1,
    main_menu_item_str2,
    main_menu_item_str3,
    main_menu_item_str4,
};

enum e_MAIN_MENU_ITEM{
    MAIN_MENU_EXIT,
    MAIN_MENU_LEVELS,
    MAIN_MENU_SCANNER,
    MAIN_MENU_SETTINGS,
};

#define MAIN_MENU_NB_ITEMS (sizeof(main_menu_item)/sizeof(char *)) //array size

///////////////////
// settings menu items

const char settings_menu_item_str1[] PROGMEM = "Exit";
const char settings_menu_item_str2[] PROGMEM = "Bat Alarm";
const char settings_menu_item_str3[] PROGMEM = "Beep Volume";
const char settings_menu_item_str4[] PROGMEM = "Min Sw Period";
const char settings_menu_item_strR[] PROGMEM = "Factory Reset";

PGM_P const settings_menu_item[] PROGMEM = {
    settings_menu_item_str1,
    settings_menu_item_str2,
    settings_menu_item_str3,
    settings_menu_item_str4,
    settings_menu_item_strR,
};

enum e_SETTINGS_MENU_ITEM {
    SETTINGS_MENU_EXIT,
    SETTINGS_MENU_VBAT_ALARM,
    SETTINGS_MENU_BEEP_VOLUME,
    SETTINGS_MENU_PERIOD,
    SETTINGS_MENU_FACTORY_RESET,    
};

#define SETTINGS_MENU_NB_ITEMS (sizeof(settings_menu_item)/sizeof(char*))

const char beep_volume_str1[] PROGMEM = "Off   ";
const char beep_volume_str2[] PROGMEM = "Quiet ";
const char beep_volume_str3[] PROGMEM = "Loud  ";
const char beep_volume_str4[] PROGMEM = "Louder";

PGM_P const beep_volume_item[] PROGMEM = {
    beep_volume_str1,
    beep_volume_str2,
    beep_volume_str3,
    beep_volume_str4,
};

///////////////////
// misc strings

const char misc_str1[] PROGMEM = "Select best channel ?";
const char misc_str2[] PROGMEM = "\x11No   Yes\x10";
const char misc_str3[] PROGMEM = "Searching ...";
const char misc_str4[] PROGMEM = "Band:";
const char misc_str5[] PROGMEM = " MHz";
const char misc_str6[] PROGMEM = "RSSI";
const char misc_str7[] PROGMEM = "Best:";

PGM_P const misc_string[] PROGMEM = {
    misc_str1,
    misc_str2,
    misc_str3,
    misc_str4,
    misc_str5,
    misc_str6,
    misc_str7,
};

enum e_MISC_STRING{
    STRING_SELECT_BEST,
    STRING_NO_YES,
    STRING_SEARCHING,
    STRING_BAND,
    STRING_MHZ,
    STRING_RSSI,
    STRING_BEST,
};

#endif