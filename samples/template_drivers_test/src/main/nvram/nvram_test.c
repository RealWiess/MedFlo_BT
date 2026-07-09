/*
 * Copyright (c) 2022 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/nvram_config.h>
#include <soc_irq.h>
#include "common.h"


#define NV_KEY_BANK0 "CNT_0"
#define NV_KEY_BANK1 "CNT_1"

typedef struct {
    uint32_t value;    // 
    uint32_t seq_num;  // 
} BankData_t;

static BankData_t g_current_data = {0, 0};
static uint8_t g_next_bank = 0; // 

void Counter_Init_From_NVRAM(void) {
    BankData_t data0 = {0, 0}, data1 = {0, 0};
    int res0 = nvram_config_get(NV_KEY_BANK0, &data0, sizeof(BankData_t));
    int res1 = nvram_config_get(NV_KEY_BANK1, &data1, sizeof(BankData_t));

    
    if (data0.seq_num >= data1.seq_num && res0 > 0) {
        g_current_data = data0;
        g_next_bank = 1; 
    } else if (res1 > 0) {
        g_current_data = data1;
        g_next_bank = 0;
    } else {
        
        g_current_data.value = 0;
        g_current_data.seq_num = 0;
        g_next_bank = 0;
    }
}


void Counter_Save_To_Bank(uint32_t new_value) {
    g_current_data.value = new_value;
    g_current_data.seq_num++; // ????

    const char* target_key = (g_next_bank == 0) ? NV_KEY_BANK0 : NV_KEY_BANK1;
    
    // 
    int ret = nvram_config_set(target_key, &g_current_data, sizeof(BankData_t));
    
    if (ret == 0) {
        g_next_bank = !g_next_bank; // 
    }
}


static uint32_t g_ram_counter = 0;   
static uint32_t g_last_saved = 0;   
#define SAVE_THRESHOLD 100            




#define NV_COUNTER_KEY "RUN_CNT"

void save_counter_to_nvram(uint32_t count_val) {
    int ret;

 
    ret = nvram_config_set(NV_COUNTER_KEY, &count_val, sizeof(count_val));
    
    if (ret == 0) {
      //        printf("Counter %u saved successfully.\n", count_val);
    } else {
 //       printf("Save failed, error code: %d\n", ret);
    }
}


uint32_t load_counter_from_nvram(void) {
    uint32_t saved_val = 0;
   
    int ret = nvram_config_get(NV_COUNTER_KEY, &saved_val, sizeof(saved_val));
    
    if (ret != sizeof(saved_val)) {
        return 0; 
    }
    return saved_val;
}

/*
int test_for_nvram(void)
{
       int data_len, i;

        printk("\nNVRAM testing\n");
        printk("==========================\n");

        nvram_config_set("ucfg", "udata", 6);
//        nvram_config_set_factory("fcfg", "fdata", 6);

        data_len = nvram_config_get("ucfg", test_nvram_buf, sizeof(test_nvram_buf));
        if (data_len < 0) {
                printk("cannot find config ucfg\n");
                return -EIO;
        }

        printk("nvram user: ucfg: %s ,data_len = %d \n", test_nvram_buf,data_len);

        data_len = nvram_config_get_factory("fcfg", test_nvram_buf, sizeof(test_nvram_buf));
        if (data_len < 0) {
                printk("cannot find config fcfg\n");
                return -EIO;
        }

        printk("nvram user: fcfg: %s\n", test_nvram_buf);

	return 0;
}*/
