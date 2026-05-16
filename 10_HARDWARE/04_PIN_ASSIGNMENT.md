# 04_PIN_ASSIGNMENT

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるGPIO割当を整理するための資料である。  
**Seeed Studio XIAO ESP32S3 Plus**を最終構成MCUとし、**全GPU割り当て確定版**を示す。

---

## 基本方針

- 最終構成MCUは **Seeed Studio XIAO ESP32S3 Plus**
- 評価用MCUで成立した機能のうち、XIAO上で動作確認済みの割当を採用
- 初号機では、配線変更リスクを抑えるため、**既に動作実績のある配線を維持**
- USB通信用USBとLiPo充電用USBは分離する
- 電池容量は固定前提で急がず、運用中に容量増減できる前提で進める
- 完成まで電流測定は実施しない

---

## MCU

- MCU: Seeed Studio XIAO ESP32S3 Plus

---

## GPIO割当（確定版）

### I2C（D4 / D5）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 1 | I2C SDA | D4 | GPIO5 | BME280 / LTR390 / OLED / DS3231 / MCP23017 | ✅ 確定 | XIAO上で同時動作確認済み |
| 2 | I2C SCL | D5 | GPIO6 | BME280 / LTR390 / OLED / DS3231 / MCP23017 | ✅ 確定 | XIAO上で同時動作確認済み |

---

### SPI（microSD）（D2 / D8 / D9 / D10）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 3 | SPI CS | D7 | GPIO44 | microSD module CS | ✅ 確定 | UART RX兼用・USB CDC主体で通常GPIO利用可 |
| 4 | SPI SCK | D8 | GPIO7 | microSD module SCK | ✅ 確定 | 単体試験 / 統合試験で使用 |
| 5 | SPI MISO | D9 | GPIO8 | microSD module MISO | ✅ 確定 | 単体試験 / 統合試験で使用 |
| 6 | SPI MOSI | D10 | GPIO9 | microSD module MOSI | ✅ 確定 | 単体試験 / 統合試験で使用 |

---

### Rotary Encoder入力（D0 / D1 / D2）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 7 | Encoder A | D0 | GPIO1 | Rotary Encoder A | ✅ 確定 | RTC GPIO・EXT0復帰可 |
| 8 | Encoder B | D1 | GPIO2 | Rotary Encoder B | ✅ 確定 | RTC GPIO対応 |
| 9 | Encoder SW | D2 | GPIO3 | Rotary Encoder SW | ✅ 確定 | RTC GPIO・EXT1復帰可 |

---

### RGB LED制御（MCP23017経由）

| 項番 | 機能 | MCP23017ピン | 用途 | 状況 | 備考 |
|------|------|-------------|------|------|------|
| 10 | LED R | GPA0 | Encoder LED R | ✅ 確定 | 100Ω電流制限抵抗経由 |
| 11 | LED G | GPA1 | Encoder LED G | ✅ 確定 | 100Ω電流制限抵抗経由 |
| 12 | LED B | GPA2 | Encoder LED B | ✅ 確定 | 100Ω電流制限抵抗経由 |

**MCP23017制御**：I2C経由（0x20アドレス）

---

### 電源制御（D3 / D6）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 13 | Battery ADC | D3 | GPIO4 | VBAT_RAW（分圧後） | ✅ 確定 | ADC1_CH3・RTC GPIO対応 |
| 14 | TPS63802 EN | D6 | GPIO43 | AE-TPS63802 4P EN | ✅ 確定 | RTC GPIO非対応→R_EN(100kΩ)プルアップで対策 |

---

### 未使用予約

| 項番 | 機能候補 | XIAO端子 | GPIO | 状況 | 備考 |
|------|----------|----------|------|------|------|
| 15 | 予約 | D16 | GPIO10 | 予約 | 将来拡張用 |
| 16 | 予約 | D14 | GPIO41 | 予約 | 将来拡張用 |
| 17 | 予約 | D15 | GPIO42 | 予約 | 将来拡張用 |

---

## 接続対象デバイス一覧

### I2Cバス接続対象

- BME280（温度・湿度・気圧・0x76）
- LTR390（照度・UV・0x53）
- OLED SSD1306 128x64（表示・0x3C）
- DS3231 + AT24C32（RTC・EEPROM・0x68/0x57）
- **MCP23017**（**I2C GPIO Expander・LED R/G/B制御・0x20**）

### SPI接続対象

