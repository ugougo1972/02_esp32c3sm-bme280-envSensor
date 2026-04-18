#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SDA_PIN 21
#define SCL_PIN 22

Adafruit_BME280 bme;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);

  Serial.println("BME280 final test start");

  bool ok = bme.begin(0x76, &Wire);
  if (!ok) {
    Serial.println("NG: BME280 init failed");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("OK: BME280 init success");
}

void loop() {
  float tempC = bme.readTemperature();
  float hum   = bme.readHumidity();
  float pres  = bme.readPressure() / 100.0f;  // Pa -> hPa

  if (isnan(tempC) || isnan(hum) || isnan(pres)) {
    Serial.println("NG: read error");
  } else {
    Serial.print("Temp   : ");
    Serial.print(tempC, 2);
    Serial.println(" degC");

    Serial.print("Hum    : ");
    Serial.print(hum, 2);
    Serial.println(" %");

    Serial.print("Press  : ");
    Serial.print(pres, 2);
    Serial.println(" hPa");

    Serial.println("------------------------");
  }

  delay(2000);
}