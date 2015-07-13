#include "pinout.h"

#define RTC6715_CLK_LOW() PORTD &= ~_BV(VRX_CLK);
#define RTC6715_CLK_HIGH() PORTD |= _BV(VRX_CLK);
#define RTC6715_DATA_LOW() PORTD &= ~_BV(VRX_DATA);
#define RTC6715_DATA_HIGH() PORTD |= _BV(VRX_DATA);
#define RTC6715_EN_LOW() PORTD &= ~_BV(VRX_EN);
#define RTC6715_EN_HIGH() PORTD |= _BV(VRX_EN);

void SPI_vRX_init()
{
    PORTD &= ~(_BV(VRX_CLK)|_BV(VRX_DATA)|_BV(VRX_EN)); // all low
    DDRD |= (_BV(VRX_CLK)|_BV(VRX_DATA)|_BV(VRX_EN)); // outputs 
    delay(100);
    SPI_vRX_set_frequency(pgm_read_word_near(channelFreqTable + config.current_channel));
}

// calc RTC6715 register 0x01 (Synthesizer Register B) value for given frequency in MHz
uint16_t calcSRB(uint16_t freq)
{
    freq -= 479; // IF output frequency (min.479, typ.480)
    uint16_t N = freq>>6;
    uint16_t A = (freq%64)>>1;
    return (N<<7)|A;
}

// following functions are modified versions from
// https://github.com/simonchambers/fs-5.8g-vrx
void SPI_vRX_set_frequency(uint16_t freq)
{
    static uint16_t last_freq = 0;
    if(freq == last_freq)
        return;
    last_freq = freq;
    uint16_t channelData = calcSRB(freq);
    uint8_t i;
    // bit bash out 25 bits of data
    // Order: A0-3, !R/W, D0-D19
    // A0=0, A1=0, A2=0, A3=1, RW=0, D0-19=0
    SERIAL_ENABLE_HIGH();
    delay(2);
    SERIAL_ENABLE_LOW();
    SERIAL_SENDBIT0();
    SERIAL_SENDBIT0();
    SERIAL_SENDBIT0();
    SERIAL_SENDBIT1();
    SERIAL_SENDBIT0();
    // remaining zeros
    for (i=20;i>0;i--)
    SERIAL_SENDBIT0();
    // Clock the data in
    SERIAL_ENABLE_HIGH();
    delay(3);
    SERIAL_ENABLE_LOW();
    // Second is the channel data 
    // 20 bytes of register data are sent, but the MSB 4 bits are zeros
    // register address = 0x1, write, data0-15=channelData data15-19=0x0
    delay(3);
    SERIAL_ENABLE_HIGH();
    delay(3);
    SERIAL_ENABLE_LOW();
    // Register 0x1
    SERIAL_SENDBIT1();
    SERIAL_SENDBIT0();
    SERIAL_SENDBIT0();
    SERIAL_SENDBIT0();
    // Write to register
    SERIAL_SENDBIT1();
    // D0-D15
    // note: loop runs backwards as more efficient on AVR
    for (i=16;i>0;i--)
    {
        // Is bit high or low?
        if (channelData & 0x1)
        {
            SERIAL_SENDBIT1();
        }
        else
        {
            SERIAL_SENDBIT0();
        }
        // Shift bits along to check the next one
        channelData >>= 1;
    }
    // Remaining D16-D19
    for (i=4;i>0;i--)
        SERIAL_SENDBIT0();
    // Finished clocking data in
    SERIAL_ENABLE_HIGH();
    delay(2);
    RTC6715_EN_HIGH();
    RTC6715_CLK_LOW();
    RTC6715_DATA_LOW();
}

void SERIAL_SENDBIT1()
{
    RTC6715_CLK_LOW();
    delayMicroseconds(300);
    RTC6715_DATA_HIGH();
    delayMicroseconds(300);
    RTC6715_CLK_HIGH();
    delayMicroseconds(300);
    RTC6715_CLK_LOW();
    delayMicroseconds(300);
}

void SERIAL_SENDBIT0()
{
    RTC6715_CLK_LOW();
    delayMicroseconds(300);
    RTC6715_DATA_LOW();
    delayMicroseconds(300);
    RTC6715_CLK_HIGH();
    delayMicroseconds(300);
    RTC6715_CLK_LOW();
    delayMicroseconds(300);
}

void SERIAL_ENABLE_LOW()
{
    delayMicroseconds(100);
    RTC6715_EN_LOW();
    delayMicroseconds(100);
}

void SERIAL_ENABLE_HIGH()
{
    delayMicroseconds(100);
    RTC6715_EN_HIGH();
    delayMicroseconds(100);
}
