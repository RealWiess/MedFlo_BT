# MedFlo BT 韌體開發專案 (JOHN01)

## 專案概述

本專案基於 Actions(炬芯) ATB1113 BLE SoC，使用 Zephyr RTOS 開發的醫療點滴藍牙監測裝置韌體。

- **原始版本**: `MedFlo_RTC_V02_03_29_1` (耀宗提供)
- **開發版本**: `MedFlo_RTC_JOHN01_20260709`
- **GitHub**: https://github.com/RealWiess/MedFlo_BT
- **SDK**: `BLE Connection SDK/` (炬芯 BLE Connection SDK)
- **開發工具**: Keil MDK (ARM Compiler)
- **硬體平台**: ATB1113 (AS3113 BLE Module)

## 目錄結構

```
MedFlo_RTC_JOHN01_20260709/
├── arch/           # ARM Cortex-M 架構相關
├── boards/         # 開發板定義
├── drivers/        # 驅動程式
├── ext/            # 外部函式庫
├── include/        # 系統標頭檔
├── kernel/         # Zephyr 核心
├── lib/            # 函式庫
├── samples/
│   └── template_drivers_test/   # 主應用程式
│       ├── keil/                 # Keil MDK 專案檔
│       ├── src/
│       │   ├── main/             # 主程式 (main.c, msg_manager.c, ...)
│       │   ├── actions/          # 功能模組 (gpio_control, rtc_op, bt_le_op, nvram)
│       │   └── include/          # 應用層標頭檔
│       └── outdir/tai_evb/_firmware/  # FW 輸出目錄
├── scripts/        # 編譯腳本
├── soc/            # SoC 相關 (ATB1113)
├── subsys/         # 子系統 (藍牙、檔案系統等)
└── tools/          # 工具
```

## 系統功能

| 功能 | 說明 |
|------|------|
| **BLE 廣播** | 裝置名稱 `MEDFLO-{MAC}`, 攜帶 GPIO18 感測器狀態 |
| **GPIO 輸入** | GPIO18 作為感測器/按鍵輸入 (雙緣觸發中斷) |
| **LED 指示** | GPIO20 綠燈, GPIO21 紅燈, GPIO7 紅燈2 |
| **RTC 定時喚醒** | 休眠 1 秒後喚醒, 廣播 29 秒, 週期循環 |
| **生命週期控制** | NVRAM counter 達到 DeviceMaximumRunningCount 後停止運作 |

## GPIO 腳位對照

| Pin | 巨集 | 方向 | 功能 |
|-----|------|------|------|
| 18 | INPUT_PIN | 輸入 (上拉) | 外部感測器/按鍵 |
| 20 | LED_GREEN_OUTPUT_PIN | 輸出 | 綠色 LED |
| 21 | LED_RED_OUTPUT_PIN | 輸出 | 紅色 LED |
| 7 | LED2_RED_OUTPUT_PIN | 輸出 | 第二紅色 LED (備用) |
| 9 | (無巨集) | 輸出 | 備用/除錯腳位 |

## FW 檔案命名規則

FW 檔產出至 `samples/template_drivers_test/outdir/tai_evb/_firmware/` 及專案根目錄。

命名格式: `tai_evb_YYYYMMDD_HHMMSS_atf.fw`

## 系統規格

完整系統規格、運作流程、廣播格式、休眠參數、GPIO 腳位等，詳見 **[系統規格書 (medflo_bt_spec.md)](medflo_bt_spec.md)**。

> ⚠️ **規格變更時，請同步更新 `medflo_bt_spec.md`！**

## 開發記錄

詳見 [研發日誌](DEVLOG.md)

## 對話記錄

所有 Claude Code 對話與決策記錄於 `.claude/memory/` 目錄。
