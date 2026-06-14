# 06_POWER_CONTROL

## 目的

本ファイルは、携帯型環境センサーロガー初号機における電源制御、給電モード、低消費電力動作、Battery ADC、TPS63802 EN制御の方針を整理する。

---

## 1. 現行前提

本ファイルは、2026-06-14時点の現行統合基板構成を基準とする。

- TP4056とTPS63802はメイン基板へ統合する。
- TPS63802はEN端子付き構成を使用する。
- EN制御は XIAO D6(GPIO43) を使用する。
- ENノードには100kΩでGNDへプルダウンを入れる。
- Battery ADCは D3(GPIO4) に100kΩ + 100kΩの分圧中点を入力する。
- microSDは新SPI割当を使用する。
  - D10 = CS
  - D9 = MOSI
  - D8 = SCK
  - D7 = MISO
- I2C外付けプルアップ抵抗は今回実装しない。
- 測定機材として日置DT4256とUSB-C電流計を使用できる。

---

## 2. 電源構成

### 2.1 電源経路

```text
LiPo
  ↓
JST0
  ↓
TP4056
  ↓
TPS63802
  ↓
3.3Vバス
  ↓
XIAO / MCP23017 / DS3231 / BME680 / microSD / OLED / LTR390
```

### 2.2 部品役割

| 部品 | 役割 |
|---|---|
| LiPo | バッテリー |
| JST0 | LiPo接続 |
| TP4056 | LiPo充電・保護 |
| TPS63802 | 3.3V昇降圧 |
| XIAO ESP32S3 Plus | 制御MCU |
| Battery ADC | 電池系電圧監視 |

---

## 3. 電源モード定義

### 3.1 USB開発モード

| 項目 | 内容 |
|---|---|
| 主用途 | 書込み・シリアル確認・Bring-up |
| 給電 | XIAO USBまたは基板側USB |
| Serial Monitor | 使用 |
| DeepSleep | 必須ではない |
| OLED | 表示可 |
| 優先事項 | 観測性・切り分け |

### 3.2 LiPo運用モード

| 項目 | 内容 |
|---|---|
| 主用途 | 携帯運用 |
| 給電 | LiPo → TP4056 → TPS63802 |
| Serial Monitor | 原則使用しない |
| DeepSleep | 使用候補 |
| OLED | 必要時のみ |
| 優先事項 | 安定記録・消費電流低減 |

### 3.3 充電中モード

| 項目 | 内容 |
|---|---|
| 主用途 | LiPo充電中の動作検証 |
| 給電 | TP4056入力あり |
| 注意 | 充電電流と負荷電流が重なる |
| 評価 | USB-C電流計で確認 |

### 3.4 Sleepモード

| 項目 | 内容 |
|---|---|
| 主用途 | 間欠記録・低消費 |
| 処理 | SD終了、OLED消灯、必要部品停止 |
| DeepSleep | `esp_deep_sleep_start()` |
| 注意 | EN制御で周辺3.3V遮断する場合は復帰設計を要確認 |

---

## 4. TPS63802 EN制御

### 4.1 接続

| 信号 | 接続 |
|---|---|
| TPS63802 EN | XIAO D6(GPIO43) |
| ENプルダウン | EN → 100kΩ → GND |

### 4.2 方針

- D6をHIGHにするとTPS63802有効。
- D6をLOWにするとTPS63802無効。
- ENプルダウンにより、XIAO未駆動時やHi-Z時に意図せずONになりにくくする。
- 実機確認前は、EN制御による自己電源断の挙動を慎重に扱う。

### 4.3 注意

TPS63802がXIAO本体へ給電している構成で、D6をLOWにするとXIAO自身の電源も落ちる可能性がある。  
このため、EN制御は以下のどちらの運用かを実機確認で確定する。

| 運用 | 内容 |
|---|---|
| 周辺電源遮断 | XIAOは別経路で維持、周辺のみ遮断 |
| 全電源遮断 | XIAOも含めて停止、復帰条件は外部操作や再給電 |

現時点では、EN制御は「試験対象」とし、DeepSleepの必須要件にはしない。

---

## 5. Battery ADC

### 5.1 回路

```text
LiPo/TP4056 B+系 ----[100kΩ]----+---- XIAO D3(GPIO4)
                                |
                              [100kΩ]
                                |
GND ----------------------------+
```

### 5.2 仕様

| 項目 | 内容 |
|---|---|
| ADC端子 | D3(GPIO4) |
| 分圧 | 100kΩ + 100kΩ |
| 分圧比 | 1:1 |
| 換算 | ADC入力値 × 2 |
| 記録列 | `vbat` |
| 初回確認 | DT4256実測値と比較 |

### 5.3 ソフト処理

```cpp
float readVbat() {
  uint32_t mv = analogReadMilliVolts(PIN_BAT_ADC);
  return (mv / 1000.0f) * 2.0f;
}
```

### 5.4 注意

