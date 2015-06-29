
// Define various ADC prescaler
const unsigned char ADC_PS_16 = (1 << ADPS2); // 1 mhz
const unsigned char ADC_PS_32 = (1 << ADPS2) | (1 << ADPS0); // 500 khz
const unsigned char ADC_PS_64 = (1 << ADPS2) | (1 << ADPS1); // 250 khz
const unsigned char ADC_PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0); // arduino default, 125 khz

void setAdcPrescaler(uint8_t prescaler)
{
    ADCSRA &= ~ADC_PS_128;  // remove bits set by Arduino library
    ADCSRA |= prescaler;
}