- microSD module（記録・D7=CS / D8=SCK / D9=MISO / D10=MOSI）

### GPIO入力対象

- Rotary Encoder（回転・押下・D0=A / D1=B / D2=SW）

### GPIO出力対象

- RGB LED（MCP23017経由制御）

### ADC入力対象

- Battery Voltage Monitor（D3=GPIO4・ADC1_CH3）

---

## GPIO使用状況サマリー

| 区分 | 使用GPIO | ピン数 | 状態 |
|------|----------|--------|------|
| I2C | GPIO5, GPIO6 | 2ピン | ✅ 確定 |
| SPI(microSD) | GPIO3, GPIO7, GPIO8, GPIO9 | 4ピン | ✅ 確定 |
| Encoder入力 | GPIO1, GPIO2, GPIO4 | 3ピン | ✅ 確定 |
| Power制御 | GPIO43 | 1ピン | ✅ 確定 |
| RGB LED制御 | MCP23017 GPA0/GPA1/GPA2 | I2C経由 | ✅ 確定 |
| 予約（今後） | GPIO10, GPIO41, GPIO42 | 3ピン | 予約 |
| **計** | **合計14ピン割り当て** | - | - |

---

## 割当理由

### 1. 既存実績を優先するため

現時点で以下が成立している。

- XIAO上で BME280 / LTR390 / OLED / DS3231 の同時動作
- microSD 単体接続およびCSV追記
- RTC + BME280 + LTR390 + microSD 統合ログ
- Deep Sleep logger による継続記録（60秒・30秒周期確認済み）
- RGB LED付きロータリーエンコーダの回転 / 押下検出
- **MCP23017経由のRGB LED制御**

このため、初号機では配線変更による新規不具合を避け、**実績のある割当をそのまま採用する**。

### 2. 機能ごとの分離が明確なため

- I2C系は D4 / D5 に固定
- microSD用SPIは D7 / D8 / D9 / D10 に固定
- Encoder入力は D0 / D1 / D2 に固定
- RGB LED出力は MCP23017 経由（I2C）
- 電源制御は D3（ADC） / D6（EN） に固定

役割分離が明確であり、ソフトウェア側でも管理しやすい。

### 3. Deep Sleep対応を考慮したため

- GPIO1/2/3（Encoder）：RTC GPIO対応→EXT0/EXT1復帰可
- GPIO4（Battery ADC）：RTC GPIO対応
- GPIO43（TPS63802 EN）：RTC GPIO非対応→回路側プルアップ対策
- microSD SPI：Deep Sleep前にCS=HIGH固定・SPI.end()実施

---

## Deep Sleepとの関係

### 周期復帰方式

- 現時点の Deep Sleep logger は **timer wakeup** を基準として成立
- 起床ごとに RTC / BME280 / LTR390 / microSD を再初期化する方式を採用
- 30秒周期 / 60秒周期の動作実績あり

### Encoder復帰対応

- GPIO1/2/3はRTC GPIO対応
- EXT0/EXT1によるEncoder_SW(GPIO3)押下復帰が可能（将来実装予定）

### 電源制御

- GPIO43(TPS63802 EN)によるDeep Sleep時電源遮断が可能
- R_EN(100kΩ)プルアップで Hi-Z化対策完了

### microSD CS処理

- Deep Sleep前に以下を実施：
  ```cpp
  pinMode(PIN_SPI_CS, OUTPUT);    // D7(GPIO44)
  digitalWrite(PIN_SPI_CS, HIGH); // CS=HIGH
  SPI.end();                       // SPI終了
  ```

---

## メイン基板配置との対応

### メイン基板座標

| 部品 | 座標 | GPIO割当 |
|-----|------|---------|
| XIAO | (16,02)～(22,08) | D0～D10・D16確定 |
| MCP23017 | (21,13)～(26,26) | I2C(GPIO5/6)経由制御 |
| 分圧回路 | (13,03)～(13,06) | GPIO4(ADC1_CH3)入力 |
| R_EN | (12,08)～(14,08) | GPIO43(EN制御)対策 |
| 端子台 | (31,25)～(34,27) | GND/3.3V/VBAT_RAW/EN |

---

## 電源基板のGPIO関連インターフェース

初号機では、電源基板とメイン基板を端子台で接続する。

