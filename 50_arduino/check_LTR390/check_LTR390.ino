#include <Wire.h>
#include <Adafruit_LTR390.h>

Adafruit_LTR390 ltr = Adafruit_LTR390();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("LTR390 ALS test start");

  Wire.begin(21, 22);
  delay(100);

  if (!ltr.begin()) {
    Serial.println("LTR390 init failed");
    while (1) delay(1000);
  }

  Serial.println("LTR390 init OK");

  ltr.setMode(LTR390_MODE_ALS);
  ltr.setGain(LTR390_GAIN_18);
  ltr.setResolution(LTR390_RESOLUTION_16BIT);

  delay(200);
}

void loop() {
  if (ltr.newDataAvailable()) {
    uint32_t als = ltr.readALS();
    Serial.print("ALS = ");
    Serial.println(als);
  } else {
    Serial.println("ALS data not ready");
  }

  delay(500);
}