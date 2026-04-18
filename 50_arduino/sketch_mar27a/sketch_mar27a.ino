  #include <Arduino.h>
#include <esp_sleep.h>

// ========================================
// ESP32-C3 SuperMini 単体試験スケッチ
// 試験内容:
// 1. 起動確認
// 2. シリアル出力確認
// 3. GPIO出力確認
// 4. GPIO入力確認
// 5. DeepSleep確認
// ========================================

// ---- 仮設定: 要注意 ----
// 手元ボードで安全に使えるGPIOかは未確認。
// 問題が出る場合は別ピンへ変更、または該当試験を無効化してください。
static const int TEST_OUT_PIN = 3;   // 出力確認用
static const int TEST_IN_PIN  = 4;   // 入力確認用

// ---- 試験設定 ----
static const unsigned long SERIAL_BAUD = 115200;
static const unsigned long STATUS_INTERVAL_MS = 1000;
static const unsigned long GPIO_TOGGLE_INTERVAL_MS = 500;
static const unsigned long ACTIVE_BEFORE_SLEEP_MS = 15000;   // 15秒後にDeepSleepへ
static const uint64_t SLEEP_TIME_US = 5ULL * 1000ULL * 1000ULL; // 5秒スリープ

RTC_DATA_ATTR int bootCount = 0;

// GPIO試験を一時的に外したい場合は false にする
static const bool ENABLE_GPIO_TEST = true;
//static const bool ENABLE_GPIO_TEST = false;

// DeepSleep試験を一時的に外したい場合は false にする
static const bool ENABLE_DEEPSLEEP_TEST = true;

unsigned long lastStatusMs = 0;
unsigned long lastToggleMs = 0;
unsigned long startMs = 0;

bool outState = false;

const char* wakeupReasonToText(esp_sleep_wakeup_cause_t cause) {
  switch (cause) {
    case ESP_SLEEP_WAKEUP_UNDEFINED: return "通常起動";
    case ESP_SLEEP_WAKEUP_TIMER:     return "タイマー復帰";
    case ESP_SLEEP_WAKEUP_GPIO:      return "GPIO復帰";
    case ESP_SLEEP_WAKEUP_UART:      return "UART復帰";
    default:                         return "その他要因";
  }
}

void printHeader() {
  Serial.println();
  Serial.println("========================================");
  Serial.println("ESP32-C3 SuperMini 単体試験");
  Serial.println("========================================");
  Serial.printf("bootCount = %d\n", bootCount);
  Serial.printf("Reset reason / Wakeup reason = %s\n", wakeupReasonToText(esp_sleep_get_wakeup_cause()));
  Serial.printf("Chip model = %s\n", ESP.getChipModel());
  Serial.printf("Chip revision = %d\n", ESP.getChipRevision());
  Serial.printf("CPU freq MHz = %u\n", ESP.getCpuFreqMHz());
  Serial.printf("Free heap = %u bytes\n", ESP.getFreeHeap());

  if (ENABLE_GPIO_TEST) {
    Serial.printf("TEST_OUT_PIN = GPIO%d\n", TEST_OUT_PIN);
    Serial.printf("TEST_IN_PIN  = GPIO%d\n", TEST_IN_PIN);
  } else {
    Serial.println("GPIO test = DISABLED");
  }

  if (ENABLE_DEEPSLEEP_TEST) {
    Serial.printf("DeepSleep = ENABLED, active %lu ms -> sleep 5 sec\n", ACTIVE_BEFORE_SLEEP_MS);
  } else {
    Serial.println("DeepSleep = DISABLED");
  }

  Serial.println("========================================");
  Serial.println();
}

void setupGpioTest() {
  if (!ENABLE_GPIO_TEST) return;

  pinMode(TEST_OUT_PIN, OUTPUT);
  digitalWrite(TEST_OUT_PIN, LOW);

  pinMode(TEST_IN_PIN, INPUT_PULLUP);

  Serial.println("[GPIO] 出力ピンを LOW で初期化しました");
  Serial.println("[GPIO] 入力ピンは INPUT_PULLUP に設定しました");
  Serial.println("[GPIO] 入力試験は TEST_IN_PIN を GND に短絡すると LOW になります");
}

void enterDeepSleep() {
  Serial.println();
  Serial.println("[SLEEP] DeepSleep に入ります");
  Serial.flush();

  esp_sleep_enable_timer_wakeup(SLEEP_TIME_US);
  esp_deep_sleep_start();
}

void setup() {
  bootCount++;
  startMs = millis();

  Serial.begin(SERIAL_BAUD);

  // USBシリアル安定待ち
  delay(1500);

  printHeader();
  setupGpioTest();

  Serial.println("[BOOT] setup() 完了");
}

void loop() {
  unsigned long now = millis();

  // 1秒ごとの状態表示
  if (now - lastStatusMs >= STATUS_INTERVAL_MS) {
    lastStatusMs = now;

    Serial.printf("[STATUS] uptime=%lu ms", now);

    if (ENABLE_GPIO_TEST) {
      int inLevel = digitalRead(TEST_IN_PIN);
      Serial.printf(", IN=%s", (inLevel == LOW) ? "LOW(短絡/押下)" : "HIGH(開放)");
      Serial.printf(", OUT=%s", outState ? "HIGH" : "LOW");
    }

    Serial.printf(", heap=%u\n", ESP.getFreeHeap());
  }

  // 500msごとに出力トグル
  if (ENABLE_GPIO_TEST && (now - lastToggleMs >= GPIO_TOGGLE_INTERVAL_MS)) {
    lastToggleMs = now;

    outState = !outState;
    digitalWrite(TEST_OUT_PIN, outState ? HIGH : LOW);
  }

  // DeepSleep試験
  if (ENABLE_DEEPSLEEP_TEST && now >= ACTIVE_BEFORE_SLEEP_MS) {
    enterDeepSleep();
  }
}