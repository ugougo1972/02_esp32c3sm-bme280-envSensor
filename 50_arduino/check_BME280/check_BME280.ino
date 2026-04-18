#include <Wire.h>

const uint8_t BME280_ADDR = 0x76;
const uint8_t REG_CHIP_ID = 0xD0;

void setup() {
  Wire.begin(21, 22);   // SDA=21, SCL=22
  Serial.begin(115200);
  delay(1000);

  Serial.println("BME280 Chip ID Read Start");

  Wire.beginTransmission(BME280_ADDR);
  Wire.write(REG_CHIP_ID);
  uint8_t txResult = Wire.endTransmission(false);  // repeated start

  if (txResult != 0) {
    Serial.print("I2C write error: ");
    Serial.println(txResult);
    return;
  }

  uint8_t readCount = Wire.requestFrom(BME280_ADDR, (uint8_t)1);
  if (readCount != 1) {
    Serial.println("I2C read error");
    return;
  }

  uint8_t chipId = Wire.read();

  Serial.print("Chip ID = 0x");
  if (chipId < 0x10) Serial.print("0");
  Serial.println(chipId, HEX);

  if (chipId == 0x60) {
    Serial.println("BME280 detected");
  } else {
    Serial.println("Unexpected chip ID");
  }
}

void loop() {
}