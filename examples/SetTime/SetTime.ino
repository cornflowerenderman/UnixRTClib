#include <UnixRTC.h>

UnixRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  Serial.println("RTC Initialized");
  rtc.setTime(1703694200);  //Put the current unix timestamp here
                            //This function also enables the oscillator and clears the clock halt flag
  printUnix(rtc.getTime());
}

void printUnix(uint64_t unixTime) {
  Serial.print("Current unix timestamp: ");
  uint32_t upper = unixTime / 1000000;
  uint32_t lower = unixTime % 1000000;
  Serial.print(upper);
  Serial.print(' ');
  Serial.println(lower);
}

void loop() {
}
