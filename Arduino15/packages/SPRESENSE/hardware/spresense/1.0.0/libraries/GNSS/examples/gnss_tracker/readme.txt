                        How to use gnss_tracker


OVERVIEW:

    The gnss_tracker is a sample sketch that performs GPS positioning by
    intermittent operation.

OPERATION AND PARAMETERS:

    Positioning is performed for the first 300 seconds after setup.
    After that, in each loop processing, it sleeps for <SleepSpec> seconds and
    performs positioning <ActiveSec> seconds.
    The gnss_tracker use <SatelliteSystem> sattelites for positioning.

    Positioning result is notificated in every <IntervalSec> second.
    The result formatted to NMEA will be saved on SD card if the parameter
    NmeaOutFile is TRUE, or/and output to UART if the parameter NmeaOutUart is
    TRUE. NMEA is buffered for each notification. Write at once when ActiveSec
    completes. If SleepSec is set to 0, positioning is performed continuously.

STATUS INDICATION:

    LEDs 0 to 3 shows the following status.

        All : Setup, ON=setup is started, OFF=Setup done.
        LED 0 : Active, Blinks during positioning.
        LED 1 : POSFIX, ON=position fixed.
        LED 2 : File access, ON=File writing.
        LED 3 : Error, ON=Error occurred.

CONFIGURATION FILE AND DEFAULT PARAMETERS:

    Each setting can be changed with "tracker.ini" created on the SD card.
    Ini file contents and initial values of parameters are as follows.

        -------------------------------------------
        ; Satellite system(GPS/GLONASS/ALL)
        SatelliteSystem=ALL
        ; Output NMEA message to UART (TRUE/FALSE)
        NmeaOutUart=TRUE
        ; Output NMEA message to file(TRUE/FALSE)
        NmeaOutFile=TRUE
        ; Output binary data to file(TRUE/FALSE)
        BinaryOut=FALSE
        ; Positioning interval sec(1-300)
        IntervalSec=1
        ; Positioning active sec(60-300)
        ActiveSec=60
        ; Positioning sleep sec(0-240)
        SleepSec=240
        ; Uart debug message(NONE/ERROR/WARNING/INFO)
        UartDebugMessage=None
        ; EOF
        -------------------------------------------

    Parameter ranges or set values are listed in parentheses after the
    description. To change the parameter, please change the "tracker.ini" and
    press the reset button. Parameters read at setup will be output to the UART.
    If a value outside the range is set, it will be replaced with the default
    value or close value.

USAGE:

    * Setup will start in about 10 seconds when the power is turned on.
      (LED 0-3 lights up)
    * Positioning starts after setup is completed. (LED 0 will blink)
    * If conditions are good you can get the position in about 40 seconds.
      (LED 1 will light up)
    * If you can not get the position during the first positioning, please
      keep the condition that the satellite can be seen without moving until
      the LED 1 lights up from the power ON.
