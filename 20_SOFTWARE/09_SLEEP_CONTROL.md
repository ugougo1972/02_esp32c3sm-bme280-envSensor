# 09_SLEEP_CONTROL

## 目的

本ファイルは携帯型環境センサーロガーにおけるSleep制御およびDeepSleep運用方針を定義する。

---

## 基本方針

- USB開発中はSleep必須としない
- 携帯運用時はDeepSleepを利用する
- 起床後は毎回再初期化する
- 状態保持を前提としない

---

## Sleepモード

### Active

通常動作

### Idle

表示待機

### DeepSleep

最低消費電力動作

---

## 起床要因

| 要因 | 用途 |
|--------|--------|
| Timer | 定期測定 |
| Reset | 手動再起動 |
| PowerOn | 電源投入 |

---

## DeepSleepシーケンス

```text
Wake
↓
I2C初期化
↓
RTC取得
↓
BME680取得
↓
LTR390取得
↓
Battery ADC取得
↓
CSV保存
↓
OLED消灯
↓
microSD終了
↓
DeepSleep
```

---

## DeepSleep前処理

### OLED

```cpp
display.clearDisplay();
display.display();
```

### microSD

```cpp
digitalWrite(PIN_SD_CS, HIGH);
SPI.end();
```

---

## DeepSleep設定例

```cpp
esp_sleep_enable_timer_wakeup(
  60ULL * 1000ULL * 1000ULL
);
```

---

## 起床後処理

1. Wire.begin()
2. RTC初期化
3. MCP23017初期化
4. BME680初期化
5. LTR390初期化
6. SPI.begin()
7. SD.begin()

---

## EN制御について

TPS63802 EN は

```text
D6(GPIO43)
```

へ接続される。

ただし現時点では

```text
D6 LOW → TPS63802停止
```

時の自己電源断挙動が未確認である。

そのため初号機では

- DeepSleep
- EN制御

を分離して評価する。

---

## 推奨周期

| 用途 | 周期 |
|--------|--------|
| Bring-up | 30秒 |
| 標準 | 60秒 |
| 長期運用 | 300秒以上 |

---

## 将来拡張

- RTC Alarm起床
- Encoder押下起床
- 低電圧自動Sleep
- 電池残量連動制御

---

## ステータス

- ACTIVE
- 初号機採用
