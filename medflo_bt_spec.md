# MedFlo BT 系統規格與分析報告

以下是根據原始碼（`main.c`, `gpio_control.c`, `rtc_op.c`, `bt_le_op.c` 等）分析出來的系統規格、功能、優化建議，以及 GPIO 腳位清單。

## 1. 系統規格與功能列表

| 功能項目 | 規格說明 | 💡 建議修改與優化 |
|----------|----------|-------------------|
| **藍牙廣播 (BLE Advertising)** | 使用 `bt_le_op.c` 定義廣播包：<br>1. Flags (General Discoverable)<br>2. 裝置名稱 (MEDFLO- 加上 MAC Address)<br>3. Manufacturer Data (夾帶 GPIO18 的 1 byte 狀態)<br>- **廣播間隔**：設為 `0x50` (50 ms) | **① Manufacturer Data 格式不符**：目前僅送出 1 byte，缺少 2 bytes 的 Company ID。這會導致 iOS/Android 無法正確解析。建議加上 `0xFFFF` 或專屬的 CID。<br>**② 廣播間隔過於耗電**：50ms 的廣播非常耗電，若非特殊即時需求，建議改為 100ms~200ms (如 `0xA0` ~ `0x140`) 來延長電池壽命。 |
| **軟體定時器 (k_timer)** | 建立兩個定時器控制狀態：<br>- `periodic_timer`：週期 29000 ms (29 秒)，時間到會送出 `STOP_ADV` 停止廣播。<br>- `led_periodic_timer`：週期 100 ms，定時呼叫 `display_led()` 刷新 LED 狀態。 | **① 定時器邏輯**：29 秒的廣播視窗是合理的配對/連線等待時間，但需注意如果在此期間發生連線，應提早取消此定時器。<br>**② LED 刷新率**：每 100ms 喚醒 CPU 檢查 GPIO 並更新 LED，對省電較不利。可以考慮改由 GPIO 中斷直接驅動 LED，或是使用硬體 PWM 節省 CPU 資源。 |
| **RTC 喚醒與深睡控制** | 當收到 `STOP_ADV` 時，系統會關閉定時器、關閉所有 LED，並呼叫 `rtc_setup_wakeup(1000000)` (時間單位換算後為 1 秒) 讓系統進入睡眠。<br>另一組 RTC 回呼 (`rtc_period = 5000000`，即 5 秒) 會定時發送 `START_ADV` 重新開始廣播。 | **① 休眠與喚醒的時間軸可能衝突**：有 1 秒的 wakeup 與 5 秒的定時廣播，需要確認這兩者的週期是否如預期運作（例如每睡 5 秒醒來廣播 29 秒）。<br>**② 進入休眠前的確保**：進入 RTC Sleep 前，建議檢查藍牙底層是否已確實停止 (`bt_le_adv_stop`)，避免射頻模組殘留耗電。 |
| **按鍵與 LED 互動邏輯** | `gpio_control.c` 設定 GPIO18 為雙緣觸發中斷 (輸入)。<br>透過 `display_led()` 邏輯判斷：<br>- 若 GPIO18 = `1` (High) → 亮綠燈、滅紅燈<br>- 若 GPIO18 = `0` (Low) → 滅綠燈、亮紅燈 | **① 去抖動 (Debounce) 設定**：目前啟用了 `GPIO_INT_DEBOUNCE`，但未明確設定時間。建議確認底層驅動的預設去抖時間是否足夠消除按鍵彈跳現象。<br>**② 中斷工作佇列**：目前 GPIO 中斷會觸發 `k_work_submit(&adv_update_work)` 去更新廣播資料，這是非常標準且安全的 Zephyr 作法 (Bottom-half 處理)！ |

---

## 2. GPIO 腳位清單與功能對照表

| GPIO 腳位 | 巨集定義名稱 | 方向 | 預設 / 初始狀態 | 額外硬體設定 | 實際對應功能 / 備註 |
|-----------|--------------|------|-----------------|--------------|---------------------|
| **Pin 18** | `INPUT_PIN` | **輸入** | 高電位 (High) | `GPIO_PULL_UP` (內部上拉)<br>`GPIO_INT_DEBOUNCE` (硬體去抖)<br>`GPIO_INT_EDGE_BOTH` (雙緣觸發) | 作為**外部感測器或按鍵**的輸入腳位。狀態改變時會觸發中斷，同時更新藍牙廣播裡的 Manufacturer Data。 |
| **Pin 20** | `LED_GREEN_OUTPUT_PIN` | **輸出** | 低電位 (Low) | 無 (初始設為 `GPIO_OUTPUT_LOW`) | **綠色 LED 控制**。<br>當 Pin 18 為 High 時點亮。 |
| **Pin 21** | `LED_RED_OUTPUT_PIN` | **輸出** | 低電位 (Low) | 無 (初始設為 `GPIO_OUTPUT_LOW`) | **紅色 LED 控制**。<br>當 Pin 18 為 Low 時點亮。 |
| **Pin 7** | `LED2_RED_OUTPUT_PIN` | **輸出** | 低電位 (Low) | 無 (初始設為 `GPIO_OUTPUT_LOW`) | 第二顆紅色 LED。目前在 `display_led()` 中**並未被實際切換使用**，只有在 `turn_led_all_off()` 時會強制拉低。建議確認硬體設計是否真的有接這顆燈。 |
| **Pin 9** | (無巨集名稱) | **輸出** | 低電位 (Low) | 無 (初始設為 `GPIO_OUTPUT_LOW`) | 程式碼中也有對 GPIO9 進行初始化及拉低的動作，但沒有其他功能邏輯，推測可能是備用的除錯腳位或未使用的硬體腳位。 |
