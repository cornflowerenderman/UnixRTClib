#include <UnixRTC.h>

UnixRTC rtc;

void setup() {
  Serial.begin(115200);
  rtc.begin();
  Serial.println("RTC Initialized");
  rtc.setTime(1703694200);  //Put the current unix timestamp here
                            //This function also enables the oscillator and clears the clock halt flag
  print64bit(rtc.getTime());
  Serial.println();
}

void loop() {
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
