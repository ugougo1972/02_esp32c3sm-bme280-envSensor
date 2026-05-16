# CHANGELOG

本ファイルは、リポジトリ全体の時系列変更記録を管理するためのファイルです。  
詳細な設計判断理由は `50_DECISIONS/`、廃止仕様は `90_HISTORY/` を参照してください。

---

## 2026-05-16

### Changed

- READMEを入口文書として再整理。
- `CURRENT_STATUS.md` を最新状態の集約点として追加する方針を明確化。
- BME280破損に伴い、現行環境センサをBME680へ変更する方針を反映。
- `50_DECISIONS/` を設計判断保存領域として使用する方針を明確化。
- `90_HISTORY/` を旧仕様・廃止案の隔離領域として使用する方針を明確化。

### Added

- `CHANGELOG.md`
- `50_DECISIONS/2026-05-16_adopt_bme680.md`
- `90_HISTORY/2026-05-16_bme280_retired.md`

### Pending

- BME680実機統合試験。
- BME680ガス指標の記録仕様確定。
- `02_PARTS_LIST.md` と `03_POWER_DESIGN.md` のさらなる責務分離。
