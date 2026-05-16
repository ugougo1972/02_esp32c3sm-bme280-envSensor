# 修正後ツリー構造

```text
02_esp32c3sm-bme280-envSensor/
├─ README.md
├─ CURRENT_STATUS.md
├─ CHANGELOG.md
├─ TREE.md
├─ 10_HARDWARE/
│  ├─ 02_PARTS_LIST.md
│  ├─ 03_POWER_DESIGN.md
│  └─ 07_HARDWARE_STATUS.md
├─ 30_LOG/
│  └─ 01_TEST_LOG.md
├─ 50_DECISIONS/
│  ├─ README.md
│  ├─ 2026-05-16_adopt_bme680.md
│  └─ 2026-05-16_document_responsibility_rules.md
└─ 90_HISTORY/
   ├─ README.md
   ├─ 2026-05-16_bme280_retired.md
   └─ 2026-05-16_pre_restructure_notes.md
```

## 反映方針

- `CURRENT_STATUS.md` を現行状態の最優先参照先とする。
- `README.md` は入口文書に限定する。
- `10_HARDWARE/02_PARTS_LIST.md` は純粋な部品表に限定する。
- `10_HARDWARE/03_POWER_DESIGN.md` は電源仕様に限定する。
- 実装進捗は `10_HARDWARE/07_HARDWARE_STATUS.md` に分離する。
- 試験結果は `30_LOG/01_TEST_LOG.md` に分離する。
- 判断理由は `50_DECISIONS/` に保存する。
- 旧仕様・廃止構成は `90_HISTORY/` に隔離する。
