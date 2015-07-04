// global pinout

// TFT LCD
#define TFT_CS (uint8_t)    10
#define TFT_RST (uint8_t)   A7   // connected to +3.3
#define TFT_DC (uint8_t)    9
#define TFT_SCLK (uint8_t)  13
#define TFT_DATA (uint8_t)  11

// buttons
#define BTN_UP_PIN  (uint8_t)   7
#define BTN_RIGHT_PIN (uint8_t) 0
#define BTN_DOWN_PIN (uint8_t)  12
#define BTN_LEFT_PIN (uint8_t)  1

#define BTN_UP      (!(PIND & 0b10000000))
#define BTN_RIGHT   (!(PIND & 0b00000001))
#define BTN_DOWN    (!(PINB & 0b00010000))
#define BTN_LEFT    (!(PIND & 0b00000010))
#define BTN_ANY     (BTN_UP|BTN_RIGHT|BTN_LEFT|BTN_DOWN)


// SPI vRX
#define VRX_DATA (uint8_t)  2 // PD2 - CH1 
#define VRX_EN (uint8_t)    3 // PD3 - CH2
#define VRX_CLK (uint8_t)   4 // PD4 - CH3

// RSSI vRX
#define RSSI_1 (uint8_t)    A2
#define RSSI_2 (uint8_t)    A1
#define RSSI_3 (uint8_t)    A0 

// LEDs
#define LED_VRX_1 (uint8_t) A3 
#define LED_VRX_2 (uint8_t) A4 
#define LED_VRX_3 (uint8_t) A5 

// analog multiplexer control
#define PI5V311_IN0 (uint8_t) 5 // PD5
#define PI5V311_IN1 (uint8_t) 6 // PD6

// misc
#define BUZZER (uint8_t)    8   // PB0
#define BUZZ_OFF PORTB &= ~0b00000001
#define BUZZ_ON  PORTB |=  0b00000001

#define VBAT (uint8_t)      A6