| ライン | 信号 | 接続先GPIO | 用途 |
|--------|------|-----------|------|
| 1 | 3.3V | XIAO 3V3 | センサ・RTC・OLED・microSD給電 |
| 2 | GND | XIAO GND | GND共通 |
| 3 | VBAT_RAW | D3(GPIO4) ADC入力 | Battery voltage monitor 測定用 |
| 4 | EN | D6(GPIO43) | AE-TPS63802 EN制御（Deep Sleep用） |

---

## 注意事項

### 1. Battery voltage monitor は実装済み方針

- D3(GPIO4)：ADC1_CH3・RTC GPIO対応
- 分圧回路：メイン基板側 (13,03)～(13,06) 座標に実装
- R1/R2：100kΩ × 2（1:1分圧）
- 分圧後電圧：約1.85V（満充電時）
- 安定化C：必要に応じて ADC中点-GND間に 0.1µF 追加可

### 2. AE-TPS63802 EN制御GPIO は確定

- D6(GPIO43)：制御対象
- RTC GPIO非対応のため、メイン基板側に R_EN(100kΩ) プルアップを実装済み
- R_EN配置：(12,08)～(14,08) メイン基板座標
- 入力側：3.3Vバス（スズメッキ線経由）
- 出力側：D6(GPIO43) + 端子台EN合流点

### 3. RGB LED制御は MCP23017 経由に変更

元々XIAO裏面JTAGランド(GPIO38/39/40)を予定していたが、ユニバーサル基板実装後アクセス不可のため、**MCP23017（I2C GPIO Expander）** で代替確定。

- MCP23017アドレス：0x20
- GPA0/GPA1/GPA2：RGB LED R/G/B制御
- 各色に100Ω電流制限抵抗を配置
- LED極性：共通カソード

### 4. 電源系は成立済みだが本統合は後段

以下は成立済みである。

- TP4056系充電モジュール
- AE-TPS63802 3.3V昇降圧DC/DC（EN端子付き）
- LiPo 3.7V 1000mAh
- LiPo → TP4056 → AE-TPS63802 → XIAO の最小給電

ただし、I2C / SPI / UI を含む **電池駆動での長時間本統合** は後段確認とする。

### 5. 初号機ではUSBを分離する

初号機では、以下の方針を採る。

- USB通信用USB（XIAO）
- LiPo充電用USB（TP4056）

この2系統は分離し、一本化は行わない。

---

## 現時点のGPIO使用状況まとめ

| 区分 | 使用GPIO | 端子 |
|------|----------|------|
| I2C SDA | GPIO5 | D4 |
| I2C SCL | GPIO6 | D5 |
| Encoder A | GPIO1 | D0 |
| Encoder B | GPIO2 | D1 |
| Encoder SW | GPIO3 | D2 |
| Battery ADC | GPIO4 | D3 |
| TPS63802 EN | GPIO43 | D6 |
| SPI CS | GPIO44 | D7 |
| SPI SCK | GPIO7 | D8 |
| SPI MISO | GPIO8 | D9 |
| SPI MOSI | GPIO9 | D10 |
| RGB LED | GPA0/GPA1/GPA2 | MCP23017 I2C |
| 予約 | GPIO10 | D16 |
| 予約 | GPIO41 | D14 |
| 予約 | GPIO42 | D15 |

---

## 今後の更新対象

以下の条件が確定した時点で、本ファイルを更新する。

- 電池電圧監視回路の本実装確認
- AE-TPS63802 EN制御実装確認
- MCP23017 I2C統合確認
- RGB LED制御動作確認
- Deep Sleep ロガー本統合確認
- ユニバーサル基板上の実配線確認

---

## ステータス

- [COMPLETE] **初号機向けGPIO割当確定版**
- [COMPLETE] XIAO ESP32S3 Plus 前提
- [COMPLETE] RGB LED付きロータリーエンコーダ対応（MCP23017経由）
- [COMPLETE] メイン基板配置座標確定
- [COMPLETE] AE-TPS63802 EN端子制御用GPIO確定（GPIO43）
- [COMPLETE] Battery voltage monitor ADC ピン確定（GPIO4）
- [COMPLETE] 全GPIO割り当て確定
- [COMPLETE] Deep Sleep対応設計確定（R_EN・SPI.end()）
- [COMPLETE] メイン基板実装座標確定
- [IN PROGRESS] メイン基板実装（分圧回路・R_EN・MCP23017から開始）
- [PENDING] EN制御・ADC動作確認
- [PENDING] 電池駆動での統合試験
