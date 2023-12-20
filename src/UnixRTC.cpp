#include "UnixRTC.h"

#include "Arduino.h"  //Arduino core libraries

#include "Wire.h"  //Arduino builtin I2C library

UnixRTC::UnixRTC() {}  //Library constructor

void UnixRTC::begin() {
  Wire.begin();  //Begin I2C interface
}

uint64_t UnixRTC::getTime() {   //Returns unix time from RTC
  Wire.beginTransmission(0x68);  //RTC address
  Wire.write(0);                 //Memory address 0
  Wire.endTransmission();        //Ending with 1 written byte, read mode
  Wire.requestFrom(0x68, 7);     //Request 7 bytes (0x00-0x06)
  uint8_t second = bcdToDec(Wire.read() & 0x7F);
  uint8_t minute = bcdToDec(Wire.read() & 0x7F);
  uint8_t rawHour = Wire.read() & 0x7F;
  bool Y2100handled = rawHour & 0x40;  //Has the Y2100 bug already been handled? (Uses the AM/PM flag as memory due to RTC limitations)
  uint8_t hour = bcdToDec(rawHour & 0x3F);
  if (Y2100handled) {  //12H time, convert to 24h (Side effect of using the AM/PM flag as memory)
    bool isPM = rawHour & 0x20;
    hour = bcdToDec(rawHour & 0x1F);
    if (hour > 11) hour = 0;
    if (isPM) hour += 12;
  }
  uint8_t dow = (Wire.read() & 0x07) - 1;      //0-6, 0 being Sunday
  uint8_t day = bcdToDec(Wire.read() & 0x3F);  //Day of month
  uint8_t temp = Wire.read();
  uint8_t month = bcdToDec(temp & 0x1F);  //Month without century bit
  uint8_t year = bcdToDec(Wire.read()) + (temp & 0x80 ? 100 : 0);
  if (afterY2100bug(day, month, year)) {
    if (!Y2100handled) {
      offsetDate(dow, day, month, year);
      writeRawTime(second, minute, hour, dow, day, month, year);  //Updates RTC
    }
  }
  return unixFromDate(second, minute, hour, day, month, year);
}

bool UnixRTC::setTime(uint64_t unix) {
  if (unix < 946684800) {  //Time cannot be less than Y2000 (RTC limitation & time can't go backwards)
    return false;
  }
  if (unix >= 7258118400) {  //Time cannot be greater than Y2199 (RTC limitation)
    return false;
  }
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t dayOfWeek;
  uint8_t day;
  uint8_t month;
  uint8_t year;
  dateFromUnix(unix, second, minute, hour, dayOfWeek, day, month, year);  //Splits unix time into smaller date parts
  writeRawTime(second, minute, hour, dayOfWeek, day, month, year);        //Writes time to RTC
  assumeTimeValid();
  enableOscillator();
  return true;
}

