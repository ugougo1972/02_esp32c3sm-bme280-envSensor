# 07_SOFTWARE_ARCHITECTURE

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるソフトウェア実装構造を整理するためのものである。  
上位文書で定義した状態遷移、ログ形式、電源制御方針を、**実際にコードへ落とし込める粒度** まで具体化する。

初号機では、まず **安定して測定・保存・Deep Sleep 周期動作が成立する構造** を優先し、過度な抽象化や複雑なクラス設計は採らない。

## 基本方針

- 実装は **Arduino IDE / XIAO ESP32S3 Plus** を前提とする
- 初号機では、**1スケッチで全体成立** を優先する
- 処理は役割ごとに関数分割する
- `setup()` / `loop()` の責務を明確に分ける
- Deep Sleep logger 基準版では、**setup() 完結型** を採用する
- UI試験版・periodic logger 版では、`loop()` による non-blocking 管理を採用する
- 入力系は **ESP32Encoder + PCNT 方式** を採用する
- 文書上の列名・状態名・コード名は  
  - `03_LOG_FORMAT.md`
  - `04_ctx.md`
  - `05_head.md`
  - `02_STATE_MACHINE.md`
  と整合させる

## 実装モードの分離方針

初号機ソフトは、1つの最終完成版に一気に寄せるのではなく、目的別に段階構成で扱う。

| モード | 目的 | 主な特徴 |
|------|------|------|
| UI試験版 | 表示・入力確認 | `loop()` 中心 |
| periodic logger 版 | 継続測定・継続保存確認 | `loop()` 中心 |
| Deep Sleep logger 基準版 | 周期起床・保存成立確認 | `setup()` 完結型 |
| Battery monitor 試験版 | 分圧回路・ADC確認 | `loop()` 中心 |

### 現時点の基準点

- 現時点の基準点は **Deep Sleep logger 基準版**
- 理由は、最終運用に近い形で  
  - RTC  
  - BME280  
  - LTR390  
  - microSD  
  - Deep Sleep  
  が成立しているため

## ソフトウェア全体構造

### 推奨レイヤ構成

| 層 | 内容 |
|------|------|
| アプリケーション層 | 状態遷移、周期制御、モード選択 |
| 機能制御層 | 測定、表示、保存、入力、Sleep 制御 |
| デバイス制御層 | BME280 / LTR390 / DS3231 / SSD1306 / SD / Encoder / ADC |

## ファイル構成方針

初号機では、まずは **単一 `.ino` スケッチ** で成立させてよい。  
必要に応じて `.h` / `.cpp` 分割へ移行する。

### 初期段階の推奨構成

```text
env_logger_main.ino
```

### 将来の分割候補

```text
env_logger_main.ino
config.h
pins.h
log_format.h
sensor_task.cpp
rtc_task.cpp
display_task.cpp
input_task.cpp
encoder_led_task.cpp
sleep_task.cpp
sd_task.cpp
battery_task.cpp
```

## データ構造方針

### 1. 測定データ構造体

```cpp
struct LogRecord {
  char date[11];
  char time[9];
  float temp_c;
  float hum_pct;
  float press_hpa;
  uint32_t als;
  uint32_t uvs;
  uint8_t context_code;
  uint8_t head_code;
  float vbat;
};
```

### 2. UI状態

```cpp
enum UiState {
  UI_VIEW,
  UI_MENU,
  UI_CLOCK,
  UI_LOG,
  UI_SLEEP
};
```

### 3. MENU選択状態

```cpp
enum MenuItem {
  MENU_VIEW,
  MENU_CLOCK,
  MENU_LOG,
  MENU_SLEEP
};
```

## 定数定義方針

### 周期系

```cpp
const uint32_t SENSOR_INTERVAL_MS = 1000;
const uint32_t DISPLAY_INTERVAL_MS = 200;
const uint32_t LOG_INTERVAL_MS = 5000;
const uint32_t OLED_SHOW_MS = 5000;
const uint64_t SLEEP_INTERVAL_US = 60ULL * 1000ULL * 1000ULL;
```

### 状態コード系

```cpp
const uint8_t CTX_UNKNOWN = 0;
const uint8_t CTX_INDOOR  = 1;
const uint8_t CTX_OUTDOOR = 2;
const uint8_t CTX_MOVING  = 3;
const uint8_t CTX_OFFICE  = 4;
const uint8_t CTX_HOME    = 5;

const uint8_t HEAD_UNKNOWN   = 0;
const uint8_t HEAD_NONE      = 1;
const uint8_t HEAD_MILD      = 2;
const uint8_t HEAD_MODERATE  = 3;
const uint8_t HEAD_SEVERE    = 4;
const uint8_t HEAD_RECOVERED = 5;
```

### Battery monitor 系

```cpp
const float VBAT_DIV_RATIO = 2.0f;   // 100kΩ : 100kΩ
```

## グローバル変数方針

| 変数 | 用途 |
|------|------|
| `UiState uiState` | 現在画面 |
| `MenuItem menuItem` | MENU選択位置 |
| `LogRecord currentRecord` | 最新測定値 |
| `unsigned long lastSensorMs` | センサ周期管理 |
| `unsigned long lastDisplayMs` | 表示周期管理 |
| `unsigned long lastLogMs` | ログ周期管理 |
| `uint8_t currentContextCode` | 現在の行動状態 |
| `uint8_t currentHeadCode` | 現在の頭痛状態 |
| `uint32_t bootCount` | 起動回数確認 |
| `int32_t encoderCount` | Encoder カウント |
| `bool encoderPressed` | 押下状態 |

## 関数分割方針

