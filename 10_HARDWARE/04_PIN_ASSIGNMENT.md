# 04_PIN_ASSIGNMENT

## 目的

本ファイルは、携帯型環境センサーロガー初号機におけるGPIO割当方針を整理するためのものである。  
現段階では、**Seeed Studio XIAO ESP32S3 Plus を最終構成候補MCU**としつつ、**Bring-upで実績のある配線と、RGB LED付きロータリーエンコーダ対応**を反映した最終GPIO割当案を示す。

## 基本方針

- 最終構成候補MCUは **Seeed Studio XIAO ESP32S3 Plus**
- 評価用MCUで成立した機能のうち、XIAO上で動作確認済みの割当を優先採用する
- 初号機では、配線変更リスクを抑えるため、**既に動作実績のある配線を極力維持する**
- USB通信用USBとLiPo充電用USBは分離する
- 電池容量は固定前提で急がず、運用中に容量増減できる前提で進める
- 完成まで電流測定は実施しない
- 追加機能用GPIOは、無理に現時点で使い切らず、**予約**として残す
- Battery voltage monitor は **GPIO10 を予約扱いのまま保留** とする
- AE-TPS63802 EN制御用GPIOは **未確定・保留** とする

## MCU

- MCU: Seeed Studio XIAO ESP32S3 Plus

## 最終GPIO割当案（初号機）

### I2C（D4 / D5）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 1 | I2C SDA | D4 | GPIO5 | BME280 / LTR390 / SSD1306 / DS3231 | ✅ 採用 | XIAO上で同時動作確認済み |
| 2 | I2C SCL | D5 | GPIO6 | BME280 / LTR390 / SSD1306 / DS3231 | ✅ 採用 | XIAO上で同時動作確認済み |

### SPI（microSD）（D2 / D8 / D9 / D10）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 3 | SPI CS | D2 | GPIO3 | microSD module CS | ✅ 採用 | 単体試験 / 統合試験で使用 |
| 4 | SPI SCK | D8 | GPIO7 | microSD module SCK | ✅ 採用 | 単体試験 / 統合試験で使用 |
| 5 | SPI MISO | D9 | GPIO8 | microSD module MISO | ✅ 採用 | 単体試験 / 統合試験で使用 |
| 6 | SPI MOSI | D10 | GPIO9 | microSD module MOSI | ✅ 採用 | 単体試験 / 統合試験で使用 |

### RGB LED付きロータリーエンコーダ / UI入力（D0 / D1 / D3 / D11 / D12 / D13）

| 項番 | 機能 | XIAO端子 | GPIO | 接続先 | 状況 | 備考 |
|------|------|----------|------|--------|------|------|
| 7 | Encoder A | D0 | GPIO1 | Rotary Encoder A | ✅ 採用 | XIAO上で確認済み |
| 8 | Encoder B | D1 | GPIO2 | Rotary Encoder B | ✅ 採用 | XIAO上で確認済み |
| 9 | Encoder SW | D3 | GPIO4 | Rotary Encoder SW | ✅ 採用 | PUSH / RELEASE 確認済み |
| 10 | LED R | D11 | GPIO38 | Encoder LED R | ✅ 採用 | LED制御確認済み |
| 11 | LED G | D12 | GPIO39 | Encoder LED G | ✅ 採用 | LED制御確認済み |
| 12 | LED B | D13 | GPIO40 | Encoder LED B | ✅ 採用 | LED制御確認済み |
| 13 | GND | GND | - | Encoder C または GND系 | ✅ 採用 | LED極性は別紙確認 |

### 予約GPIO（非割り当て）

| 項番 | 機能候補 | XIAO端子 | GPIO | 状況 | 備考 |
|------|----------|----------|------|------|------|
| 14 | Battery voltage monitor（分圧後ADC） | D16 | GPIO10 | 予約 | 現時点では保留。分圧回路はメイン基板側で実装予定 |
| 15 | AE-TPS63802 EN制御（未確定） | TBD | TBD | 予約 | 未確定・後段で決定予定 |
| 16 | UART TX reserve | D6 | GPIO43 | 予約 | 将来デバッグ用候補 |
| 17 | UART RX reserve | D7 | GPIO44 | 予約 | 将来デバッグ用候補 |
| 18 | Future expansion | D14 | GPIO41 | 予約 | 未確認 |
| 19 | Future expansion | D15 | GPIO42 | 予約 | 未確認 |

## 接続対象一覧

### I2Cバス接続対象

- BME280（温度・湿度・気圧）
- LTR390（照度・UV）
- OLED SSD1306 128x64（表示）
- DS3231 + AT24C32（RTC・EEPROM）

### SPI接続対象

- microSD module（記録）

### UI接続対象

- RGB LED付きスイッチ付きロータリーエンコーダ（回転・押下・LED）

## 電源系GPIO（今後の確定対象）

### AE-TPS63802 EN端子制御用GPIO

| 項目 | 状態 | 備考 |
|------|------|------|
| 制御対象 | AE-TPS63802 4P (EN) | Deep Sleep時の電源遮断 |
| 制御方式 | HIGH = 動作 / LOW = シャットダウン | TPS63802内部設計に従う |
| GPIO候補 | 未確定 | 端子台経由でメイン基板へ引き出し予定 |
| 確定予定時期 | 電源基板実装完了後 | 分圧回路実装時に同時確定 |

