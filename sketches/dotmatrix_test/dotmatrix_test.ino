// 8x8 도트 매트릭스(MAX7219 모듈) 테스트
// 배선: DIN -> D11, CLK -> D13, CS -> D10, VCC -> 5V, GND -> GND
#include <LedControl.h>

#define DIN_PIN 11
#define CLK_PIN 13
#define CS_PIN  10

LedControl matrix(DIN_PIN, CLK_PIN, CS_PIN, 1);  // 모듈 1개

const byte SMILE[8] = {
  B00111100,
  B01000010,
  B10100101,
  B10000001,
  B10100101,
  B10011001,
  B01000010,
  B00111100,
};

const byte HEART[8] = {
  B00000000,
  B01100110,
  B11111111,
  B11111111,
  B11111111,
  B01111110,
  B00111100,
  B00011000,
};

void drawBitmap(const byte bitmap[8]) {
  for (byte row = 0; row < 8; row++) {
    matrix.setRow(0, row, bitmap[row]);
  }
}

void setup() {
  Serial.begin(9600);
  matrix.shutdown(0, false);  // 절전 모드 해제
  matrix.setIntensity(0, 4);  // 밝기 0~15
  matrix.clearDisplay(0);
  Serial.println(F("Dot matrix test"));
}

void loop() {
  // 1) 모든 LED 하나씩 켜기 (불량 도트 확인)
  for (byte row = 0; row < 8; row++) {
    for (byte col = 0; col < 8; col++) {
      matrix.setLed(0, row, col, true);
      delay(20);
    }
  }
  delay(300);
  matrix.clearDisplay(0);

  // 2) 그림 표시
  drawBitmap(SMILE);
  delay(1500);
  drawBitmap(HEART);
  delay(1500);
  matrix.clearDisplay(0);
  delay(300);
}