### 初期化系

| 関数名例 | 役割 |
|------|------|
| `initWire()` | I2C初期化 |
| `initRtc()` | DS3231初期化 |
| `initBme280()` | BME280初期化 |
| `initLtr390()` | LTR390初期化 |
| `initDisplay()` | OLED初期化 |
| `initSd()` | SPI / microSD初期化 |
| `initEncoder()` | Encoder入力初期化 |
| `initBatteryAdc()` | Battery ADC 初期化 |

### 読出し系

| 関数名例 | 役割 |
|------|------|
| `readRtcToRecord(LogRecord &rec)` | 日付・時刻取得 |
| `readBme280ToRecord(LogRecord &rec)` | 温度・湿度・気圧取得 |
| `readLtr390ToRecord(LogRecord &rec)` | ALS / UVS取得 |
| `readBatteryToRecord(LogRecord &rec)` | `vbat`取得 |

### UI系

| 関数名例 | 役割 |
|------|------|
| `updateEncoder()` | Encoder状態更新 |
| `handleUiInput()` | 状態遷移処理 |
| `setEncoderLed()` | RGB LED 制御 |
| `drawViewScreen()` | VIEW画面描画 |
| `drawMenuScreen()` | MENU画面描画 |
| `drawClockScreen()` | CLOCK画面描画 |
| `drawLogScreen()` | LOG画面描画 |
| `drawSleepScreen()` | SLEEP画面描画 |

### ログ系

| 関数名例 | 役割 |
|------|------|
| `buildCsvLine(const LogRecord &rec)` | CSV1行生成 |
| `ensureLogFileHeader()` | ヘッダ確認・作成 |
| `appendLogRecord(const LogRecord &rec)` | 1行追記 |

### 電源系

| 関数名例 | 役割 |
|------|------|
| `prepareForSleep()` | Sleep前処理 |
| `enterDeepSleep()` | Deep Sleep移行 |
| `printWakeupReason()` | 起床要因表示 |

## `setup()` / `loop()` の責務

### UI試験版 / periodic logger 版

#### setup()

- Serial開始
- I2C / SPI 初期化
- RTC / BME280 / LTR390 / OLED / SD 初期化
- Encoder初期化
- 初期状態設定
- ヘッダ確認

#### loop()

- Encoder読取り
- UI状態遷移
- 周期ごとにセンサ読出し
- 周期ごとに表示更新
- 周期ごとにCSV保存

### Deep Sleep logger 基準版

#### setup()

- Serial開始
- 起床要因確認
- bootCount更新
- I2C / SPI初期化
- RTC / BME280 / LTR390 / SD 初期化
- RTC妥当性確認と必要なら復旧
- 測定値取得
- CSV保存
- 必要なら短時間表示
- 次回起床条件設定
- Deep Sleep移行

#### loop()

```cpp
void loop() {
  // Deep Sleep logger 基準版では未使用
}
```

## Battery monitor 実装構造

### 回路前提

```text
TP4056 OUT+ -- 100kΩ --+-- ADC
                       |
                     100kΩ
                       |
TP4056 OUT- -----------+-- GND
```

### 読出し方針

- ADCで読んだ値を **2倍** して `vbat` とする
- 初回はテスター値との一致確認を優先する
- `analogReadMilliVolts()` を優先利用する

### 例

```cpp
void readBatteryToRecord(LogRecord &rec) {
  uint32_t vadc_mv = analogReadMilliVolts(PIN_BAT_MON);
  rec.vbat = (vadc_mv / 1000.0f) * VBAT_DIV_RATIO;
}
```

## CSV生成方針

### 推奨列順

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat
```

### 文字列生成例

```cpp
String buildCsvLine(const LogRecord &rec) {
  String line = "";
  line += rec.date;
  line += ",";
  line += rec.time;
  line += ",";
  line += String(rec.temp_c, 2);
  line += ",";
  line += String(rec.hum_pct, 2);
  line += ",";
  line += String(rec.press_hpa, 2);
  line += ",";
  line += String(rec.als);
  line += ",";
  line += String(rec.uvs);
  line += ",";
  line += String(rec.context_code);
  line += ",";
  line += String(rec.head_code);
  line += ",";
  line += String(rec.vbat, 3);
  return line;
}
```

## 実装順序

1. ピン定義固定
2. 定数・enum・struct 定義
3. RTC読出し関数
4. BME280読出し関数
5. LTR390読出し関数
6. CSV生成関数
7. SD追記関数
8. Deep Sleep logger 基準版完成
9. periodic logger 版完成
10. UI状態遷移追加
11. RGB Encoder 入力・LED追加
12. 画面描画整理
13. `context_code` / `head_code` 入力反映
14. `vbat` 追加

## 初号機で保留とする実装

- クラスベース全面再設計
- 複数ファイルへの過度な細分化
- イベント駆動フレームワーク導入
- FreeRTOS タスク分割
- Wi-Fi時刻同期
- センサ値からの状態自動推定
- 電池残量アルゴリズム本格実装

## ステータス

- [ACTIVE] 初号機ソフトウェア実装構造として有効
- [ACTIVE] Deep Sleep logger 基準版を基準構造とする
- [ACTIVE] 単一スケッチ + 関数分割方針を採用
- [ACTIVE] `context_code` / `head_code` / `vbat` を含む LogRecord 構造を採用
- [ACTIVE] RGB LED付きスイッチ付き Rotary Encoder 前提へ更新済み
- [CHECK] `vbat` 実装未着手
- [CHECK] `context_code` / `head_code` 入力UI未確定
- [CHECK] 将来のファイル分割は未確定