uint8_t UnixRTC::decToBcd(uint8_t i) {  //Converts decimal to RTC BCD format
  return ((i / 10) << 4) | (i % 10);
}
uint8_t UnixRTC::bcdToDec(uint8_t i) {  //Converts RTC BCD format to decimal
  return (i & 0xF) + ((i >> 4) * 10);
}
bool UnixRTC::afterY2100bug(uint8_t day, uint8_t month, uint8_t year) {  //Returns true if the time is after Feb 29, 2100
  if (year > 100) return true;                                           //After Y2100
  else if (year == 100) {
    if (month > 2 || (month == 2 && day == 29)) return true;  //After Feb 29, 2100
  }
  return false;  //Before Feb 29, 2100
}
void UnixRTC::offsetDate(uint8_t& dow, uint8_t& day, uint8_t& month, uint8_t& year) {  //Year as 00-199
  uint8_t daysInMonths[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
  if ((year % 4) == 0) {
    daysInMonths[1] = 29;
  }
  day++;
  if (day > daysInMonths[month - 1]) {
    day = 1;
    month++;
    if (month > 12) {
      month = 1;
      year++;
    }
  }
  dow++;
  if (dow >= 7) dow = 0;
}

void UnixRTC::writeRawTime(uint8_t sec, uint8_t min, uint8_t hr, uint8_t dow, uint8_t day, uint8_t month, uint8_t year) {  //Used internally for Y2100 correction on read and writing
  Wire.beginTransmission(0x68);
  Wire.write(0);              //Memory address 0
  Wire.write(decToBcd(sec));  //Writes second, removes Clock Halt on DS1307
  Wire.write(decToBcd(min));
  bool mode = afterY2100bug(day, month, year);
  if (mode) {  //After Feb 2100, 12h
    bool isPM = hr > 11;
    if (isPM) hr -= 12;
    if (hr == 0) hr = 12;
    Wire.write(decToBcd(hr) | (isPM ? 0x60 : 0x40));
  } else {  //Before Feb 2100, 24h
    Wire.write(decToBcd(hr));
  }
  Wire.write(dow + 1);  //Never exceeds 15 so BCD encoding is unnessecary (0-6)
  Wire.write(decToBcd(day));
  Wire.write(decToBcd(month) | (year > 99 ? 0x80 : 0));  //Month with century bit
  Wire.write(decToBcd(year % 100));
  Wire.endTransmission();
}

uint64_t UnixRTC::unixFromDate(uint8_t second, uint8_t minute, uint8_t hour, uint8_t day, uint8_t month, uint8_t year) {  //Internal conversion for unix time
  uint8_t my = (month >= 3) ? 1 : 0;
  uint16_t y = year + 30 + my;
  uint16_t dm = 0;
  for (int i = 0; i < month - 1; i++) dm += (i < 7) ? ((i == 1) ? 28 : ((i & 1) ? 30 : 31)) : ((i & 1) ? 31 : 30);
  return ((((day - 1 + dm + ((y + 1) / 4) - ((y + 69) / 100) + ((y + 369) / 400)) + (365UL * (y - my))) * (uint32_t)24 + hour) * (uint32_t)60 + minute) * (uint64_t)60 + second;
}

void UnixRTC::dateFromUnix(uint64_t unix, uint8_t& second, uint8_t& minute, uint8_t& hour, uint8_t& dayOfWeek, uint8_t& day, uint8_t& month, uint8_t& year) {  //Internal conversion for unix time
  second = unix % 60;
  uint32_t t = unix / 60;
  minute = t % 60;
  t /= 60;
  hour = t % 24;
  t /= 24;
  dayOfWeek = (t + 4) % 7;
  uint32_t z = t + 719468;
  uint8_t era = z / 146097ul;
  uint32_t doe = z - era * 146097ul;
  uint32_t yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
  uint32_t y = yoe + era * 400;
  uint32_t doy = doe - (yoe * 365 + yoe / 4 - yoe / 100);
  uint32_t mp = (doy * 5 + 2) / 153;
  day = doy - (mp * 153 + 2) / 5 + 1;
  month = mp + (mp < 10 ? 3 : -9);
  y += (month <= 2);
  year = y - 2000;
}

int8_t UnixRTC::getAgingOffset() {
  Wire.beginTransmission(0x68);
  Wire.write(0x10);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read();
}

void UnixRTC::setAgingOffset(int8_t age = 0) {
  Wire.beginTransmission(0x68);
  Wire.write(0x10);
  Wire.write(age);
  Wire.endTransmission();
}

int16_t UnixRTC::getTempInt(bool force = false) {
  if (force) {
    Wire.beginTransmission(0x68);
    Wire.write(0xE);
    Wire.endTransmission();
    Wire.requestFrom(0x68, 2);
    uint8_t oldControl = Wire.read();
    bool isBusy = Wire.read() & 0x4;
    if (!isBusy) {
      Wire.beginTransmission(0x68);
      Wire.write(0xE);
      Wire.write(oldControl | 0x20);
      Wire.endTransmission();
    }
    for (int a = 0; a < 30; a++) {  //Timeout after 30 busy checks
      Wire.beginTransmission(0x68);
      Wire.write(0xE);
      Wire.endTransmission();
      Wire.requestFrom(0x68, 1);
      if (!(Wire.read() & 0x20)) break;
      delay(50);
    }
  }
  Wire.beginTransmission(0x68);
  Wire.write(0x11);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 2);
  int8_t tempMSB = Wire.read();
  uint8_t tempLSB = Wire.read() >> 6;
  int16_t temp = (tempMSB * 4) | tempLSB;
  return temp;
}

