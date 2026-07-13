# MedFlo BT 系統規格書

> 最後更新: 2026-07-13 (JOHN01 正式版)
> 對應 FW: `tai_evb_20260713_222139_atf.fw`

---

## 1. 系統運作流程

```
┌──────────┐    STOP_ADV     ┌──────────┐   RTC wakeup    ┌──────────┐
│  BLE 廣播 │ ──────────────→ │  深睡模式  │ ──────────────→ │  BLE 廣播 │
│  (29秒)   │                │  (1秒)    │   START_ADV    │  (29秒)   │
│  LED 閃爍  │                │  LED 全滅  │                │  LED 閃爍  │
└──────────┘                └──────────┘                └──────────┘
      │                           │                           │
      └───────────────────────────┴───────────────────────────┘
                             循環週期: 30秒
```

### 詳細時序

| 階段 | 時間 | 動作 |
|------|------|------|
| **廣播階段** | 0 ~ 29 秒 | BLE 廣播中，每 100ms 檢查 GPIO18 並更新 LED |
| **停止廣播** | 第 29 秒 | `periodic_timer` 觸發，發送 `STOP_ADV` |
| **進入睡眠** | 第 29 秒 | 停止 LED timer、熄滅所有 LED、設定 RTC 喚醒 |
| **深睡** | 29 ~ 30 秒 | RTC 計時 1 秒 (`sleeptime = 1000000`)，CPU 休眠 |
| **RTC 喚醒** | 第 30 秒 | `_rtc_callback` 發送 `START_ADV`，重新開始廣播 |

---

## 2. 藍牙廣播格式

### 廣播參數

| 參數 | 設定值 | 說明 |
|------|--------|------|
| 廣播間隔 | `0xA0` (160 × 0.625ms = **100ms**) | 每秒約 10 次廣播 |
| 廣播類型 | Connectable (可連線) | `BT_LE_ADV_OPT_CONNECTABLE` |
| 廣播時間窗 | **29 秒** (`dwelltime = 29000`) | 每次喚醒廣播 29 秒後停止 |
| 裝置名稱 | `MEDFLO-XXXXXXXXXXXX` | MAC Address 後 6 bytes 轉大寫 hex |

### 廣播封包結構 (Adv Data)

| 順序 | AD Type | 長度 | 內容 |
|------|---------|------|------|
| 1 | **Flags** (`0x01`) | 3 bytes | `02 01 06` (LE General Discoverable, BR/EDR not supported) |
| 2 | **Shortened Name** (`0x08`) | 21 bytes | `13 08 4D 45 44 46 4C 4F 2D XX XX XX XX XX XX` (MEDFLO- + MAC) |
| 3 | **Manufacturer Data** (`0xFF`) | 5 bytes | `04 FF FF FF XX` |

### Manufacturer Data 欄位細節

| Byte | 欄位 | 值 | 說明 |
|------|------|-----|------|
| 0 | AD Length | `0x04` | 後面 4 bytes |
| 1 | AD Type | `0xFF` | Manufacturer Specific Data |
| 2-3 | **Company ID** | `0xFF 0xFF` | Bluetooth SIG 測試 ID (LSB first)，正式版需更換 |
| 4 | **GPIO18 狀態** | `0x00` 或 `0x01` | 即時感測器/按鍵狀態 |

### 結構體定義 (`bt_le_op.c`)

```c
struct medflo_manufacturer_data {
    uint16_t company_id;    // 0xFFFF (Bluetooth SIG test ID)
    uint8_t  gpio_status;   // GPIO18 即時狀態
};
```

### 注意事項
- `0xFFFF` 為 Bluetooth SIG 保留的測試用 Company ID，**正式醫療產品必須申請專屬 CID**
- 若無 Company ID，iOS 的 CoreBluetooth 無法正確解析 Manufacturer Data
- Android 的 `ScanRecord.getManufacturerSpecificData()` 同樣依賴 Company ID 做 key

---

## 3. 休眠與喚醒參數

| 參數 | 變數名稱 | 設定值 | 實際時間 | 說明 |
|------|----------|--------|----------|------|
| 廣播持續時間 | `dwelltime` | `29000` ms | **29 秒** | `k_timer` 週期，到期發送 STOP_ADV |
| 休眠時間 | `sleeptime` | `1000000` | **1 秒** | `rtc_setup_wakeup()` 參數，單位為 RTC tick (1MHz) |
| RTC 週期 (備用) | `rtc_period` | `5000000` | **5 秒** | 宣告於 `rtc_op.c`，目前未被使用 |
| LED 刷新週期 | `led_periodic_timer` | `100` ms | **0.1 秒** | 每 100ms 讀取 GPIO18 並更新 LED |
| 生命週期上限 | `DeviceMaximumRunningCount` | `63664` 次 | **約 168 小時 (7天)** | 每次 `STOP_ADV` counter+1，超過上限停止運作 |

### 喚醒流程

```
RTC 喚醒 → _rtc_callback()
  → app_to_msg(MSG_BLE_STATE, START_ADV)
    → main() while loop 接收訊息
      → system_ble_event_handle(START_ADV)
        → checkDeviceLifeCycle() == 0 ?
          → init_gpio_led()       # 重設 GPIO LED
          → start_adv()           # 開始 BLE 廣播
          → k_timer_start(periodic_timer, 29s)  # 重設定時器
          → k_timer_start(led_periodic_timer, 100ms)
```

---

## 4. GPIO 腳位清單

