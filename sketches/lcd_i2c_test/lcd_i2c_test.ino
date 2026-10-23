// 16x2 I2C LCD 테스트
// 배선: SDA -> A4, SCL -> A5, VCC -> 5V, GND -> GND
// 화면이 비어 있으면 i2c_scanner로 주소 확인 후 LCD_ADDR 수정, 뒷면 가변저항으로 명암 조절
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define LCD_ADDR 0x27  // 0x3F 인 모듈도 많음
#define LCD_COLS 16
#define LCD_ROWS 2

LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Hello, A2test!");
  lcd.setCursor(0, 1);
  lcd.print("LCD I2C OK");
  Serial.println(F("LCD test"));
  delay(2000);
}

void loop() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Uptime:");
  lcd.setCursor(0, 1);
  lcd.print(millis() / 1000);
  lcd.print(" s");
  delay(1000);
}
