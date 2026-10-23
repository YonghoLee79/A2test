// 통합 테스트: 모든 센서/부품을 동시에 연결해서 동작 확인
//   DHT      -> LCD 1번째 줄 (온도/습도)
//   HC-SR04  -> LCD 2번째 줄 + RGB LED 색 (가까움=빨강, 중간=초록, 멀리=파랑)
//   가변저항 -> 서보 각도
//   CdS 조도 -> 도트 매트릭스 막대 그래프
// 핀 배치는 README.md 의 핀 맵 참고
#include <DHT.h>
#include <Servo.h>
#include <LedControl.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DHT_PIN   2
#define DHT_TYPE  DHT11  // DHT22 사용 시 변경
#define RED_PIN   3
#define GREEN_PIN 5
#define BLUE_PIN  6
#define TRIG_PIN  7
#define ECHO_PIN  8
#define SERVO_PIN 9
#define CS_PIN    10
#define DIN_PIN   11
#define CLK_PIN   13
#define LIGHT_PIN A0
#define POT_PIN   A1
#define LCD_ADDR  0x27

const bool COMMON_ANODE = false;
const unsigned long ECHO_TIMEOUT_US = 30000;

DHT dht(DHT_PIN, DHT_TYPE);
Servo servo;
LedControl matrix(DIN_PIN, CLK_PIN, CS_PIN, 1);
LiquidCrystal_I2C lcd(LCD_ADDR, 16, 2);

float temperature = NAN;
float humidity = NAN;
float distanceCm = -1;
int light = 0;
int pot = 0;

unsigned long lastDht = 0;
unsigned long lastFast = 0;
unsigned long lastPrint = 0;

void setColor(byte r, byte g, byte b) {
  if (COMMON_ANODE) {
    r = 255 - r;
    g = 255 - g;
    b = 255 - b;
  }
  analogWrite(RED_PIN, r);
  analogWrite(GREEN_PIN, g);
  analogWrite(BLUE_PIN, b);
}

float readDistanceCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  unsigned long duration = pulseIn(ECHO_PIN, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) return -1;
  return duration * 0.0343 / 2.0;
}

void updateRgb() {
  if (distanceCm < 0)        setColor(0, 0, 0);
  else if (distanceCm < 10)  setColor(255, 0, 0);
  else if (distanceCm < 30)  setColor(0, 255, 0);
  else                       setColor(0, 0, 255);
}

void updateMatrix() {
  // 밝을수록 아래에서부터 더 많은 줄이 켜짐 (0~8줄)
  byte rows = map(light, 0, 1023, 0, 8);
  for (byte row = 0; row < 8; row++) {
    matrix.setRow(0, 7 - row, row < rows ? B11111111 : B00000000);
  }
}

void updateLcd() {
  char line[17];
  char tBuf[6], hBuf[6], dBuf[6];

  if (isnan(temperature)) {
    snprintf(line, sizeof(line), "T: --  H: --    ");
  } else {
    dtostrf(temperature, 4, 1, tBuf);
    dtostrf(humidity, 4, 1, hBuf);
    snprintf(line, sizeof(line), "T:%sC H:%s%%  ", tBuf, hBuf);
  }
  lcd.setCursor(0, 0);
  lcd.print(line);

  if (distanceCm < 0) {
    snprintf(line, sizeof(line), "D: ---  L:%4d  ", light);
  } else {
    dtostrf(distanceCm, 5, 1, dBuf);
    snprintf(line, sizeof(line), "D:%scm L:%4d", dBuf, light);
  }
  lcd.setCursor(0, 1);
  lcd.print(line);
}

void printSerial() {
  Serial.print(F("T="));
  Serial.print(temperature);
  Serial.print(F("C H="));
  Serial.print(humidity);
  Serial.print(F("% D="));
  Serial.print(distanceCm);
  Serial.print(F("cm light="));
  Serial.print(light);
  Serial.print(F(" pot="));
  Serial.print(pot);
  Serial.print(F(" servo="));
  Serial.println(servo.read());
}

void setup() {
  Serial.begin(9600);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  dht.begin();
  servo.attach(SERVO_PIN);

  matrix.shutdown(0, false);
  matrix.setIntensity(0, 4);
  matrix.clearDisplay(0);

  lcd.init();
  lcd.backlight();
  lcd.print("A2test all");
  delay(1000);
  lcd.clear();

  Serial.println(F("All sensors test"));
}

void loop() {
  unsigned long now = millis();

  if (now - lastDht >= 2000) {
    lastDht = now;
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    if (!isnan(h) && !isnan(t)) {
      humidity = h;
      temperature = t;
    }
  }

  if (now - lastFast >= 100) {
    lastFast = now;
    distanceCm = readDistanceCm();
    light = analogRead(LIGHT_PIN);
    pot = analogRead(POT_PIN);

    servo.write(map(pot, 0, 1023, 0, 180));
    updateRgb();
    updateMatrix();
  }

  if (now - lastPrint >= 500) {
    lastPrint = now;
    updateLcd();
    printSerial();
  }
}
