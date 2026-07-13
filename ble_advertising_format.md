# MedFlo BT 廣播格式規格

> 版本: JOHN01 正式版
> FW: `tai_evb_20260713_222139_atf.fw`

---

## 1. 廣播參數

| 參數 | 設定值 | 說明 |
|------|--------|------|
| 廣播間隔 | `0xA0` (160 × 0.625ms = **100ms**) | 每秒約 10 次廣播 |
| 廣播類型 | **Connectable** (可連線) | `BT_LE_ADV_OPT_CONNECTABLE` |
| 廣播時間窗 | **29 秒** | 每次喚醒廣播 29 秒後停止並立即重新廣播 |
| 裝置名稱 | `MEDFLO-XXXXXXXXXXXX` | MAC Address 後 6 bytes 轉大寫 hex |

---

## 2. 廣播封包結構 (Advertising Data)

總長度: **30 bytes** (BLE 上限 31 bytes)

| Offset | 欄位 | 長度 | 值 | 說明 |
|--------|------|------|-----|------|
| 0 | AD Length | 1 | `0x02` | |
| 1 | AD Type | 1 | `0x01` | Flags |
| 2 | Flags | 1 | `0x06` | LE General Discoverable, BR/EDR Not Supported |
| 3 | AD Length | 1 | `0x14` | 20 bytes 後續 |
| 4 | AD Type | 1 | `0x09` | Complete Local Name |
| 5-23 | Name | 19 | `MEDFLO-XXXXXXXXXXXX` | ASCII, MAC 後 6 bytes 大寫 hex |
| 24 | AD Length | 1 | `0x05` | 5 bytes 後續 |
| 25 | AD Type | 1 | `0xFF` | Manufacturer Specific Data |
| 26-27 | Company ID | 2 | `0xFF 0xFF` | Bluetooth SIG 測試用 ID (LSB first) |
| 28 | GPIO18 狀態 | 1 | `0x00` / `0x01` | 即時感測器/按鍵狀態 |
| 29 | Status Flags | 1 | bitmap | BIT0=低電量, BIT1=感測器異常 |

---

## 3. Manufacturer Data 結構體

```c
struct medflo_manufacturer_data {
    uint16_t company_id;    // 0xFFFF (Bluetooth SIG test ID)
    uint8_t  gpio_status;   // GPIO18 即時狀態
    uint8_t  status_flags;  // MEDFLO_FLAG_xxx
};
```

### Status Flags 定義

| Bit | 巨集 | 說明 |
|-----|------|------|
| 0 | `MEDFLO_FLAG_BATTERY_LOW` | 電池低於 2.5V |
| 1 | `MEDFLO_FLAG_SENSOR_ALERT` | 感測器異常 (預留) |
| 2-7 | — | 預留 |

---

## 4. Byte 陣列範例

```
GPIO18=Low,  電池正常:
02 01 06 14 09 4D 45 44 46 4C 4F 2D 41 31 42 32 43 33 44 34 45 35 46 36 05 FF FF FF 00 00

GPIO18=High, 電池正常:
02 01 06 14 09 4D 45 44 46 4C 4F 2D 41 31 42 32 43 33 44 34 45 35 46 36 05 FF FF FF 01 00

GPIO18=Low,  低電量:
02 01 06 14 09 4D 45 44 46 4C 4F 2D 41 31 42 32 43 33 44 34 45 35 46 36 05 FF FF FF 00 01
```

---

## 5. 注意事項

- `0xFFFF` 為 Bluetooth SIG 保留的測試用 Company ID，**正式醫療產品必須申請專屬 CID**
- 若無 Company ID，iOS CoreBluetooth 無法正確解析 Manufacturer Data
- Android `ScanRecord.getManufacturerSpecificData()` 同樣依賴 Company ID 做 key
- 裝置名稱固定 19 bytes，不可超過 BLE 廣播 payload 上限 (31 bytes)
