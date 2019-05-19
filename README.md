# DIY Triversity 5.8 Ghz Video Receiver

## Building

Requires Arduino >= 1.6

PDQ_GFX for ST7735 must be installed in the Arduino libraries folder.

## Usage

### Main dialog


Press left or right to change channel, long press enable auto scan


Press up to enter calibration / bargraph dialog

### Calibration procedure

Allow the Triversity receiver to warm up for at least 5-10 minutes before calibrating.

- Start with transmitter switched off
- Press left / right to manually set frequency to the same as your transmitter
- Press up to enter calibration / bargraph dialog
- Set gain potentiometers to their maximum position
- Adjust zero adjust potentiometers until every channel read the same value in the 50-60 range, lower is better, but they must read the same value, ie 51 51 51
- Switch transmitter on
- Adjust gain potentiometer until every channel read 1000
- Press up to exit calibration dialog
