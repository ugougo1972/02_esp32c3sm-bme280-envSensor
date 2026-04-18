# 01_HARDWARE_OVERVIEW

## 目的

本ファイルは、携帯型環境センサーロガー初号機のハードウェア構成全体を整理するための概要資料である。  
本プロジェクトでは、**頭痛発生と環境要因の相関記録**を目的とし、気圧・温度・湿度・照度・UV・行動情報を記録対象とする。

初号機では、専用基板化を急がず、**ユニバーサル基板 + 既製モジュール実装**を前提に、各機能ブロックを段階的に成立させる。

## ハードウェア方針

- 最終構成候補MCUは **Seeed Studio XIAO ESP32S3 Plus**
- Bring-up は **ESP32-WROOM系開発ボード** と **XIAO ESP32S3 Plus** を使い分けて進める
- センサ・表示・記録・UI を個別に成立させた後、統合する
- 初号機では **USB通信用USB** と **LiPo充電用USB** を分離する
- 専用基板化は後段とし、当面は **ユニバーサル基板 + 既製モジュール** で進める
- 電池容量は固定前提で急がず、運用中に容量増減できる構成を優先する
- 完成まで電流測定は実施しない
- 電池寿命の厳密評価は後段とし、まずは機能成立を優先する
- 初号機では **電源系を別基板、メイン機能系を別基板** とする二基板構成を採用する

## システム全体構成

本機は、以下の機能ブロックで構成する。

1. MCUブロック
2. 環境センサブロック
3. 時刻保持ブロック
4. 表示ブロック
5. 入力ブロック
6. 記録ブロック
7. 電源ブロック
8. 電池電圧監視ブロック

## 機能ブロック概要

### 1. MCUブロック

MCUは、センサ読出し、時刻取得、画面表示、入力読取り、CSV記録、Deep Sleep制御を担当する。

#### 採用方針

- 最終構成候補: **Seeed Studio XIAO ESP32S3 Plus**
- 評価用MCU: **ESP32-WROOM系開発ボード**

#### MCU選定理由

- I2C / SPI / GPIO の取り回し余裕がある
- OLED、microSD、複数センサ、RGB LED付きロータリーエンコーダを同時に扱いやすい
- USB接続で書込み・シリアル確認が容易
- Deep Sleep を用いた間欠動作設計に向く
- 初号機の段階では、既存評価資産を活かしやすい

### 2. 環境センサブロック

環境要因の記録対象として、以下を扱う。

#### BME280

- 温度
- 湿度
- 気圧

#### LTR390

- 照度（ALS）
- UV指標（UVS）

#### センサ構成方針

- BME280 と LTR390 は I2C 接続とする
- OLED、DS3231 と同一の I2C バスにぶら下げる
- I2C統合時にも安定動作する構成を優先する
- LTR390 は ALS / UVS 切替直後に待ち時間が必要であり、ソフト側で考慮する

### 3. 時刻保持ブロック

ログに時刻を付与するため、RTC を使用する。

#### DS3231 + AT24C32

- RTC: DS3231
- 付属EEPROM: AT24C32

#### 採用理由

- 電源断後も時刻保持が可能
- ログ記録にタイムスタンプを付与できる
- Deep Sleep 復帰後も継続した時刻管理がしやすい

#### 運用方針

- 時刻調整は **PC → USBシリアル → MCU → DS3231** で行う
- lostPower 発生時は、妥当性判定の上で自動再設定を行う
- 正常運用時は RTC を時刻基準とする

## 4. 表示ブロック

表示には OLED SSD1306 128x64 を用いる。

#### 表示内容

- 通常画面
  - 上段: 時刻欄
  - 右上: 電池アイコン表示領域
  - 本文: 温度 / 湿度、気圧、ALS / UV
  - 下段: PUSH: MENU

- MENU画面
  - VIEW
  - CLOCK
  - LOG
  - SLEEP

- CLOCK画面
  - RTC時刻表示

#### 方針

- OLED は通常時の情報確認用とする
- センサ値表示とUI遷移の確認を優先する
- ソフトは non-blocking ベースで構成し、表示処理がセンサ読出しを止めないようにする

## 5. 入力ブロック

入力には **RGB LED付きスイッチ付きロータリーエンコーダ** を使用する。

#### 入力内容

- 回転
- 押し込み
- RGB LED 制御

#### 役割

- 画面遷移
- MENU操作
- 将来の設定変更操作
- 状態表示用LED

#### 実装方針

- 回転検出は **ESP32Encoder + PCNT方式** を採用する
- 右回転 = CW、左回転 = CCW
- 5ノッチ = 5イベントを基準とする
- PUSH / RELEASE を安定検出できる構成とする
- RGB LED は各色に外付け電流制限抵抗を入れる
- LED極性は現時点では **共通アノード前提 [CHECK]** とする

## 6. 記録ブロック

記録媒体には microSD カードを使用する。

#### 目的

- CSV形式での継続ログ保存
- PCでの解析しやすさを優先

#### 記録対象例

- timestamp
- temp_c
- hum_pct
- press_hpa
- als
- uvs
- context_code
- head_code
- vbat（後段追加）