float UnixRTC::getTemp(bool force = false) {
  return getTempInt(force) / 4.0;
}

bool UnixRTC::timeValid() {
  Wire.beginTransmission(0x68);
  Wire.write(0xF);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return !(Wire.read() & 0x80);
}

void UnixRTC::assumeTimeValid() {
  Wire.beginTransmission(0x68);
  Wire.write(0xF);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t status = Wire.read();
  if (status & 0x80) {  // Oscillator stopped
    Wire.beginTransmission(0x68);
    Wire.write(0xF);
    Wire.write(status & 0x7F);
    Wire.endTransmission();
  }
}

bool UnixRTC::oscillatorEnabled() {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read() & 0x80;
}

void UnixRTC::enableOscillator(bool enable = true) {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t control = Wire.read();
  uint8_t newControl = control;
  if (enable) {
    newControl |= 0x80;
  } else {
    newControl &= 0x7F;
  }
  if (newControl != control) {
    Wire.beginTransmission(0x68);
    Wire.write(0xE);
    Wire.write(newControl);
    Wire.endTransmission();
  }
}
void UnixRTC::disableOscillator() {
  enableOscillator(false);
}

bool UnixRTC::output32KHzEnabled() {
  Wire.beginTransmission(0x68);
  Wire.write(0xF);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read() & 0x8;
}

void UnixRTC::enable32KHzOut(bool enable = true) {
  Wire.beginTransmission(0x68);
  Wire.write(0xF);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t status = Wire.read();
  uint8_t newStatus = status;
  if (enable) {
    newStatus |= 0x8;
  } else {
    newStatus &= 0x87;
  }
  if (newStatus != status) {
    Wire.beginTransmission(0x68);
    Wire.write(0xF);
    Wire.write(newStatus);
    Wire.endTransmission();
  }
}
void UnixRTC::disable32KHzOut() {
  enable32KHzOut(false);
}

uint64_t UnixRTC::getAlarm1Time() {  //This function assumes the alarm was set by this library, for simplicity
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 7);
  uint8_t second = bcdToDec(Wire.read());
  uint8_t minute = bcdToDec(Wire.read());
  uint8_t rawHour = Wire.read() & 0x7F;
  bool Y2100handled = rawHour & 0x40;  //Has the Y2100 bug already been handled? (Uses the AM/PM flag as memory due to RTC limitations)
  uint8_t hour = bcdToDec(rawHour & 0x3F);
  if (Y2100handled) {  //12H time, convert to 24h (Side effect of using the AM/PM flag as memory)
    bool isPM = rawHour & 0x20;
    hour = bcdToDec(rawHour & 0x1F);
    if (hour > 11) hour = 0;
    if (isPM) hour += 12;
  }
  Wire.read();  //DoW not needed
  uint8_t day = bcdToDec(Wire.read());
  uint8_t month = Wire.read();           //Gets current month
  uint8_t year = bcdToDec(Wire.read());  //Gets current year
  if (month & 0x80) year += 100;
  month = bcdToDec(month & 0x1F);
  Wire.beginTransmission(0x68);
  Wire.write(0x07);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 4);
  uint8_t almSecond = bcdToDec(Wire.read() & 0x7F);
  uint8_t almMinute = bcdToDec(Wire.read() & 0x7F);
  uint8_t almHour = bcdToDec(Wire.read() & 0x3F);
  uint8_t almDay = bcdToDec(Wire.read() & 0x3F);

  uint64_t now = unixFromDate(second, minute, hour, day, month, year);
  uint64_t alm = unixFromDate(almSecond, almMinute, almHour, almDay, month, year);
  if (alm < now) {
    month++;
    if (month > 12) {
      month = 0;
      year++;
    }
    alm = unixFromDate(almSecond, almMinute, almHour, almDay, month, year);
  }
  return alm;
}
void UnixRTC::setAlarm1Time(uint64_t unix) {
  uint8_t second;
  uint8_t minute;
  uint8_t hour;
  uint8_t dayOfWeek;  //not used
  uint8_t day;
  uint8_t month;  //not used
  uint8_t year;   //not used
  dateFromUnix(unix, second, minute, hour, dayOfWeek, day, month, year);
  Wire.beginTransmission(0x68);
  Wire.write(0x7);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(day));
  Wire.endTransmission();
}

