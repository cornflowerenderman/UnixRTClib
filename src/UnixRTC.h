/*
  MIT License

  Copyright (c) 2023 William Latter (cornflowerenderman)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

/*
  UnixRTC, an arduino library for the common DS3231 RTC module
  - This library is available from here: https://github.com/cornflowerenderman/UnixRTClib

  This library uses some modified code from the UnixTime library (Available at https://github.com/GyverLibs/UnixTime)
  My modifications include:
    - Date support from Y2000 until Y2199
    - Removal of UTC offset (as we want to store UTC+0 to allow offsets for different timezones and DST)
    - Integration into the RTC functions, for easy use in code
  
*/

#ifndef UnixRTC_H
#define UnixRTC_H

#include "Arduino.h"  //Arduino core libraries
#include "Wire.h"     //Arduino builtin I2C library

#define RTC_1Hz 1
#define RTC_1KHz 1024
#define RTC_4KHz 4096
#define RTC_8KHz 8192

class UnixRTC {  //RTC class
public:
  UnixRTC(void);                                    //Constructor
  void begin();                                     //Initializes I2C bus
  uint64_t getTime();                              //Reads unix time from RTC (with Y2100 correction)
  bool setTime(uint64_t unix);                      //Writes unix time to RTC
  float getTemp(bool force = false);                //Returns the RTC temperature as a float (in deg C)
  int16_t getTempInt(bool force = false);           //Returns the RTC temperature as an int (in x4 deg C)
  int8_t getAgingOffset();                          //Gets current crystal aging offset
  void setAgingOffset(int8_t age = 0);              //Sets crystal aging offset
  bool timeValid();                                 //Returns true if the time is valid
  void assumeTimeValid();                           //Sets the Oscillator stop flag to 0 (used when setting time)
  bool oscillatorEnabled();                         //Checks if the main oscillator is enabled
  void enableOscillator(bool enable = true);        //Enables or disables the oscillator when on battery backup
  void disableOscillator();                         //Same as enableOscillator(false);
  bool output32KHzEnabled();                        //Returns true if the 32KHz output is enabled
  void enable32KHzOut(bool enable = true);          //Enables or disables the 32KHz output
  void disable32KHzOut();                           //Same as enable32KHzOut(false);
  uint64_t getAlarm1Time();                         //Gets the unix time at which Alarm 1 will trip
  void setAlarm1Time(uint64_t unix);                //Changes the time at which Alarm 1 will trip
  bool alm1Tripped(bool clearFlag = false);         //Checks if the flag for Alarm 1 has tripped
  void clearAlm1();                                 //Clears the alarm flag, same as alm1Tripped(true);
  bool alm1InterrptEnabled();                       //Checks if the interrupt for alarm 1 is enabled
  void enableAlm1Interrupt(bool enable = true);     //Enables the alarm 1 interrupt
  void disableAlm1Interrupt();                      //Disables the alarm 1 interrupt
  uint64_t getAlarm2Time();                         //Gets the unix time at which Alarm 2 will trip
  void setAlarm2Time(uint64_t unix);                //Changes the time at which Alarm 2 will trip
  bool alm2Tripped(bool clearFlag = false);         //Checks if the flag for Alarm 2 has tripped
  void clearAlm2();                                 //Clears the alarm flag, same as alm2Tripped(true);
  bool alm2InterrptEnabled();                       //Checks if the interrupt for alarm 2 is enabled
  void enableAlm2Interrupt(bool enable = true);     //Enables the alarm 2 interrupt
  void disableAlm2Interrupt();                      //Disables the alarm 2 interrupt
  uint16_t getSQWFreq();                            //Gets the current SQW frequency in Hz
  bool setSQWFreq(uint16_t freq);                   //Sets the SQW frequency in Hz, returns true on success
  bool batteryBackedSQWEnabled();                   //Returns true if the BBSQW function is enabled
  void enableBatteryBackedSQW(bool enable = true);  //Enables the BBSQW function
  void disableBatteryBackedSQW();                   //Same as enableBatteryBackedSQW(false);
  bool SQWEnabled();                                //Checks the SQW/INT mode (True = SQW, False = INT)
  void enableSQW(bool enable = true);               //Enables the SQW output
  void disableSQW();                                //Same as enableSQW(false);
private:
  uint8_t decToBcd(uint8_t i);                                                                                                                         //Converts decimal to BCD
  uint8_t bcdToDec(uint8_t i);                                                                                                                         //Converts BCD to decimal
  bool afterY2100bug(uint8_t day, uint8_t month, uint8_t year);                                                                                        //Returns true after Feb 28, 2100
  void offsetDate(uint8_t& dayOfWeek, uint8_t& day, uint8_t& month, uint8_t& year);                                                                    //Offsets the date forward 1 day
  void writeRawTime(uint8_t second, uint8_t minute, uint8_t hour, uint8_t dayOfWeek, uint8_t day, uint8_t month, uint8_t year);                        //Used internally for writing to the RTC and Y2100 correction
  uint64_t unixFromDate(uint8_t second, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint8_t year);                                       //Internal conversion for unix time
  void dateFromUnix(uint64_t unix, uint8_t& second, uint8_t& minute, uint8_t& hour, uint8_t& dayOfWeek, uint8_t& day, uint8_t& month, uint8_t& year);  //Internal conversion for unix time
};

#endif
