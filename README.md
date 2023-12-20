# UnixRTClib
An arduino library for interfacing with the DS3231 RTC module with built-in unix time support and Y2100 leap year bug mitigation
## Features
- Unix timestamps in timekeeping functions, for easy integration with DST offsets and NTP
- Timekeeping from Y2000 to Y2199, with mitigations in place for Y2100 leap year bug and Y2106 32bit overflow
- Architecture independent (uses built-in libraries for I2C communication)
- Minimal dependencies (just the built-in arduino libraries) 
## Planned features
- Getting/Setting RTC alarms
- Getting/Setting control and status bits
- Ability to adjust crystal aging offset
- RTC temperature reading
- Support for DS1307 including Y2100 workarounds
### About
Credit to https://github.com/GyverLibs/UnixTime for the UnixTime library, which has been modified for the time conversion to and from unix time.

The remainder of the code has been created by myself, using the datasheet for the DS3231 RTC. (available here: https://www.analog.com/media/en/technical-documentation/data-sheets/DS3231.pdf)
