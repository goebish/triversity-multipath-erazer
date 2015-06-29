
void initSwitcher()
{
    //Setting up PI5V331 connection (D5,D6)
    DDRD |= 0b01100000; // output
    select(0);
}

void select(uint8_t source)
{
    // select PI5V Output 
    uint8_t temp = PORTD & 0b10011111;
    PORTD = temp | (source << 5); 
    // led for active vRX
    /*uint8_t val =  PORTC &= ~0b111000;
    val |= 0b1000 << source;
    PORTC = val;*/
    PORTC = (PORTC & ~0b111000) | (0b1000 << source);
    SelectedSource = source;
}