#### 方針

- microSD は SPI 接続とする
- 初号機では、ログ信頼性を優先してCSVベースとする
- ヘッダ付きCSVを採用する
- 起動ごと初期化、追記型運用を基本とする

## 7. 電源ブロック

初号機では、電源系は**成立優先・安全側・分離構成**とする。

#### 構成要素

- LiPo 3.7V 1000mAh（初期評価用）
- TP4056系充電モジュール
- XC9306系 3.3V昇降圧DC/DC
- JST-PH系バッテリコネクタ
- メイン基板接続用端子台

#### 方針

- 初号機では **USB通信用USB** と **LiPo充電用USB** を分離する
- 電池容量は固定せず、運用で増減可能な前提とする
- LiPo 1000mAh はあくまで初期評価用の暫定容量とする
- 電流測定は完成まで実施しない
- 電源寿命の厳密見積りも後回しとする
- 電源基板は **LiPo → TP4056 → XC9306** までを担当する
- メイン基板とは **3.3V / GND / VBAT_SENSE_RAW** を介して接続する

## 8. 電池電圧監視ブロック

#### 目的

- LiPo の電圧変化を MCU で監視する
- OLED 上の電池アイコン表示および低電圧判定へつなげる

#### 方針

- 測定対象は **TP4056 OUT+ / OUT-** とする
- 分圧回路は **メイン基板側** に実装する
- 初期値は **100kΩ / 100kΩ の 1:1 分圧** を第一候補とする
- 必要時のみ ADC 安定化用コンデンサを追加する

## 現時点の接続方式

### I2C

- BME280
- LTR390
- OLED SSD1306
- DS3231 + AT24C32

### SPI

- microSD module

### GPIO入力 / 出力

- Rotary Encoder A / B / Push
- RGB LED R / G / B

## 現時点のXIAO GPIO概要

### I2C

- D4(GPIO5) = SDA
- D5(GPIO6) = SCL

### SPI（microSD）

- D2(GPIO3) = CS
- D8(GPIO7) = SCK
- D9(GPIO8) = MISO
- D10(GPIO9) = MOSI

### RGB Encoder

- D0(GPIO1) = Encoder A
- D1(GPIO2) = Encoder B
- D3(GPIO4) = Encoder SW
- D11(GPIO38) = LED R
- D12(GPIO39) = LED G
- D13(GPIO40) = LED B

### 予約

- D16(GPIO10) = Battery voltage monitor 候補予約
- D6(GPIO43) = UART TX reserve
- D7(GPIO44) = UART RX reserve

詳細は `04_PIN_ASSIGNMENT.md` を参照する。

## Bring-upと評価の考え方

本プロジェクトでは、以下の順序を重視する。

1. MCU単体確認
2. 各部品の1:1接続確認
3. I2C統合確認
4. microSD統合確認
5. RTC統合確認
6. periodic logger 確認
7. Deep Sleep logger 確認
8. UI統合確認
9. 電源系単体確認
10. LiPo駆動での MCU 起動確認
11. Battery voltage monitor 実装

この順序により、不具合発生時の切り分けを容易にする。

## 現時点で成立している主なハードウェア機能

- ESP32-WROOM 書込み / シリアル通信
- XIAO ESP32S3 Plus 書込み / シリアル通信 / Lチカ
- BME280 読出し
- LTR390 読出し
- OLED 表示
- DS3231 時刻保持
- microSD CSV記録
- periodic logger
- Deep Sleep logger
- VIEW / MENU / CLOCK / LOG / SLEEP 基本UI
- RGB LED付きロータリーエンコーダの回転 / 押下 / RGB LED 制御
- LiPo → TP4056 → XC9306 → XIAO 3.3V給電
- LiPo駆動での Blink 動作

## 初号機で未確定の要素

以下は現時点で未確定、または後段で確定する。

- 電池電圧監視回路の本実装
- 残量表示方式
- 電源基板とメイン基板の最終レイアウト
- ユニバーサル基板上の最終配線
- ケース / 筐体構成
- context_code / head_code 入力UIの最終形
- 充電中常時動作の扱い
- 完成後の連続稼働時間評価

## 本ファイルの位置付け

本ファイルは、個別部品一覧やGPIO詳細より上位の、**ハードウェア全体像を整理するための概要ファイル**である。  
詳細は以下を参照する。

- `02_PARTS_LIST.md`
- `03_POWER_DESIGN.md`
- `04_PIN_ASSIGNMENT.md`
- `05_WIRING_DIAGRAM.md`
- `06_ENCLOSURE.md`

## ステータス

- [ACTIVE] 初号機ハードウェア概要として有効
- [ACTIVE] XIAO ESP32S3 Plus を最終構成候補MCUとする
- [ACTIVE] ユニバーサル基板 + 既製モジュール前提
- [ACTIVE] LiPo → TP4056 → XC9306 → XIAO の最小電源経路成立済み
- [ACTIVE] 電源系別基板方針を採用
- [CHECK] Battery voltage monitor 本実装未完了
- [CHECK] 筐体設計未確定
