# 02_携帯型環境センサーロガー作成プロジェクト（02_esp32s3-bme280-envSensor）

携帯型環境センサーロガー  
頭痛発生と環境要因の相関を記録・分析するための装置

---

## リポジトリ構成

```text
02_esp32s3-bme280-envSensor/
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
- 電池電圧

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
- MCP23017（I2C GPIO Expander・LED R/G/B制御用）
- LiPo（容量は運用途中で増減可能）
- TP4056系充電モジュール
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
- AE-TPS63802
- LiPo（初期評価用の暫定容量）
- RGB LED付きスイッチ付きロータリーエンコーダ
- ユニバーサル基板（72mm×47mm・電源基板用）
- ユニバーサル基板（95mm×72mm・メイン基板用）
- MCP23017（I2C GPIO Expander・DIP28）
- JST-XH 2.5mmコネクタ（2.54mmピッチ統一）
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

## 基板構成（確定）

### 2基板構成

| 基板 | サイズ | 主要部品 |
|------|--------|---------|
| 電源基板 | 72mm × 47mm | LiPo / TP4056 / AE-TPS63802 / C1/C2/C3 / 4ライン端子台 |
| メイン基板 | 95mm × 72mm | XIAO / BME280 / DS3231 / microSD / MCP23017 / JST群 / 分圧回路 / R_EN |

### メイン基板レイアウト（確定）

基板有効穴数：横37穴 × 縦27穴

**3.3Vバス**: 外周横線(X:1-34, Y:1) + 支線横線(X:1-10, Y:20) + 外周縦線(X:1, Y:1-25)

**GNDバス**: 外周横線(X:3-36, Y:27) + 支線横線(X:29-36, Y:12) + 外周縦線(X:36, Y:3-27)

**SDAバス**: 縦線(X:13, Y:10-23)

**SCLバス**: 縦線(X:17, Y:10-23)

| 部品 | 座標 | 特記事項 |
|------|------|---------|
| XIAO | (16,02)～(22,08) | D0～D10ピン配置確定 |
| MCP23017 | (21,13)～(26,26) | VDD/VSS間にバイパスC(0.1uF) |
| DS3231 | (02,02)～(10,17) | 上辺(Y:02)は機械支持専用・電気的未接続 |
| BME280 | (02,22)～(07,26) | CSB=High(3.3V) / SDO=Low(GND) |
| microSD | (27,02)～(35,09) | SPI接続 |
| JST1(OLED) | (10,24)～(13,26) | I2C接続(0x3C) |
| JST2(LTR390) | (15,24)～(18,26) | I2C接続(0x53) |
| JST3(Encoder) | (32,14)～(34,21) | GPIO+LED制御 |
| 端子台 | (31,25)～(34,27) | GND / 3.3V / VBAT_RAW / EN |
| R_EN(100kΩ) | (12,08)～(14,08) | 入力:(11,08)→3.3Vバス / 出力:(14,08)→D6合流 |
| バイパスC | (21,21)～(21,22) | MCP23017 VDD-VSS間 |
| 分圧R | (13,03)～(13,06) | R1(13,03-04) / R2(13,05-06) |
| プルアップR | (27,23)～(28,23) | MCP23017 /RESET |
| LED抵抗 | (27,18)～(28,20) | R_R / R_G / R_B |

---

## GPIO割り当て（確定）

### XIAO ESP32S3 Plus ピン配置

| 端子 | GPIO | 用途 | 特性 |
|------|------|------|------|
| D0 | GPIO1 | Encoder_A | RTC GPIO対応・EXT0復帰可 |
| D1 | GPIO2 | Encoder_B | RTC GPIO対応 |
| D2 | GPIO3 | Encoder_SW | RTC GPIO対応・EXT1復帰可 |
| D3 | GPIO4 | Battery_ADC | RTC GPIO対応・ADC1_CH3 |
| D4 | GPIO5 | I2C_SDA | 確定 |
| D5 | GPIO6 | I2C_SCL | 確定 |
| D6 | GPIO43 | TPS63802_EN | RTC非対応→100kΩプルアップで対策 |
| D7 | GPIO44 | SPI_CS | UART RX兼用・USB CDC主体で使用可 |
| D8 | GPIO7 | SPI_SCK | 確定 |
| D9 | GPIO8 | SPI_MISO | 確定 |
| D10 | GPIO9 | SPI_MOSI | 確定 |
| D16 | GPIO10 | 予約 | 未使用 |

### MCP23017（I2Cアドレス 0x20）

| ピン | 用途 |
|-----|------|
| GPA0 | LED R（100Ω経由） |
| GPA1 | LED G（100Ω経由） |
| GPA2 | LED B（100Ω経由） |
| その他 | 未使用（将来拡張余地） |

---

## 電源系構成（確定）

### 電源経路

```
LiPo
  │
  └─ JST（2pin）AWG24
        │
     TP4056（充電・保護）
        │  AWG28
     AE-TPS63802（昇降圧・3.3V出力・EN端子付き）
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
| EN | TPS63802 EN端子 | XIAO D6(GPIO43)（Deep Sleep制御） |

