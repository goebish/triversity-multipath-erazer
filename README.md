### Building ###

Requires Arduino >= 1.6


PDQ_GFX for ST7735 must be installed in the Arduino libraries folder.

### Usage ###

**Main dialog**


Press left or right to change channel, long press enable auto scan


Press up to enter calibration / bargraph dialog

**Calibration procedure**

Allow the Triversity receiver to warm up for at least 5-10 minutes before calibrating.

1. Start with transmitter switched off
2. Press left / right to manually set frequency to the same as your transmitter
3. Press up to enter calibration / bargraph dialog
4. Set gain potentiometers to their maximum position
5. Adjust zero adjust potentiometers until every channel read the same value in the 50-60 range, lower is better, but they must read the same value, ie 51 51 51
6. Switch transmitter on
7. Adjust gain potentiometer until every channel read 1000
8. Press up to exit calibration dialog

Note that maximum performance is achieved when the main dialog is active, diversity performance may be degraded while operating in other dialogs.

### Contribution guidelines ###

Please report issues or ideas for improvements to [the bug tracker](https://bitbucket.org/goebish/triversity-multipath-erazer/issues?status=new&status=open)