bool UnixRTC::alm1Tripped(bool clearFlag = false) {
  Wire.beginTransmission(0x68);
  Wire.write(0xF);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t status = Wire.read();
  bool tripped = status & 0x01;
  if (clearFlag && tripped) {
    status &= 0xFE;
    Wire.beginTransmission(0x68);
    Wire.write(0xF);
    Wire.write(status);
    Wire.endTransmission();
  }
  return tripped;
}

void UnixRTC::clearAlm1() {
  alm1Tripped(true);
}

bool UnixRTC::alm1InterrptEnabled() {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read() & 0x01;
}

void UnixRTC::enableAlm1Interrupt(bool enable = true) {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t control = Wire.read();
  uint8_t newControl = control;
  if (enable) {
    newControl |= 1;
  } else {
    newControl &= 0xFE;
  }
  if (newControl != control) {
    Wire.beginTransmission(0x68);
    Wire.write(0xE);
    Wire.write(newControl);
    Wire.endTransmission();
  }
}

void UnixRTC::disableAlm1Interrupt() {
  enableAlm1Interrupt(false);
}

uint64_t UnixRTC::getAlarm2Time() {  //This function assumes the alarm was set by this library, for simplicity
  Wire.beginTransmission(0x68);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 7);
  uint8_t second = bcdToDec(Wire.read());
  uint8_t minute = bcdToDec(Wire.read());
  uint8_t rawHour = Wire.read() & 0x7F;
  bool Y2100handled = rawHour & 0x40;  //Has the Y2100 bug already been handled? (Uses the AM/PM flag as memory due to RTC limitations)
  uint8_t hour = bcdToDec(rawHour & 0x3F);
  if (Y2100handled) {  //12H time, convert to 24h (Side effect of using the AM/PM flag as memory)
    bool isPM = rawHour & 0x20;
    hour = bcdToDec(rawHour & 0x1F);
    if (hour > 11) hour = 0;
    if (isPM) hour += 12;
  }
  Wire.read();  //DoW not needed
  uint8_t day = bcdToDec(Wire.read());
  uint8_t month = Wire.read();           //Gets current month
  uint8_t year = bcdToDec(Wire.read());  //Gets current year
  if (month & 0x80) year += 100;
  month = bcdToDec(month & 0x1F);
  Wire.beginTransmission(0x68);
  Wire.write(0x0B);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 3);
  uint8_t almMinute = bcdToDec(Wire.read() & 0x7F);
  uint8_t almHour = bcdToDec(Wire.read() & 0x3F);
  uint8_t almDay = bcdToDec(Wire.read() & 0x3F);
  uint64_t now = unixFromDate(second, minute, hour, day, month, year);
  uint64_t alm = unixFromDate(0, almMinute, almHour, almDay, month, year);
  if (alm < now) {
    month++;
    if (month > 12) {
      month = 0;
      year++;
    }
    alm = unixFromDate(0, almMinute, almHour, almDay, month, year);
  }
  return alm;
}