### 電源基板コンデンサ構成

| 部品 | 容量 | 種別 | 接続 |
|------|------|------|------|
| C1 | 0.1uF | MLCC | TP4056 OUT+ ↔ OUT- 間 |
| C2 | 47uF | 電解（長足→VOUT側） | TPS63802 VOUT ↔ GND 間 |
| C3 | 0.1uF | MLCC | TPS63802 VOUT ↔ GND 間（C2並列） |

### 線材

| 区間 | 線材 |
|------|------|
| JST ↔ TP4056 B+/B- | AWG24（直付け） |
| その他全配線 | AWG28 |
| GNDバス・3.3Vライン | スズメッキ線 |

### Battery Voltage Monitor

- 測定対象: TP4056 OUT+ / OUT-
- 分圧回路: メイン基板側配置（100kΩ+100kΩ 1:1）
- ADC入力: XIAO D3(GPIO4)・ADC1_CH3
- 安定化: ADC中点-GND間に0.1uF追加可

---

## DeepSleep対応設計（確定）

### GPIO43(TPS63802_EN)の注意

- RTC GPIO非対応のため、DeepSleep中にHi-Z化する可能性
- **回路側対策**: 100kΩプルアップ(R_EN)で対応
- R_EN: (12,08)～(14,08)横方向・縦置き実装
- 入力: (11,01)→(11,08)→3.3Vバス経由
- 出力: (14,08)→XIAO D6+端子台EN合流

### GPIO44(SPI_CS)の注意

- UART RX兼用だが、USB CDC主体のため通常GPIO利用可
- UARTデバッグが必要な場合は別ピン検討
- 初号機ではGPIO44利用確定

### microSD CS処理

DeepSleep前に以下を実施:
```cpp
pinMode(PIN_SPI_CS, OUTPUT);
digitalWrite(PIN_SPI_CS, HIGH);
SPI.end();
```

### Encoder復帰

- GPIO1/2/3はRTC GPIO対応
- EXT0/EXT1によるDeepSleep復帰が可能
- Encoder_SW(GPIO3)での復帰実装可

---

## ケーブル接続モジュール

| モジュール | 基板側 | ケーブル | モジュール側 |
|-----------|--------|---------|-------------|
| OLED | JST1 メスヘッダ 4P | AWG28ケーブル | モジュール足に直接ハンダ付け |
| LTR390 | JST2 メスヘッダ 4P | AWG28ケーブル | モジュール足に直接ハンダ付け |
| Encoder | JST3 メスヘッダ 8P | AWG28ケーブル | ブレイクアウト基板オスピン |

---

## 現在の進捗

### 完了済み（DONE）

- 要求仕様整理
- ログ設計
- 状態設計
- 部品選定
- XIAO ESP32S3 Plus 単体起動確認
- BME280 / LTR390 / OLED / DS3231 / microSD 統合動作確認
- periodic logger / Deep Sleep logger 動作確認
- UI基本遷移動作確認
- RGB LED付きロータリーエンコーダ確認
- TP4056単体試験合格
- **電源基板フェーズ1～7通電試験 PASS**
- **GPIO割り当て確定**
- **MCP23017採用確定**
- **メイン基板全部品配置座標確定**
- **レイアウト案確定（R_EN配置・DeepSleep対応）**

### 進行中（IN PROGRESS）

- メイン基板実装作業

### 未実施（TODO）

- battery voltage monitor 分圧回路実装
- MCP23017実装・LED制御確認
- EN端子制御（GPIO43）実装・確認
- LiPo駆動での統合logger試験
- LiPo駆動でのDeep Sleep logger試験

### 確認中（CHECK）

- DS3231主電源断後バックアップ保持

---

## 変更履歴

| 変更内容 |
|---------|
| XC9306→AE-TPS63802へ変更（EN端子追加） |
| 端子台3ライン→4ライン（EN追加） |
| 分圧回路をメイン基板側に配置確定 |
| ハンダ温度を280℃に修正（共晶ハンダ適正温度） |
| 電源基板フェーズ1～7通電試験PASS |
| メイン基板サイズ72mm×47mm→95mm×72mm |
| MCP23017採用確定（LED R/G/B制御） |
| **GPIO割り当て確定（D0～D10・D16）** |
| **メイン基板全部品配置座標確定** |
| **レイアウト案確定（R_EN配置済み）** |
| **DeepSleep対応設計確定** |

---

## 補足

本リポジトリは、以下を段階的に整理・実装するための管理場所とする。

- 要求仕様の整理
- ハードウェア構成の整理
- ソフトウェア構成の整理
- ログ仕様の整理
- 試作・評価・改善履歴の蓄積
