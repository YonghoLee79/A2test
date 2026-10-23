// DHT11 / DHT22 온습도 센서 테스트
// 배선: DATA -> D2, VCC -> 5V, GND -> GND (모듈이 아니면 DATA-VCC 사이 10k 풀업)
#include <DHT.h>

#define DHT_PIN  2
#define DHT_TYPE DHT11  // DHT22 사용 시 DHT22 로 변경

DHT dht(DHT_PIN, DHT_TYPE);

void setup() {
  Serial.begin(9600);
  dht.begin();
  Serial.println(F("DHT test"));
}

void loop() {
  delay(2000);  // DHT11은 1초, DHT22는 2초 이상 간격 필요

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor"));
    return;
  }

  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F(" %  Temp: "));
  Serial.print(t);
  Serial.print(F(" C  HeatIndex: "));
  Serial.print(dht.computeHeatIndex(t, h, false));
  Serial.println(F(" C"));
}
