# 02_SYSTEM_OVERVIEW

## 目的

本ファイルは、携帯型環境センサーロガー初号機のシステム全体像を整理するための概要資料である。  
初号機では、**頭痛発生と環境要因の相関記録**を目的とし、まずは **安定した周期記録の成立** を最優先とする。

---

## システム概要

本システムは、携帯型の環境センサーロガーであり、一定時間ごとに環境データを取得し、microSDカードへ CSV 形式で記録する装置である。  
必要に応じて OLED 表示および Rotary Encoder による操作を行う。  
また、環境データだけでは分からない補助情報として、`context_code` および `head_code` を扱える構成とする。

---

## システム目的

本システムの目的は以下の通り。

1. 気圧・温湿度・照度・UV を継続記録する
2. DS3231 による時刻基準でログを残す
3. `context_code` により行動状態を補助記録する
4. `head_code` により頭痛状態を補助記録する
5. 環境要因と体調変化の相関を後から解析できるようにする

---

## 取得データ

本装置で取得・保持するデータは以下とする。

| 項番 | データ | 保存項目名 | 取得元 | 備考 |
|------|--------|------------|--------|------|
| D-1 | 日付 | `date` | DS3231 | YYYY-MM-DD |
| D-2 | 時刻 | `time` | DS3231 | HH:MM:SS |
| D-3 | 温度 | `temp_c` | BME280 | ℃ |
| D-4 | 湿度 | `hum_pct` | BME280 | % |
| D-5 | 気圧 | `press_hpa` | BME280 | hPa |
| D-6 | 照度センサ値 | `als` | LTR390 | ALS raw/count |
| D-7 | UVセンサ値 | `uvs` | LTR390 | UVS raw/count |
| D-8 | 行動状態 | `context_code` | 手動入力 | `04_ctx.md` 準拠 |
| D-9 | 頭痛状態 | `head_code` | 手動入力 | `05_head.md` 準拠 |
| D-10 | 電池電圧 | `vbat` | ADC | 分圧回路経由・メイン基板側実装 |

### 取得データに関する方針

- `als` / `uvs` は現時点の実装実績に合わせて採用する
- `context_code` の既定値は **0**
- `head_code` の既定値は **0**
- `vbat` は分圧回路(100kΩ+100kΩ)と XIAO D3(GPIO4・ADC1_CH3)で実装
- 推測値を自動設定しない

---

## システム構成

### ハードウェア構成

| 区分 | 部品 | 備考 |
|------|------|------|
| MCU | Seeed Studio XIAO ESP32S3 Plus | 最終構成 |
| 評価用MCU | ESP32-WROOM系開発ボード | Bring-up用 |
| 温湿度・気圧 | BME280 | I2C・基板直付け |
| 照度・UV | LTR390 | I2C・ケース直付け |
| RTC | DS3231 | I2C・基板直付け |
| 表示 | OLED SSD1306 | I2C・ケース直付け |
| 入力 | RGB LED付きスイッチ付きロータリーエンコーダ | GPIO+MCP23017・ケース直付け |
| GPIO拡張 | MCP23017 | I2C・LED R/G/B制御用 |
| 記録 | microSD | SPI・基板実装 |
| 充電 | TP4056系充電モジュール | USB-C入力 |
| 電源IC | AE-TPS63802（昇降圧スイッチング） | 3.3V出力・EN端子付き・Deep Sleep制御用 |
| 電池 | LiPo | 容量は運用で変更可能 |

### ソフトウェア構成

| 機能 | 内容 |
|------|------|
| センサ読取 | I2C（BME280・LTR390・DS3231・MCP23017） |
| SD保存 | SPI・毎周期初期化・追記保存 |
| 時刻取得 | RTC(DS3231)・妥当性判定・復旧処理 |
| 表示 | OLED・操作時点灯・5秒後消灯 |
| 入力 | Rotary Encoder・EXT0/EXT1復帰対応 |
| LED制御 | MCP23017経由（I2C・GPA0/1/2） |
| 省電力 | Deep Sleep + GPIO43(TPS63802_EN)制御 |
| 通信 | USB CDC Serial |
| 電池電圧取得 | ADC + 分圧回路(100kΩ+100kΩ) |

