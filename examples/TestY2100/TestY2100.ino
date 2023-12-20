#include <UnixRTC.h>

UnixRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  Serial.println("RTC Initialized");
  rtc.setTime(1777777777);  // May 03 2026 03:09:37
  delay(3000);
  printUnix(rtc.getTime());
  rtc.setTime(4102444799);  // 31st dec 2099 23:59:59
  delay(3000);
  printUnix(rtc.getTime());
  rtc.setTime(4107542399);  // Feb 28th Y2100 23:59:59
  delay(3000);
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
  printUnix(rtc.getTime());
  delay(5000);
}
