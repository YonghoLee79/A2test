// I2C 스캐너: LCD I2C 모듈 주소(보통 0x27 또는 0x3F) 확인용
// 배선: SDA -> A4, SCL -> A5
#include <Wire.h>

void setup() {
  Serial.begin(9600);
  Wire.begin();
  Serial.println(F("I2C scanner"));
}

void loop() {
  byte count = 0;
  for (byte addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.print(F("Found device at 0x"));
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      count++;
    }
  }
  if (count == 0) Serial.println(F("No I2C devices found"));
  Serial.println(F("---"));
  delay(3000);
}
