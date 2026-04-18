#include <Wire.h>
#include <Adafruit_LTR390.h>

Adafruit_LTR390 ltr = Adafruit_LTR390();

void setup() {
  Serial.begin(115200);
  delay(1000);

  Serial.println("LTR390 UVS test start");

  Wire.begin(21, 22);
  delay(100);

  if (!ltr.begin()) {
    Serial.println("LTR390 init failed");
    while (1) delay(1000);
  }

  Serial.println("LTR390 init OK");

  ltr.setMode(LTR390_MODE_UVS);
  ltr.setGain(LTR390_GAIN_3);
  ltr.setResolution(LTR390_RESOLUTION_16BIT);

  Serial.println("UVS mode set");
  delay(200);
}

void loop() {
  if (ltr.newDataAvailable()) {
    uint32_t uvs = ltr.readUVS();
    Serial.print("UVS = ");
    Serial.println(uvs);
  } else {
    Serial.println("UVS data not ready");
  }

  delay(200);
}