void UnixRTC::setAlarm2Time(uint64_t unix) {
  uint8_t second;  //not used
  uint8_t minute;
  uint8_t hour;
  uint8_t dayOfWeek;  //not used
  uint8_t day;
  uint8_t month;  //not used
  uint8_t year;   //not used
  dateFromUnix(unix, second, minute, hour, dayOfWeek, day, month, year);
  Wire.beginTransmission(0x68);
  Wire.write(0xB);
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(day));
  Wire.endTransmission();
}

bool UnixRTC::alm2Tripped(bool clearFlag = false) {
  Wire.beginTransmission(0x68);
  Wire.write(0xF);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t status = Wire.read();
  bool tripped = status & 0x02;
  if (clearFlag && tripped) {
    status &= 0xFD;
    Wire.beginTransmission(0x68);
    Wire.write(0xF);
    Wire.write(status);
    Wire.endTransmission();
  }
  return tripped;
}
void UnixRTC::clearAlm2() {
  alm2Tripped(true);
}

bool UnixRTC::alm2InterrptEnabled() {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read() & 0x02;
}

void UnixRTC::enableAlm2Interrupt(bool enable = true) {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t control = Wire.read();
  uint8_t newControl = control;
  if (enable) {
    newControl |= 0x02;
  } else {
    newControl &= 0xFD;
  }
  if (newControl != control) {
    Wire.beginTransmission(0x68);
    Wire.write(0xE);
    Wire.write(newControl);
    Wire.endTransmission();
  }
}

void UnixRTC::disableAlm2Interrupt() {
  enableAlm2Interrupt(false);
}

uint16_t UnixRTC::getSQWFreq() {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t rawFreq = (Wire.read() >> 3) & 0x3;
  switch (rawFreq) {
    case 0:
      return 1;
    case 1:
      return 1024;
    case 2:
      return 4096;
    default:
      return 8192;
  }
}

bool UnixRTC::setSQWFreq(uint16_t freq) {
  uint8_t freqBits = 0;
  switch (freq) {
    case 1:
      freqBits = 0;
      break;
    case 1024:
      freqBits = 1;
      break;
    case 4096:
      freqBits = 2;
      break;
    case 8192:
      freqBits = 3;
      break;
    default:
      return false;
  }
  freqBits <<= 3;
  Wire.beginTransmission(0x68);
  Wire.write(0x0E);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t oldControl = Wire.read();
  freqBits |= (oldControl & 0xE7);
  if (freqBits != oldControl) {
    Wire.beginTransmission(0x68);
    Wire.write(0x0E);
    Wire.write(freqBits);
    Wire.endTransmission();
  }
  return true;
}

bool UnixRTC::batteryBackedSQWEnabled() {
  Wire.beginTransmission(0x68);
  Wire.write(0x0E);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read() & 0x40;
}

void UnixRTC::enableBatteryBackedSQW(bool enable = true) {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t control = Wire.read();
  uint8_t newControl = control;
  if (enable) {
    newControl |= 0x40;
  } else {
    newControl &= 0xBF;
  }
  if (newControl != control) {
    Wire.beginTransmission(0x68);
    Wire.write(0xE);
    Wire.write(newControl);
    Wire.endTransmission();
  }
}
void UnixRTC::disableBatteryBackedSQW() {
  enableBatteryBackedSQW(false);
}

bool UnixRTC::SQWEnabled() {
  Wire.beginTransmission(0x68);
  Wire.write(0x0E);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  return Wire.read() & 0x4;
}

void UnixRTC::enableSQW(bool enable = true) {
  Wire.beginTransmission(0x68);
  Wire.write(0xE);
  Wire.endTransmission();
  Wire.requestFrom(0x68, 1);
  uint8_t control = Wire.read();
  uint8_t newControl = control;
  if (enable) {
    newControl |= 0x4;
  } else {
    newControl &= 0xFB;
  }
  if (newControl != control) {
    Wire.beginTransmission(0x68);
    Wire.write(0xE);
    Wire.write(newControl);
    Wire.endTransmission();
  }
}
void UnixRTC::disableSQW() {
  enableSQW(false);
}
