# 02_携帯型環境センサーロガー作成プロジェクト（02_esp32c3sm-bme280-envSensor）

携帯型環境センサーロガー  
頭痛発生と環境要因の相関を記録・分析するための装置

---

## リポジトリ構成

```text
02_esp32c3sm-bme280-envSensor/
├─ 01_README.md
├─ 02_SYSTEM_OVERVIEW.md
├─ 03_REQUIREMENTS.md
├─ 10_HARDWARE/
│  ├─ 01_HARDWARE_OVERVIEW.md
│  ├─ 02_PARTS_LIST.md
│  ├─ 03_POWER_DESIGN.md
│  ├─ 04_PIN_ASSIGNMENT.md
│  ├─ 05_WIRING_DIAGRAM.md
│  └─ 06_ENCLOSURE.md
├─ 20_SOFTWARE/
│  ├─ 01_SOFTWARE_OVERVIEW.md
│  ├─ 02_STATE_MACHINE.md
│  ├─ 03_LOG_FORMAT.md
│  ├─ 04_ctx.md
│  ├─ 05_head.md
│  ├─ 06_POWER_CONTROL.md
│  └─ 07_SOFTWARE_ARCHITECTURE.md
├─ 30_LOG/
│  ├─ 01_DATA_ANALYSIS_PLAN.md
│  └─ 02_HEADACHE_ANALYSIS.md
└─ 40_DEV/
   ├─ 01_BRINGUP_PLAN.md
   ├─ 02_TEST_PLAN.md
   └─ 03_TROUBLESHOOTING.md
```

---

## 記録項目

- 時刻
- 温度
- 湿度
- 気圧
- 照度
- UV
- 移動ログ
- 頭痛ステータス
- 電池電圧（実装予定）

---

## ハード構成

- メインMCU: Seeed Studio XIAO ESP32S3 Plus
- 評価用MCU: ESP32-WROOM系開発ボード
- BME280
- LTR390
- DS3231（RTC）
- microSD
- OLED 0.96インチ（SSD1306, 128x64）
- RGB LED付きスイッチ付きロータリーエンコーダ
- LiPo（容量は運用途中で増減可能）
- TP4056系充電モジュール（AE-TP4056またはTP4056互換）
- AE-TPS63802（昇降圧スイッチングレギュレータ・3.3V出力・EN端子付き）

### 到着済み部材（現時点）

- ESP32-WROOM系開発ボード（評価用）
- Seeed Studio XIAO ESP32S3 Plus
- OLED 0.96インチ
- BME280
- LTR390
- DS3231+AT24C32 モジュール
- microSDカード
- microSDモジュール
- TP4056系充電モジュール
- AE-TPS63802（昇降圧スイッチングレギュレータ）※XC9306から変更
- LiPo（初期評価用の暫定容量）
- RGB LED付きスイッチ付きロータリーエンコーダ
- ユニバーサル基板（72mm×47mm）
- ICピン用クリップ

### 未到着または未提示の部材

- 追加の電源スイッチ候補
- 筐体固定用補助部材一式

---

## 動作方針

- 1分ごとに測定
- 測定後にSDへ保存
- 通常はDeep Sleep
- OLEDは通常消灯
- 操作時にOLED点灯
- 測定後は一定時間のみOLED点灯
- Wi-Fiは通常OFF
- Bluetoothは通常OFF
- 想定稼働時間帯は 6:00〜23:00

---

## 目的

頭痛発生と以下の要因の相関分析を行う。

- 気圧変化
- 明るさ
- UV
- 温湿度
- 行動（屋内 / 屋外 / 移動）

---

## 電源系構成（確定）

### 電源経路

```
LiPo
  │
  └─ JST（2pin）
        │  AWG24
     TP4056（充電・保護）
        │  AWG28
     AE-TPS63802（昇降圧・3.3V出力）
        │
     端子台（4ライン）
        │
     メイン基板
```

### 端子台ライン構成

| ライン | 内容 | 接続先 |
|--------|------|--------|
| 3.3V | TPS63802 VOUT出力 | メイン基板 3.3V |
| GND | GNDバス | メイン基板 GND |
| VBAT_RAW | TP4056 OUT+直接引き出し | メイン基板 ADC分圧回路 |
| EN | TPS63802 EN端子 | XIAO GPIO（Deep Sleep制御） |

### 電源基板コンデンサ構成

