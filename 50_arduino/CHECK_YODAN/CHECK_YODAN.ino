#include <ESP32Encoder.h>

const int PIN_SW    = D2;
const int PIN_LED_R = D3;
const int PIN_LED_G = D4;
const int PIN_LED_B = D5;

// 共通アノード想定
const bool LED_COMMON_ANODE = true;

ESP32Encoder encoder;

long lastCount = 0;
int swLast = LOW;
unsigned long swLastChangeMs = 0;
const unsigned long SW_DEBOUNCE_MS = 20;

void ledWrite(bool r, bool g, bool b) {
  if (LED_COMMON_ANODE) {
    digitalWrite(PIN_LED_R, r ? LOW : HIGH);
    digitalWrite(PIN_LED_G, g ? LOW : HIGH);
    digitalWrite(PIN_LED_B, b ? LOW : HIGH);
  } else {
    digitalWrite(PIN_LED_R, r ? HIGH : LOW);
    digitalWrite(PIN_LED_G, g ? HIGH : LOW);
    digitalWrite(PIN_LED_B, b ? HIGH : LOW);
  }
}

void ledGreen() { ledWrite(false, true,  false); }
void ledBlue()  { ledWrite(false, false, true ); }
void ledRed()   { ledWrite(true,  false, false); }
void ledWhite() { ledWrite(true,  true,  true ); }

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(PIN_SW, INPUT_PULLDOWN);
  pinMode(PIN_LED_R, OUTPUT);
  pinMode(PIN_LED_G, OUTPUT);
  pinMode(PIN_LED_B, OUTPUT);

  ledWhite();
  delay(250);
  ledGreen();

  // weak pullup を内部で使う設定
  ESP32Encoder::useInternalWeakPullResistors = puType::up;

  // A, B の順
  encoder.attachHalfQuad(D0, D1);
  encoder.clearCount();
  lastCount = encoder.getCount();

  swLast = digitalRead(PIN_SW);

  Serial.println("=== RGB encoder PCNT test start ===");
}

void loop() {
  long count = encoder.getCount();

static long accum = 0;

if (count != lastCount) {
  long diff = count - lastCount;
  accum += diff;
  lastCount = count;

  if (accum >= 2) {
    accum = 0;
    Serial.println("CCW");   // いまは名前を反転
    ledWhite();
    delay(50);
    ledGreen();
  } else if (accum <= -2) {
    accum = 0;
    Serial.println("CW");    // いまは名前を反転
    ledRed();
    delay(50);
    ledGreen();
  }
}
  int swNow = digitalRead(PIN_SW);
  if (swNow != swLast) {
    delay(SW_DEBOUNCE_MS);
    swNow = digitalRead(PIN_SW);

    if (swNow != swLast) {
      swLast = swNow;
      if (swNow == HIGH) {
        Serial.println("PUSH");
        ledBlue();
      } else {
        Serial.println("RELEASE");
        ledGreen();
      }
    }
  }
}