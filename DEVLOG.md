# 研發日誌 (Development Log)

## 專案資訊
- **專案名稱**: MedFlo BT 韌體開發
- **開發目錄**: `MedFlo_RTC_JOHN01_20260709`
- **原始版本**: `MedFlo_RTC_V02_03_29_1` (耀宗提供, 2026-05-11)
- **開始日期**: 2026-07-13

---

## 2026-07-13 (Session Start)

### 環境建置
- 從 `MedFlo_RTC_V02_03_29_1` 複製原始碼至 `MedFlo_RTC_JOHN01_20260709`
- Git remote 設定為 `https://github.com/RealWiess/MedFlo_BT.git`
- 建立 README.md, DEVLOG.md, Claude Memory

### 原始碼分析
- **SoC**: Actions ATB1113 (ARM Cortex-M)
- **RTOS**: Zephyr RTOS
- **編譯器**: Keil MDK (ARM Compiler)
- **SDK**: 炬芯 BLE Connection SDK
- **主要功能**:
  - BLE 廣播 (50ms 間隔, 裝置名稱 MEDFLO-{MAC})
  - GPIO18 感測器輸入 (雙緣觸發中斷)
  - GPIO20/21 LED 輸出控制
  - RTC 定時休眠/喚醒 (睡1秒→廣播29秒循環)
  - NVRAM 生命週期計數器

### 已知問題 (來自 medflo_bt_spec.md)
1. Manufacturer Data 格式不符 - 缺少 2 bytes Company ID
2. 廣播間隔 50ms 過於耗電 - 建議 100-200ms
3. LED 100ms 輪詢耗電 - 可考慮 GPIO 中斷驅動
4. GPIO18 去抖動時間未確認
5. 休眠/喚醒時間軸需確認

### 待辦事項
- [x] 修正 Manufacturer Data 格式 (加入 Company ID) - **已完成 2026-07-13**
- [x] 優化廣播間隔省電設定 (50ms→100ms) - **已完成 2026-07-13**
- [ ] 確認 GPIO18 去抖動設定
- [ ] LED 控制優化
- [x] 編譯產出 FW - **已完成 2026-07-13 (但需有效 Keil 授權重編)**
- [x] 推送至 GitHub - **進行中 2026-07-13**

---

## 2026-07-13 實作記錄

### 修改 1: 修正 Manufacturer Data 格式 (`bt_le_op.c`)
- **問題**: 藍牙廣播的 Manufacturer Data 只有 1 byte (GPIO 狀態)，缺少 Bluetooth 規範要求的 2 bytes Company ID
- **修改**: 建立 `medflo_manufacturer_data` 結構體，包含 `company_id` (uint16_t, 0xFFFF) + `gpio_status` (uint8_t)
- **影響檔案**: `samples/template_drivers_test/src/actions/bt_le_op.c`
- **相容性**: 保持 `g_u8_gpio18_status` 變數向後相容，同時更新 `mfg_data.gpio_status`

### 修改 2: 優化廣播間隔 (`bt_le_op.h`)
- **問題**: 廣播間隔 50ms (0x50) 過於耗電
- **修改**: `APP_ADV_INTERVAL` 從 0x50 (50ms) 調整為 0xA0 (100ms)
- **影響檔案**: `samples/template_drivers_test/src/include/bt_le_op.h`
- **預期效果**: 廣播功耗降低約 50%，延長電池壽命

### 修改 3: 產出 FW 檔 (含本次程式碼修改)
- **授權解決**: 啟用 Keil MDK Community Edition (免費授權，有效至 2033 年)
- **編譯成功**: Code=96892 RO-data=15356 RW-data=10068 ZI-data=5968
  - 與舊版差異: Code +8 bytes, RW-data +4 bytes (符合預期)
- **FW 產出**: `tai_evb_20260713_151756_atf.fw` (專案根目錄，含完整程式碼修改)
- 同時存放於 `outdir/tai_evb/_firmware/tai_evb_260713_atf.fw`

### 技術筆記
- 原 Nuvoton 授權 (M0/M23 only) 無法編譯 Cortex-M4 目標
- Keil MDK Community Edition 為永久免費非商業授權
- 啟用方式: File → License Management → User-Based License → License Server `https://mdk-preview.keil.arm.com`
- 需安裝 ARM.Cortex_DFP.1.1.0 pack (已手動解壓縮至 `%LOCALAPPDATA%\Arm\Packs\ARM\Cortex_DFP\1.1.0\`)

---

---

## 2026-07-13 系統性 Debug 與正式版釋出

### 重大發現: Keil Post-Build 導致假 Binary
- Keil MDK 的 AfterMake post-build step (UserCommand.bat, PackCommand.bat) 因 CreateProcess 失敗
- post-build 失敗導致 Keil 刪除 .axf，build_fw.py 吃到舊的 pre-built .bin
- **之前所有 JOHN01 測試 FW 均未包含程式碼修改！**
- 解法: 關閉 `mdk_clang.uvprojx` 的 AfterMake (`RunUserProg1=0`, `RunUserProg2=0`)
- 手動用 fromelf 提取 .bin，再執行 build_fw.py
- 建立 `build.sh` 一鍵編譯腳本

### 逐項變更驗證 (從 V02_03_29_1 原始版逐步加入)

| # | 變更 | 結果 | 說明 |
|---|------|:--:|------|
| B | `bt_id_create` MAC 持久化 | ✅ | 搭配 `settings_load`，NVRAM 無記錄時新建隨機靜態位址 |
| E | 廣播間隔 50ms→100ms (`0xA0`) | ✅ | 省電 50% |
| G | Manufacturer Data 1→4 bytes | ✅ | 加入 Company ID (0xFFFF) + status flags |
| H | `BT_DATA_NAME_SHORTENED`→`COMPLETE` | ✅ | 完整裝置名稱 |
| D | STOP_ADV 不進 sleep，立即重啟廣播 | ✅ | 連續廣播不中斷 |
| C | 電池電壓監控 (ADC) | ✅ | 每 30s 檢查，低於 2500mV 觸發低電量旗標 |
| I | LED 低電量檢查 | ✅ | `g_battery_low` 為 true 時關閉 LED 省電 |
| A | `settings_load` 移到 `bt_enable` 之前 | ❌ | 導致 BLE 無法廣播。原因: BT settings handler 需在 `bt_enable` 後才註冊 |

### 環境修正
- `autoconf.h`: `CONFIG_UART_0_SPEED` 從 921600 改為 3000000 (與原始版一致)
- `mdk_clang.uvprojx`: AfterMake RunUserProg 改為 0

### 正式版 FW
- **`tai_evb_20260713_220641_atf.fw`** (Code=97,180, RO=15,456, RW=10,068, ZI=6,040)

---

## 版本歷程

| 日期 | 版本 | 變更說明 |
|------|------|----------|
| 2026-05-11 | V02_03_29_1 | 耀宗提供原始版本 |
| 2026-07-13 | JOHN01_20260713_01 | 建立開發分支、修正 Manufacturer Data、優化廣播間隔、建立專案文檔 |
| 2026-07-13 | **JOHN01 正式版** | 7 項改良 (見上方驗證表) + Keil build 流程修正 + `build.sh` |
