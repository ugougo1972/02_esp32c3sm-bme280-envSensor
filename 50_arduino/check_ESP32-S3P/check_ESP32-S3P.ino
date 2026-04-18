void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("XIAO ESP32S3 Plus boot OK");
}

void loop() {
  static unsigned long last = 0;
  if (millis() - last >= 1000) {
    last = millis();
    Serial.println("heartbeat");
  }
}