- TP4056 OUT+またはLiPo B+系をD3へ直結しない。
- 必ず分圧を通す。
- ADC値が不安定な場合は中点-GND間に0.1uFを追加検討する。
- `vbat = 0` は未実装・未取得を意味する。

---

## 6. microSDとSleep前処理

### 6.1 SPI割当

| 信号 | XIAO端子 |
|---|---|
| CS | D10 |
| MOSI | D9 |
| SCK | D8 |
| MISO | D7 |

### 6.2 Sleep前処理

DeepSleepまたは電源制御前には、microSDを安全に終了する。

```cpp
digitalWrite(PIN_SD_CS, HIGH);
SPI.end();
```

### 6.3 保存完了条件

Sleepへ移行する前に以下を満たす。

1. ファイル書込み完了
2. `flush()` または `close()` 完了
3. CS=HIGH
4. SPI.end() 実行済み

---

## 7. DeepSleep方針

### 7.1 基本方針

- 最終的にはDeepSleepによる間欠記録を目指す。
- Bring-up中はDeepSleepを必須にしない。
- USB開発中は周期実行・シリアル出力を優先する。
- DeepSleep版では起床ごとに各デバイスを再初期化する。

### 7.2 1サイクル

```text
Wake
↓
GPIO初期化
↓
I2C初期化
↓
RTC確認
↓
センサ読取り
↓
Battery ADC読取り
↓
microSD保存
↓
OLED必要時表示
↓
SD終了
↓
DeepSleep
```

### 7.3 起床要因

| 起床要因 | 用途 |
|---|---|
| Timer | 通常記録周期 |
| GPIO | 将来の押下復帰候補 |
| Reset/PowerOn | 初回起動 |

### 7.4 RTC GPIOに関する注意

D6(GPIO43)はRTC GPIOではありません。  
DeepSleep中の保持や起床用GPIOとしては扱いません。

---

## 8. OLED制御

| 状態 | OLED |
|---|---|
| USB開発 | 表示可 |
| UI操作中 | 表示 |
| ログ記録直後 | 必要時のみ短時間表示 |
| DeepSleep前 | 消灯 |
| DeepSleep中 | 消灯 |

OLEDは常時点灯を前提にしない。

---

## 9. RGB LED制御

RGB LEDはMCP23017経由で制御する。

| LED | MCP23017 |
|---|---|
| LED_R | GPA2 |
| LED_G | GPA1 |
| LED_B | GPA0 |

方針：

- 常時点灯しない。
- 状態表示用に短時間点灯する。
- 消費電流測定前は明るさ・点灯時間を固定する。
- LED極性は実物確認後に最終確定する。

---

## 10. センサ電源方針

現行基板では、各I2Cデバイスは3.3Vバスに接続する。

| デバイス | 電源 |
|---|---|
| MCP23017 | 3.3V |
| DS3231 + AT24C32 | 3.3V |
| BME680 | 3.3V |
| OLED | 3.3V |
| LTR390 | 3.3V |
| microSD | 3.3V |

個別電源制御は初号機では実装しない。

---

## 11. 消費電流測定

### 11.1 使用機材

- 日置 DT4256
- USB-C電流計
- ピンフックプローブ

### 11.2 測定対象

| 項目 | 測定方法 |
|---|---|
| USB給電電流 | USB-C電流計 |
| 3.3Vバス電圧 | DT4256 DCV |
| LiPo電圧 | DT4256 DCV |
| Battery ADC中点 | DT4256 DCV |
| Sleep時電流 | DT4256電流レンジまたは後日治具 |

### 11.3 記録する値

- 起動時電流
- アイドル時電流
- OLED表示時電流
- microSD書込み時電流
- RGB LED点灯時電流
- DeepSleep時電流

---

## 12. Bring-up順序

1. LiPo未接続で導通確認
2. USB給電で3.3V確認
3. XIAO起動
4. MCP23017確認
5. DS3231確認
6. BME680確認
7. microSD確認
8. Battery ADC確認
9. TPS63802 EN確認
10. OLED確認
11. LTR390確認
12. RotaryEncoder確認
13. LiPo駆動確認
14. 消費電流測定

---

## 13. リスクと対策

| リスク | 内容 | 対策 |
|---|---|---|
| EN制御で自己電源断 | D6 LOWでXIAOも停止する可能性 | 実機で段階試験 |
| microSD書込み中断 | ファイル破損 | close後にSleep |
| ADC値不安定 | 高抵抗分圧・ノイズ | 平均化、必要時0.1uF |
| I2C不安定 | 複数モジュール接続 | 外付けプルアップは後付け検討 |
| LiPo充電中負荷 | TP4056の熱・電流 | USB-C電流計で確認 |

---

## 14. ステータス

- [COMPLETE] 統合基板前提へ更新
- [COMPLETE] ENプルダウン構成へ更新
- [COMPLETE] 新SPI割当へ更新
- [COMPLETE] Battery ADC方針更新
- [ACTIVE] 電源制御文書として有効
- [PENDING] EN制御実機確認
- [PENDING] LiPo駆動DeepSleep確認
- [PENDING] 実消費電流測定