| Pin | 巨集名稱 | 方向 | 初始狀態 | 硬體設定 | 功能 |
|-----|----------|------|----------|----------|------|
| **18** | `INPUT_PIN` | 輸入 | High (上拉) | `GPIO_PULL_UP` + `GPIO_INT_DEBOUNCE` + `GPIO_INT_EDGE_BOTH` | 外部感測器/按鍵。狀態變化 → 觸發中斷 → 更新廣播 |
| **20** | `LED_GREEN_OUTPUT_PIN` | 輸出 | Low | MFP=`0x3000` (GPIO mode) | 綠燈：GPIO18=High 時亮 |
| **21** | `LED_RED_OUTPUT_PIN` | 輸出 | Low | MFP=`0x3000` (GPIO mode) | 紅燈：GPIO18=Low 時亮 |
| **7** | `LED2_RED_OUTPUT_PIN` | 輸出 | Low | MFP=`0x3000` (GPIO mode) | 第二紅燈：目前 `display_led()` 未使用，僅 `turn_led_all_off()` 時拉低 |
| **9** | (無巨集) | 輸出 | Low | 同 GPIO mode | 備用/除錯腳位，僅初始化拉低 |

### GPIO 暫存器位址 (Base: `0x40068000`)

| 腳位 | CTL Register | 位址 |
|------|-------------|------|
| GPIO7 | `GPIO7_CTL` | `0x40068020` |
| GPIO9 | `GPIO9_CTL` | `0x40068028` |
| GPIO18 | `GPIO18_CTL` | `0x4006804C` |
| GPIO20 | `GPIO20_CTL` | `0x40068054` |
| GPIO21 | `GPIO21_CTL` | `0x40068058` |

### LED 顯示邏輯 (`display_led()`)

| GPIO18 狀態 | 綠燈 (Pin 20) | 紅燈 (Pin 21) | 紅燈2 (Pin 7) | 備用 (Pin 9) |
|-------------|:-----------:|:-----------:|:------------:|:----------:|
| **High (1)** | ON | OFF | OFF | OFF |
| **Low (0)** | OFF | ON | OFF | OFF |
| **睡眠中** | OFF | OFF | OFF | OFF |

---

## 5. 生命週期控制

### Counter 機制

| 項目 | 值 |
|------|-----|
| 遞增時機 | 每次 `STOP_ADV` 觸發時 `update_counter()` |
| 上限值 | `DeviceMaximumRunningCount = 63664` |
| 換算時間 | 63664 × 30秒 ≈ **168 小時 (7 天)** |
| 到上限後 | `checkDeviceLifeCycle()` 回傳 -1，不再廣播，直接休眠 |
| 儲存方式 | 目前 NVRAM 寫入已註解 (`nvram_config_set` disabled)，每次開機從 0 開始 |

### 注意
- Counter 目前**未寫入 NVRAM**，斷電重開會歸零
- `Load_Counter_First()` 直接設 `counter = 0`
- 若需記錄使用時數，需恢復 `nvram_config_set()` 呼叫

---

## 6. 系統架構

### 訊息傳遞

```
中斷/事件 → app_to_msg(type, event) → send_msg() → msg_queue (FIFO)
                                                        ↓
main() while loop ← receive_msg() ← k_fifo_get()
       ↓
   switch(msg.type)
   ├── MSG_KEY_INPUT    → system_input_event_handle()
   └── MSG_BLE_STATE    → system_ble_event_handle()
```

### 訊息類型

| msg.type | msg.value | 說明 |
|----------|-----------|------|
| `MSG_BLE_STATE` | `START_ADV` | 開始廣播 |
| `MSG_BLE_STATE` | `STOP_ADV` | 停止廣播 + 進入睡眠 |
| `MSG_BLE_STATE` | `CONNECTED` | BLE 連線建立 |
| `MSG_KEY_INPUT` | `0x10` | K1: 手動開始廣播 |
| `MSG_KEY_INPUT` | `0x11` | K2: 手動停止廣播 |
| `MSG_KEY_INPUT` | `0x12` | K11: 開始 notify |
| `MSG_KEY_INPUT` | `0x13` | K3: 停止 notify |
| `MSG_KEY_INPUT` | `0x24` | K6: 手動斷線 |

### 原始碼對照

| 功能 | 檔案 |
|------|------|
| 主程式、系統初始化、訊息迴圈 | `src/main/main.c` |
| GPIO 控制、LED 顯示、中斷處理 | `src/actions/gpio_control.c` |
| RTC 喚醒、睡眠控制 | `src/actions/rtc_op.c` |
| BLE 廣播、連線管理、Manufacturer Data | `src/actions/bt_le_op.c` |
| 生命週期計數、NVRAM 操作 | `src/actions/nvram.c` |
| GPIO 暫存器位址定義 | `src/main/common.h` |
| 訊息管理系統 | `src/main/msg_manager.c` |

---

## 7. 變更記錄

| 日期 | 版本 | 變更 |
|------|------|------|
| 2026-05-11 | V02_03_29_1 | 原始版本 (耀宗提供) |
| 2026-07-13 | JOHN01 | ① Manufacturer Data 加入 2 bytes Company ID (0xFFFF) + status flags ② 廣播間隔 50ms→100ms ③ 加入 `bt_id_create` MAC 持久化 ④ `BT_DATA_NAME_COMPLETE` ⑤ STOP_ADV 改為連續廣播 (不進 sleep) ⑥ 加入電池電壓監控 (ADC) ⑦ 低電量時關閉 LED 省電 ⑧ Keil post-build 修正 (AfterMake 關閉，fromelf 手動提取 .bin) |
