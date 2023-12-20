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

#include "Wire.h"  //Arduino builtin I2C library

class UnixRTC {  //RTC class
public:
  UnixRTC(void);                  //Constructor
  void begin();                   //Initializes I2C bus
  uint64_t readTime();            //Reads unix time from RTC (with Y2100 correction)
  bool writeTime(uint64_t unix);  //Writes unix time to RTC
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