| 部品 | 容量 | 種別 | 接続 |
|------|------|------|------|
| C1 | 0.1uF | MLCC（極性なし） | TP4056 OUT+ ↔ OUT- 間 |
| C2 | 47uF | 電解（極性あり・長足→VOUT側） | TPS63802 VOUT ↔ GND 間 |
| C3 | 0.1uF | MLCC（極性なし） | TPS63802 VOUT ↔ GND 間（C2と並列） |

### 線材

| 区間 | 線材 |
|------|------|
| JST ↔ TP4056 B+/B- | AWG24（直付け） |
| その他全配線 | AWG28 |
| GNDバス・3.3Vライン | スズメッキ線 |

### 分圧回路

- 分圧回路（R1/R2 各100kΩ）はメイン基板側に配置
- VBAT_RAW端子台からメイン基板上のR1/R2を経由してXIAO ADCピンへ接続
- ADC中点-GND間に0.1uFを追加（ふらつき確認後に判断）

### EN端子の活用

- TPS63802の4P ENピンをXIAO GPIOで制御
- HIGH：通常動作
- LOW：Deep Sleep時にTPS63802をシャットダウン→センサ類への3.3V供給を遮断

### ハンダ作業メモ

- 共晶ハンダ（鉛入り）使用時の適正温度：**280℃**（320℃は不適切）
- コテ先は常に銀色の状態を維持
- フラックスを活用
- スルーホール間ブリッジにはスズメッキ線を橋として使用

---

## GPIO割り当て（現時点）

| ピン | GPIO | 用途 |
|------|------|------|
| D0 | GPIO1 | Encoder A |
| D1 | GPIO2 | Encoder B |
| D3 | GPIO4 | Encoder SW |
| D11 | GPIO38 | LED R |
| D12 | GPIO39 | LED G |
| D13 | GPIO40 | LED B |
| D4 | GPIO5 | I2C SDA |
| D5 | GPIO6 | I2C SCL |
| D2 | GPIO3 | microSD CS |
| D8 | GPIO7 | SPI SCK |
| D9 | GPIO8 | SPI MISO |
| D10 | GPIO9 | SPI MOSI |
| D16 | GPIO10 | Battery monitor ADC（候補・未確定） |
| TBD | TBD | TPS63802 EN制御（未確定） |

---

## 現在の進捗

### 完了済み（DONE）

- 要求仕様整理
- ログ設計
- 状態設計
- 部品選定
- XIAO ESP32S3 Plus 単体起動確認
- BME280 単体 / 統合
- LTR390 単体 / 統合
- OLED 単体 / 統合
- DS3231 単体 / 統合
- microSD 単体 / 統合
- periodic logger
- Deep Sleep logger
- UI基本遷移（VIEW / MENU / CLOCK / LOG / SLEEP）
- RGB LED付きスイッチ付きロータリーエンコーダ置換
- TP4056単体試験合格
- 電源基板レイアウト確定（72mm×47mm）
- 電源基板配線図確定

### 進行中（IN PROGRESS）

- AE-TPS63802換装・電源基板実装
- ハンダ付け作業（温度280℃に修正後）

### 未実施（TODO）

- TPS63802通電試験
- battery voltage monitor 分圧回路実装（メイン基板側）
- ADCピン最終確定
- TPS63802 EN端子用GPIOピン確定
- LiPo駆動での統合logger試験
- LiPo駆動でのDeep Sleep logger試験
- DS3231 主電源断後バックアップ保持の最終判定
- 電源基板とメイン基板の端子台インターフェース最終固定
- 筐体 / 最終レイアウト

### 確認中（CHECK）

- DS3231の主電源断後バックアップ保持

---

## 変更履歴

| 日付 | 変更内容 |
|------|---------|
| 2025年以前 | 初期構成確定・各センサBring-up完了 |
| 最新 | XC9306→AE-TPS63802へ変更（EN端子追加） |
| 最新 | 端子台3ライン→4ライン（EN追加） |
| 最新 | 分圧回路をメイン基板側に配置する方針確定 |
| 最新 | C1/C2/C3構成・接続方法確定 |
| 最新 | ハンダ温度を280℃に修正（共晶ハンダ適正温度） |
| 最新 | 電源基板レイアウト・配線図最終版確定 |

---

## 補足

本リポジトリは、以下を段階的に整理・実装するための管理場所とする。

- 要求仕様の整理
- ハードウェア構成の整理
- ソフトウェア構成の整理
- ログ仕様の整理
- 試作・評価・改善履歴の蓄積
