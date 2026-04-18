#include <Wire.h>
#include <Adafruit_BME280.h>
#include <Adafruit_LTR390.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_BME280 bme;
Adafruit_LTR390 ltr = Adafruit_LTR390();
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

bool loggingEnabled = true;
int batteryPercent = 100;

void drawBatteryIcon(int x, int y, int percent) {
  if (percent < 0) percent = 0;
  if (percent > 100) percent = 100;

  display.drawRect(x, y, 14, 8, SSD1306_WHITE);
  display.fillRect(x + 14, y + 2, 2, 4, SSD1306_WHITE);

  int fillW = map(percent, 0, 100, 0, 10);
  if (fillW > 0) {
    display.fillRect(x + 2, y + 2, fillW, 4, SSD1306_WHITE);
  }
}

void drawHeader(unsigned long sec) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 1);
  display.print("MAIN");

  display.setCursor(32, 1);
  display.print(loggingEnabled ? "REC" : "STOP");

  int mm = (sec / 60) % 60;
  int ss = sec % 60;

  display.setCursor(68, 1);
  if (mm < 10) display.print('0');
  display.print(mm);
  display.print(':');
  if (ss < 10) display.print('0');
  display.print(ss);

  drawBatteryIcon(110, 1, batteryPercent);

  display.drawLine(0, 11, 127, 11, SSD1306_WHITE);
}

void drawBody(float temp, float hum, float press, uint32_t als, uint32_t uvs) {
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 15);
  display.print("T:");
  display.print(temp, 1);
  display.print("C");

  display.setCursor(64, 15);
  display.print("H:");
  display.print(hum, 0);
  display.print("%");

  display.setCursor(0, 27);
  display.print("P:");
  display.print(press, 1);
  display.print("hPa");

  display.setCursor(0, 39);
  display.print("ALS:");
  display.print(als);

  display.setCursor(72, 39);
  display.print("UV:");
  display.print(uvs);
}

void drawFooter() {
  display.drawLine(0, 52, 127, 52, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

  display.setCursor(0, 54);
  display.print("<MENU");
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(21, 22);

  Serial.println();
  Serial.println("Main layout spacing adjusted start");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED init NG");
    while (1);
  }

  if (!bme.begin(0x76)) {
    Serial.println("BME280 init NG");
    while (1);
  }

  if (!ltr.begin()) {
    Serial.println("LTR390 init NG");
    while (1);
  }

  // ALS固定で安定動作を優先
  ltr.setMode(LTR390_MODE_ALS);
  ltr.setGain(LTR390_GAIN_3);
  ltr.setResolution(LTR390_RESOLUTION_16BIT);
  delay(200);

  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.setTextSize(1);
  display.display();

  Serial.println("OLED init OK");
  Serial.println("BME280 init OK");
  Serial.println("LTR390 init OK");
}

void loop() {
  float temp = bme.readTemperature();
  float hum = bme.readHumidity();
  float press = bme.readPressure() / 100.0f;

  uint32_t als = 0;
  if (ltr.newDataAvailable()) {
    als = ltr.readALS();
  }

  // 現在はALS固定動作のため、UVは仮表示
  uint32_t uvs = 0;

  Serial.print("T=");
  Serial.print(temp, 1);
  Serial.print("C H=");
  Serial.print(hum, 1);
  Serial.print("% P=");
  Serial.print(press, 1);
  Serial.print("hPa ALS=");
  Serial.print(als);
  Serial.print(" UV=");
  Serial.println(uvs);

  display.clearDisplay();
  drawHeader(millis() / 1000);
  drawBody(temp, hum, press, als, uvs);
  drawFooter();
  display.display();

  delay(300);
}