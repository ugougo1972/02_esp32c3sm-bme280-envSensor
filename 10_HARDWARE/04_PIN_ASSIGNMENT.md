# 04_PIN_ASSIGNMENT

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるGPIO割当方針を整理するためのものである。  
現段階では、**Seeed Studio XIAO ESP32S3 Plus を最終構成候補MCU**としつつ、**Bring-upで実績のある配線と、RGB LED付きロータリーエンコーダ対応**を反映した最終GPIO割当案の母体とする。

## 基本方針

- 最終構成候補MCUは **Seeed Studio XIAO ESP32S3 Plus**
- 評価用MCUで成立した機能のうち、XIAO上で動作確認済みの割当を優先採用する
- 初号機では、配線変更リスクを抑えるため、**既に動作実績のある配線を極力維持する**
- USB通信用USBとLiPo充電用USBは分離する
- 電池容量は固定前提で急がず、運用中に容量増減できる前提で進める
- 完成まで電流測定は実施しない
- 追加機能用GPIOは、無理に現時点で使い切らず、**予約**として残す
- Battery voltage monitor は **GPIO10 を予約扱いのまま保留** とする

## MCU

- MCU: Seeed Studio XIAO ESP32S3 Plus

## 最終GPIO割当案（初号機）

### I2C

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 1 | I2C SDA | D4 | GPIO5 | BME280 / LTR390 / SSD1306 / DS3231 | 採用 | XIAO上で同時動作確認済み |
| 2 | I2C SCL | D5 | GPIO6 | BME280 / LTR390 / SSD1306 / DS3231 | 採用 | XIAO上で同時動作確認済み |

### SPI（microSD）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 3 | SPI CS | D2 | GPIO3 | microSD module CS | 採用 | 単体試験 / 統合試験で使用 |
| 4 | SPI SCK | D8 | GPIO7 | microSD module SCK | 採用 | 単体試験 / 統合試験で使用 |
| 5 | SPI MISO | D9 | GPIO8 | microSD module MISO | 採用 | 単体試験 / 統合試験で使用 |
| 6 | SPI MOSI | D10 | GPIO9 | microSD module MOSI | 採用 | 単体試験 / 統合試験で使用 |

### RGB LED付きロータリーエンコーダ / UI入力

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 7 | Encoder A | D0 | GPIO1 | Rotary Encoder A | 採用 | XIAO上で確認済み |
| 8 | Encoder B | D1 | GPIO2 | Rotary Encoder B | 採用 | XIAO上で確認済み |
| 9 | Encoder SW | D3 | GPIO4 | Rotary Encoder SW | 採用 | PUSH / RELEASE 確認済み |
| 10 | LED R | D11 | GPIO38 | Encoder LED R | 採用 | LED制御確認済み |
| 11 | LED G | D12 | GPIO39 | Encoder LED G | 採用 | LED制御確認済み |
| 12 | LED B | D13 | GPIO40 | Encoder LED B | 採用 | LED制御確認済み |
| 13 | GND | GND | - | Encoder C または GND系 | 採用 | LED極性は別紙確認 |

### 予約GPIO

| 項番 | 機能候補 | XIAO端子 | GPIO | 状況 | 備考 |
|------|----------|----------|------|------|------|
| 14 | Battery voltage monitor candidate | D16 | GPIO10 | 予約 | 現時点では保留。分圧回路はメイン基板側で実装予定 |
| 15 | UART TX reserve | D6 | GPIO43 | 予約 | 将来デバッグ用候補 |
| 16 | UART RX reserve | D7 | GPIO44 | 予約 | 将来デバッグ用候補 |
| 17 | Future expansion | D14 | GPIO41 | 予約 | 未確認 |
| 18 | Future expansion | D15 | GPIO42 | 予約 | 未確認 |

## 接続対象一覧

### I2Cバス接続対象

- BME280
- LTR390
- OLED SSD1306 128x64
- DS3231 + AT24C32

### SPI接続対象

- microSD module

### UI接続対象

- RGB LED付きスイッチ付きロータリーエンコーダ

## 割当理由

### 1. 既存実績を優先するため

現時点で以下が成立している。

- XIAO上で BME280 / LTR390 / OLED / DS3231 の同時動作
- microSD 単体接続およびCSV追記
- RTC + BME280 + LTR390 + microSD 統合ログ
- Deep Sleep logger による継続記録
- RGB LED付きロータリーエンコーダの回転 / 押下 / LED制御

このため、初号機では配線変更による新規不具合を避け、**実績のある割当をそのまま採用する**。

### 2. 機能ごとの分離が明確なため

- I2C系は D4 / D5 に固定
- microSD用SPIは D2 / D8 / D9 / D10 に固定
- UI入力は D0 / D1 / D3 に固定
- LED出力は D11 / D12 / D13 に固定

役割分離が明確であり、ソフトウェア側でも管理しやすい。

### 3. 将来拡張の逃げ道を残すため

現段階では、電池電圧監視、追加ボタン、将来のデバッグUARTなどの実装有無が未確定である。  
そのため、未使用GPIOは無理に消費せず、**予約**として残す。

## Deep Sleepとの関係

- 現時点の Deep Sleep logger は **timer wakeup** を基準として成立している
- 起床ごとに RTC / BME280 / LTR390 / microSD を再初期化する方式を採用している
- 30秒周期 / 60秒周期の動作実績あり
- 将来的に、ボタン押下による wakeup を追加検討する余地はあるが、現時点では未実施

## 注意事項

### 1. Battery voltage monitor は未実装

D16(GPIO10) は候補予約のみであり、以下は未確定である。

- 分圧抵抗値の最終値
- 常時接続時の消費電流影響
- 表示用の残量換算方式
- 低電圧判定しきい値
- GPIO10 を本当に使用するか、別ADCピンへ移すか

よって、現時点では **「予約」扱い** とする。

### 2. 電源系は成立済みだが本統合は後段

以下は成立済みである。

- TP4056系充電モジュール
- XC9306系 3.3V昇降圧DC/DC
- LiPo 3.7V 1000mAh
- LiPo → TP4056 → XC9306 → XIAO の最小給電

ただし、I2C / SPI / UI を含む **電池駆動での長時間本統合** は後段確認とする。

### 3. 初号機ではUSBを分離する

初号機では、以下の方針を採る。

- USB通信用USB
- LiPo充電用USB

この2系統は分離し、一本化は行わない。

## 現時点のGPIO使用状況まとめ

| 区分 | 使用GPIO |
|------|----------|
| I2C | GPIO5, GPIO6 |
| SPI(microSD) | GPIO3, GPIO7, GPIO8, GPIO9 |
| UI入力 | GPIO1, GPIO2, GPIO4 |
| UI LED | GPIO38, GPIO39, GPIO40 |
| 予約 | GPIO10, GPIO41, GPIO42, GPIO43, GPIO44 |

## 今後の更新対象

以下の条件が確定した時点で、本ファイルを更新する。

- 電池電圧監視回路の採否確定
- 追加ボタンやブザー等の搭載有無確定
- XIAO ESP32S3 Plus 実装形態の確定
- ユニバーサル基板上の実配線確定
- 電源系（TP4056 / XC9306 / LiPo）の本統合試験結果反映

## ステータス

- [ACTIVE] 初号機向けGPIO割当案として有効
- [ACTIVE] XIAO ESP32S3 Plus 前提
- [ACTIVE] RGB LED付きロータリーエンコーダ対応版
- [CHECK] Battery voltage monitor 用 ADC ピン最終確定待ち
- [CHECK] 予約GPIOの最終用途未確定
