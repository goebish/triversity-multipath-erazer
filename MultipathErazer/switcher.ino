
void initSwitcher()
{
    //Setting up PI5V331 connection (D5,D6)
    DDRD |= 0b01100000; // output
    select(0);
}

void select(uint8_t source)
{
    static uint8_t SelectedSource = 0;
    // select PI5V Output 
    uint8_t temp = PORTD & 0b10011111;
    PORTD = temp | (source << 5);
    // led for active vRX + all other vRX(s) with same RSSI 
    if(show_active_leds) {
        uint8_t val =  PORTC &= ~0b111000;
        for(uint8_t i=0; i<NUMBER_OF_RECEIVER; i++) {
            if(RSSI_Value[i] == RSSI_Value[source]) {
                val |= 0b1000 << i;
            }
        }
        PORTC = val;
    }
    if(source != SelectedSource) {
        if(millis() > last_switch_beep+100)
            last_switch_beep = millis();
        SelectedSource = source; 
    }       
}
