#include <UnixRTC.h>

UnixRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  Serial.println("RTC Initialized");
  if (rtc.timeValid()) {
    if (rtc.oscillatorEnabled()) {
      Serial.println("RTC is valid and oscillator is enabled");
    } else {
      Serial.println("RTC is valid but will lose time when power is removed!");
      Serial.println("Set the time or run rtc.enableOscillator() to fix this");
    }
  } else {
    Serial.println("RTC time is not valid!");
    Serial.println("Use the SetTime example to set the RTC time");
    while (true) {};
  }
}

void loop() {
  uint64_t unixTime = rtc.getTime();
  Serial.print("Current unix timestamp: ");
  print64bit(unixTime);
  Serial.println();
  delay(1000);
}

void print64bit(uint64_t number) {
  if (number == 0) {
    Serial.print('0');
    return;
  }
  int8_t buffer[20];
  uint8_t len = 0;
  while (number > 0) {
    uint64_t t = number / 10;
    buffer[len++] = number - t * 10 + '0';
    number = t;
  }
  for (; len > 0; len--) Serial.print((char)buffer[len - 1]);
}
