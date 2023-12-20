#include <UnixRTC.h>

UnixRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  Serial.println("RTC Initialized");
  rtc.setTime(1777777777);  // May 03 2026 03:09:37
  delay(3000);
  print64bit(rtc.getTime());
  Serial.println();
  rtc.setTime(4102444799);  // 31st dec 2099 23:59:59
  delay(3000);
  print64bit(rtc.getTime());
  Serial.println();
  rtc.setTime(4107542399);  // Feb 28th Y2100 23:59:59
  delay(3000);
  print64bit(rtc.getTime());
  Serial.println();
}

void loop() {
  print64bit(rtc.getTime());
  Serial.println();
  delay(5000);
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