### Battery voltage monitor ADC入力

| 項目 | 状態 | 備考 |
|------|------|------|
| ADC対象 | TP4056 OUT+（分圧後1.85V@満充電） | メイン基板側に分圧回路実装 |
| GPIO候補 | GPIO10（D16） | 現時点では予約・未確定 |
| 分圧比 | 1:2（100kΩ+100kΩ） | 安全範囲内 |
| 確定予定時期 | 分圧回路実装時 | GPIO10以外への変更可能性あり |

## GPIO使用状況サマリー

| 区分 | 使用GPIO | ピン数 | 状態 |
|------|----------|--------|------|
| I2C | GPIO5, GPIO6 | 2ピン | ✅ 確定 |
| SPI(microSD) | GPIO3, GPIO7, GPIO8, GPIO9 | 4ピン | ✅ 確定 |
| UI入力 | GPIO1, GPIO2, GPIO4 | 3ピン | ✅ 確定 |
| UI LED | GPIO38, GPIO39, GPIO40 | 3ピン | ✅ 確定 |
| 予約（電源） | GPIO10他 | 複数 | 🔄 検討中 |
| 予約（今後） | GPIO41, GPIO42, GPIO43, GPIO44 | 4ピン | 予約 |
| **計** | **合計14ピン割り当て / 予約中** | - | - |

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
- AE-TPS63802 EN制御により、Deep Sleep時の3.3V系完全遮断が可能（後段実装予定）

## 注意事項

### 1. Battery voltage monitor は未実装

D16(GPIO10) は候補予約のみであり、以下は未確定である。

- 分圧抵抗値の最終値
- 常時接続時の消費電流影響
- 表示用の残量換算方式
- 低電圧判定しきい値
- GPIO10 を本当に使用するか、別ADCピンへ移すか

よって、現時点では **「予約」扱い** とする。

### 2. AE-TPS63802 EN制御GPIO は未確定

以下は未確定である。

- EN制御用GPIO の選定
- GPIO から TPS63802 EN への配線経路
- 端子台との接続方法

電源基板実装完了後に決定予定。

### 3. 電源系は成立済みだが本統合は後段

以下は成立済みである。

- TP4056系充電モジュール
- AE-TPS63802 3.3V昇降圧DC/DC（EN端子付き）
- LiPo 3.7V 1000mAh
- LiPo → TP4056 → AE-TPS63802 → XIAO の最小給電

ただし、I2C / SPI / UI を含む **電池駆動での長時間本統合** は後段確認とする。

### 4. 初号機ではUSBを分離する

初号機では、以下の方針を採る。

- USB通信用USB（XIAO）
- LiPo充電用USB（TP4056）

この2系統は分離し、一本化は行わない。

## 電源基板のGPIO関連インターフェース

初号機では、電源基板とメイン基板を端子台で接続する。

| ライン | 信号 | 接続先 | 用途 |
|--------|------|--------|------|
| 1 | 3.3V | XIAO 3V3 | センサ・RTC・OLED・microSD給電 |
| 2 | GND | XIAO GND | GND共通 |
| 3 | VBAT_RAW | 分圧回路 R1上端 | Battery voltage monitor 測定用 |
| 4 | EN | XIAO GPIO（未確定） | AE-TPS63802 EN制御（Deep Sleep用） |

## 現時点のGPIO使用状況まとめ

| 区分 | 使用GPIO |
|------|----------|
| I2C | GPIO5(D4=SDA), GPIO6(D5=SCL) |
| SPI(microSD) | GPIO3(D2=CS), GPIO7(D8=SCK), GPIO8(D9=MISO), GPIO9(D10=MOSI) |
| UI入力 | GPIO1(D0=A), GPIO2(D1=B), GPIO4(D3=SW) |
| UI LED | GPIO38(D11=R), GPIO39(D12=G), GPIO40(D13=B) |
| 予約 | GPIO10(D16), GPIO41(D14), GPIO42(D15), GPIO43(D6), GPIO44(D7) |

## 今後の更新対象

以下の条件が確定した時点で、本ファイルを更新する。

- 電池電圧監視回路の採否確定
- AE-TPS63802 EN制御用GPIO確定
- 追加ボタンやブザー等の搭載有無確定
- XIAO ESP32S3 Plus 実装形態の確定
- ユニバーサル基板上の実配線確定
- 電源系（TP4056 / AE-TPS63802 / LiPo）の本統合試験結果反映

## ステータス

- [ACTIVE] 初号機向けGPIO割当案として有効
- [ACTIVE] XIAO ESP32S3 Plus 前提
- [ACTIVE] RGB LED付きロータリーエンコーダ対応版
- [ACTIVE] AE-TPS63802 EN端子の電源基板インターフェース確定
- [CHECK] Battery voltage monitor 用 ADC ピン最終確定待ち
- [CHECK] AE-TPS63802 EN制御GPIO未確定
- [CHECK] 予約GPIOの最終用途未確定
- [CHECK] 電源基板 ← → メイン基板のGPIO引き出し詳細未確定