---

## 電源アーキテクチャ（確定）

### 電源経路

```
LiPo
  │
  └─ JST（2pin）  AWG24
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
| EN | TPS63802 EN端子 | XIAO D6(GPIO43) |

### 電源基板コンデンサ構成

| 部品 | 容量 | 種別 | 接続 |
|------|------|------|------|
| C1 | 0.1uF | MLCC | TP4056 OUT+ ↔ OUT- 間 |
| C2 | 47uF | 電解（長足→VOUT側） | TPS63802 VOUT ↔ GND 間 |
| C3 | 0.1uF | MLCC | TPS63802 VOUT ↔ GND 間（C2並列） |

### 基板分割方針（確定）

- **電源基板（72mm×47mm）**: LiPo / JST / TP4056 / AE-TPS63802 / C1/C2/C3 / 端子台
- **メイン基板（95mm×72mm）**: XIAO / BME280 / DS3231 / microSD / MCP23017 / JST群 / 分圧回路 / R_EN(100kΩ)
- LiPo ↔ TP4056: JST コネクタ接続
- 電源基板 ↔ メイン基板: 端子台経由ケーブル接続
- OLED / LTR390 / Encoder: ケース直付け・AWG28ケーブルでメイン基板へ接続

### TPS63802 EN端子の活用

- TPS63802の4P ENピンをXIAO D6(GPIO43)で制御する
- HIGH：通常動作（3.3V供給）
- LOW：Deep Sleep時にTPS63802をシャットダウン → センサ類への3.3V供給を完全遮断
- XC9306にはなかった機能であり、Deep Sleep時の消費電力を大幅に削減できる

### battery voltage monitor 方針

- 測定対象は `TP4056 OUT+ / OUT-` とする
- **分圧回路はメイン基板側へ配置する**
- 初期値は `100kΩ + 100kΩ` の 1:1 分圧を第一候補とする
- ADC入力ピン：XIAO D3(GPIO4・ADC1_CH3)
- 必要に応じて ADC 中点-GND 間へ 0.1µF を追加できる構成

### 線材

| 区間 | 線材 |
|------|------|
| JST ↔ TP4056 B+/B- | AWG24（直付け） |
| その他全配線 | AWG28 |
| GNDバス・3.3Vライン | スズメッキ線 |

---

## 電源基板通電試験結果（PASS）

| 試験項目 | 結果 |
|---------|------|
| 無負荷出力電圧 | 3.304V ✅ |
| USB-C単独接続時出力電圧 | 3.304V ✅ |
| LiPo単独接続時出力電圧 | 3.304V ✅ |
| USB-C + LiPo同時接続時出力電圧 | 3.304V ✅ |
| GND-3.3V間ショート確認 | ショートなし ✅ |
| RAW-3.3V間ショート確認 | ショートなし ✅ |

---

## メイン基板 部品配置（確定）

基板サイズ：95mm × 72mm（有効37穴 × 27穴）

### バス配線

| 信号 | 配置 |
|------|------|
| 3.3Vバス | 外周横(X:1-34, Y:1) + 支線横(X:1-10, Y:20) + 外周縦(X:1, Y:1-25) |
| GNDバス | 外周横(X:3-36, Y:27) + 支線横(X:29-36, Y:12) + 外周縦(X:36, Y:3-27) |
| SDAバス | 縦線(X:13, Y:10-23) |
| SCLバス | 縦線(X:17, Y:10-23) |

### 部品配置

| 部品 | 座標 | 特記 |
|-----|------|------|
| XIAO | (16,02)～(22,08) | D0～D10ピン配置確定 |
| MCP23017 | (21,13)～(26,26) | VDD/VSS間にバイパスC(0.1uF) |
| DS3231 | (02,02)～(10,17) | 上辺(Y:02)機械支持専用・電気的未接続 |
| BME280 | (02,22)～(07,26) | CSB=High / SDO=Low |
| microSD | (27,02)～(35,09) | SPI接続 |
| JST1(OLED) | (10,24)～(13,26) | I2C(0x3C) |
| JST2(LTR390) | (15,24)～(18,26) | I2C(0x53) |
| JST3(Encoder) | (32,14)～(34,21) | GPIO+LED制御 |
| 端子台 | (31,25)～(34,27) | GND/3.3V/VBAT_RAW/EN |
| R_EN(100kΩ) | (12,08)～(14,08) | DeepSleep対応・Hi-Z化対策 |
| バイパスC | (21,21)～(21,22) | MCP23017 VDD-VSS間 |
| 分圧R | (13,03)～(13,06) | R1(13,03-04) / R2(13,05-06) |
| プルアップR | (27,23)～(28,23) | MCP23017 /RESET |
| LED抵抗 | (27,18)～(28,20) | R_R / R_G / R_B |

---

## GPIO割り当て（確定）

### XIAO ESP32S3 Plus

| 端子 | GPIO | 用途 | 特性 |
|------|------|------|------|
| D0 | GPIO1 | Encoder_A | RTC GPIO・EXT0復帰可 |
| D1 | GPIO2 | Encoder_B | RTC GPIO対応 |
| D2 | GPIO3 | Encoder_SW | RTC GPIO・EXT1復帰可 |
| D3 | GPIO4 | Battery_ADC | RTC GPIO・ADC1_CH3 |
| D4 | GPIO5 | I2C_SDA | I2C確定 |
| D5 | GPIO6 | I2C_SCL | I2C確定 |
| D6 | GPIO43 | TPS63802_EN | RTC非対応→100kΩプルアップ対策 |
| D7 | GPIO44 | SPI_CS | UART RX兼用・USB CDC主体で使用可 |
| D8 | GPIO7 | SPI_SCK | SPI確定 |
| D9 | GPIO8 | SPI_MISO | SPI確定 |
| D10 | GPIO9 | SPI_MOSI | SPI確定 |
| D16 | GPIO10 | 予約 | 未使用 |

### MCP23017（I2Cアドレス 0x20）

| ピン | 用途 |
|-----|------|
| GPA0 | LED R（100Ω経由） |
| GPA1 | LED G（100Ω経由） |
| GPA2 | LED B（100Ω経由） |
| その他 | 未使用（将来拡張余地） |

---

## I2Cデバイス一覧

| デバイス | I2Cアドレス | 接続方法 |
|---------|-----------|---------|
| BME280 | 0x76 | 基板直付け |
| LTR390 | 0x53 | JST2経由・ケース直付け |
| OLED SSD1306 | 0x3C | JST1経由・ケース直付け |
| DS3231 | 0x68 | 基板直付け |
| AT24C32 | 0x57 | DS3231モジュール同梱 |
| MCP23017 | 0x20 | 基板直付け |

---

## DeepSleep対応設計（確定）

### GPIO43(TPS63802_EN)の対策

GPIO43はRTC GPIO非対応のため、DeepSleep中にHi-Z化する可能性がある。

**回路側対策**: 100kΩプルアップ(R_EN)を追加

- R_EN配置: (12,08)～(14,08) 横方向・縦置き実装
- 入力側: (11,01)→(11,08)→3.3Vバス経由
- 出力側: (14,08)→XIAO D6(GPIO43) + 端子台EN合流

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

## 動作モード

本システムは、開発段階では主に以下のモードで扱う。

| モード | 内容 | 用途 |
|--------|------|------|
| UI試験モード | 画面表示・入力確認中心 | 画面遷移・操作確認 |
| periodic logger | 一定周期で継続保存 | USB給電またはLiPo給電でのログ確認 |
| Deep Sleep logger | 起床ごと保存して再Sleep | 低消費電力運用の基準版 |
| Battery monitor試験 | 分圧回路・ADC確認 | vbat取得確認 |

---

## 基本動作シーケンス

### Deep Sleep logger 基本シーケンス

1. Deep Sleep から起床
2. 起床要因を確認
3. RTC / BME280 / LTR390 / microSD を再初期化
4. RTC から現在時刻取得
5. 時刻妥当性確認、必要なら復旧
6. BME280 測定
7. LTR390 測定
8. `context_code` / `head_code` / `vbat` を含むログ行を生成
9. microSD へ追記保存
10. 必要に応じて OLED 表示
11. GPIO43→LOW（TPS63802シャットダウン・センサ電源遮断）
12. SPI.end() / CS=HIGH 処理
13. 次回起床条件を設定
14. Deep Sleep に移行

### periodic logger 基本シーケンス

1. 初期化
2. RTC 取得
3. センサ測定
4. CSV 生成
5. microSD 追記
6. 必要に応じ表示更新
7. 次周期まで待機

---

## ログデータ形式

ログデータ形式は `03_LOG_FORMAT.md` を正とする。  
運用版では、以下の列構成を推奨する。

| 項番 | 項目名 | 内容 |
|------|--------|------|
| 1 | `date` | 日付 |
| 2 | `time` | 時刻 |
| 3 | `temp_c` | 温度 |
| 4 | `hum_pct` | 湿度 |
| 5 | `press_hpa` | 気圧 |
| 6 | `als` | 照度センサ値 |
| 7 | `uvs` | UVセンサ値 |
| 8 | `context_code` | 行動状態コード |
| 9 | `head_code` | 頭痛状態コード |
| 10 | `vbat` | 電池電圧 |

### 推奨CSVヘッダ

```text
date,time,temp_c,hum_pct,press_hpa,als,uvs,context_code,head_code,vbat
```

### CSV例

```text
2026-04-05,15:11:11,22.52,62.38,1006.26,565,0,1,1,3.98
```

---

## 画面構成

| 画面 | 内容 |
|------|------|
| VIEW | センサ値表示 |
| MENU | メニュー選択 |
| CLOCK | 時刻表示 |
| LOG | 記録状態表示 |
| SLEEP | スリープ移行確認または移行状態 |

### 画面構成に関する補足

- 画面切替は Rotary Encoder で行う
- VIEW / MENU / CLOCK / LOG / SLEEP の基本遷移は成立済み
- `context_code` / `head_code` の入力UIは後段で確定する
- 電池アイコンは `vbat` 実装後に統合する

---

## 省電力方針

| 項目 | 方針 |
|------|------|
| 通常運用 | Deep Sleep を基本とする |
| 基本周期 | 60秒を基準とする |
| 試験周期 | 5秒 / 30秒 / 60秒を許容 |
| OLED | 必要時のみ ON |
| Wi-Fi | 使用しない |
| Bluetooth | 使用しない |
| センサ電源 | Deep Sleep時にTPS63802 EN→LOWで遮断 |
| 電流測定 | 完成まで実施しない |

---

## システム成立条件

初号機で優先する成立条件は以下の通り。

| 優先度 | 項目 | 内容 |
|--------|------|------|
| 1 | RTC成立 | 正しい時刻で記録できること |
| 2 | センサ成立 | BME280 / LTR390 が読めること |
| 3 | 記録成立 | microSD に安定保存できること |
| 4 | Deep Sleep成立 | 周期起床で継続記録できること |
| 5 | UI成立 | 画面遷移と基本表示ができること |
| 6 | 状態コード反映 | `context_code` / `head_code` を扱えること |
| 7 | 電源成立 | LiPo → TP4056 → TPS63802 → XIAO が安定動作すること |
| 8 | EN制御成立 | Deep Sleep時にTPS63802をシャットダウンできること |
| 9 | LED制御成立 | MCP23017経由でRGB LEDを制御できること |
| 10 | 電池監視 | `vbat` 実装・ADC読み出し完成 |

---

## 本システムの設計方針

| 項番 | 方針 |
|------|------|
| S-1 | 携帯型 |
| S-2 | 長時間動作を目指す |
| S-3 | microSD記録 |
| S-4 | RTC基準時刻 |
| S-5 | Deep Sleep使用 |
| S-6 | USBで開発・ログ確認可能 |
| S-7 | 電池容量は運用で変更可能 |
| S-8 | 初号機は評価機 |
| S-9 | `context_code` / `head_code` は補助情報として扱う |
| S-10 | 電流評価は完成後に行う |
| S-11 | 電源回りは別基板化（電源基板 / メイン基板の2基板構成） |
| S-12 | TPS63802 EN端子でDeep Sleep時の電源を完全遮断する |
| S-13 | XIAO裏面JTAGランドは使用しない（MCP23017でLED制御を代替） |
| S-14 | 全配線は2.54mmピッチで統一する |
| S-15 | GPIO43にはRTC GPIO非対応のためプルアップで対策する |

---

## 現時点で確認済みのシステム成立範囲

- USB給電での統合動作
- LiPo → TP4056 → AE-TPS63802 → XIAO の最小電源経路
- LiPo駆動での Blink 動作
- BME280 / LTR390 / OLED / DS3231 の統合動作
- microSD 追記保存
- periodic logger
- Deep Sleep logger（60秒・30秒周期確認済み）
- VIEW / MENU / CLOCK / LOG / SLEEP の基本UI
- RGB LED付きスイッチ付きロータリーエンコーダ基本動作
- `context_code` / `head_code` 文書定義
- TP4056単体試験合格
- **電源基板フェーズ1～7完全成立（通電試験PASS）**
- **メイン基板 部品配置座標確定**
- **GPIO割り当て確定**
- **DeepSleep対応設計確定（R_EN・microSD CS処理）**

---

## 現時点で未確定の項目

- `vbat` 実装（分圧回路・ADC読み出し）
- MCP23017実装・LED R/G/B制御確認
- EN端子制御（GPIO43）実装・確認
- `context_code` の最終入力方法
- `head_code` の最終入力方法
- 電源基板の最終固定方法
- 筐体内レイアウト
- 長期運用時のログローテーション
- 完成後の消費電流評価

---

## ステータス

- [ACTIVE] システム基本構成として有効
- [ACTIVE] USB給電での統合動作確認済み
- [ACTIVE] LiPo駆動での最小動作確認済み
- [ACTIVE] 電源ICをXC9306からAE-TPS63802へ変更済み
- [ACTIVE] 端子台を4ライン構成（3.3V / GND / VBAT_RAW / EN）に更新済み
- [ACTIVE] 分圧回路はメイン基板側配置に方針確定
- [ACTIVE] Deep Sleep時のTPS63802 ENシャットダウン方針確定
- [ACTIVE] **電源基板フェーズ1～7完全成立（通電試験PASS）**
- [ACTIVE] **メイン基板サイズ95mm×72mmに確定**
- [ACTIVE] **MCP23017採用確定（LED R/G/B制御）**
- [ACTIVE] **GPIO割り当て確定（EN=D6/GPIO43・ADC=D3/GPIO4）**
- [ACTIVE] **メイン基板全部品配置座標確定**
- [ACTIVE] **DeepSleep対応設計確定（R_EN配置・microSD CS処理）**
- [ACTIVE] **Encoder復帰EXT0/EXT1対応確認済み**
- [IN PROGRESS] メイン基板実装作業
- [CHECK] 電池電圧測定未実装
- [CHECK] Deep Sleep電流未測定
- [CHECK] DS3231主電源断後バックアップ保持の最終判定
