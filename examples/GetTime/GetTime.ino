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
  uint32_t upper = unixTime/1000000;
  uint32_t lower = unixTime%1000000;
  Serial.print(upper);
  Serial.println(lower,6);
  delay(1000);
}
