# 03_電源デザイン

> 本書は CURRENT_STATUS.md を優先参照する。

---

# 目的

本書は、携帯型環境センサーロガー初号機の電源設計仕様を定義する。

本書には以下を含めない。

- 通電試験ログ
- 実装進捗
- フェーズ管理
- TODO
- 作業記録

それらは 30_LOG へ分離する。

---

# 基本方針

- 初号機は安全側設計を優先
- 電源系とメイン基板を分離
- Deep Sleep 中心運用
- 完成後に消費電力最適化を実施

---

# 電源構成

```text
LiPo
 ↓
TP4056
 ↓
AE-TPS63802
 ↓
3.3V BUS
 ↓
MCU / SENSOR / OLED / microSD
```

---

# Battery Monitor

## 分圧回路

```text
VBAT_RAW
  ↓
R1(100k)
  ↓
ADC(GPIO4)
  ↓
R2(100k)
  ↓
GND
```

---

# GPIO設計

| GPIO | 用途 |
|---|---|
| GPIO43 | TPS63802 EN |
| GPIO4 | Battery ADC |

---

# Deep Sleep設計

| 状態 | EN | 説明 |
|---|---|---|
| 通常動作 | HIGH | 電源ON |
| Deep Sleep | LOW | センサ遮断 |

---

# センサ電源条件

| センサ | 電圧 | 通信 |
|---|---|---|
| BME680 | 3.3V | I2C |
| LTR390 | 3.3V | I2C |
| DS3231 | 3.3V | I2C |
| MCP23017 | 3.3V | I2C |
| OLED SSD1306 | 3.3V | I2C |
| microSD | 3.3V | SPI |

---

# 文書責務

本書は電源仕様のみを扱う。

| 内容 | 分離先 |
|---|---|
| 通電試験 | 30_LOG/02_POWER_TEST_LOG.md |
| 実装進捗 | 30_LOG/03_IMPLEMENTATION_LOG.md |
| 廃止電源構成 | 90_HISTORY/ |
| 採用理由 | 51_DECISIONS/ |
