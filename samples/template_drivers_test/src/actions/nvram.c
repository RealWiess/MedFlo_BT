#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/nvram_config.h>
#include <soc_irq.h>
#include "common.h"

#define KEY_BOOT_COUNT "BOOT_COUNT"
#define storeCounterCount 7// 9.5*7 == 66.5Sec
#define DeviceMaximumRunningCount 63664 // 63664*9.5 =60488Sec.==168 Hour
uint32_t counter = 0;
uint32_t last_counter=0;
void update_counter_in_nvram(void);

void update_counter(void){

	counter++;
	update_counter_in_nvram();
/*	if((counter-last_counter)>=storeCounterCount){
		update_counter_in_nvram();
		last_counter=counter;
	}*/
}
void Load_Counter_First(void){
	 int ret;
//	 uint32_t saved_val = 0;
//     ret = nvram_config_get(KEY_BOOT_COUNT, &saved_val, sizeof(saved_val));
	 counter = 0; //no record set counter= to 0;
	
//	 ret = nvram_config_set(KEY_BOOT_COUNT, &counter, sizeof(counter));
}

/*
void Load_Counter_First(void){
	 int ret;
	 uint32_t saved_val = 0;
    ret = nvram_config_get(KEY_BOOT_COUNT, &saved_val, sizeof(saved_val));
	 if (ret < 0) {
         counter = 0; //no record set counter= to 0;
		 printk("===nvarm initial error ==\r\n");
		 ret = nvram_config_set(KEY_BOOT_COUNT, &counter, sizeof(counter));
    }
	 else{
		 counter=saved_val;
		 ret = nvram_config_set(KEY_BOOT_COUNT, &counter, sizeof(counter));
		 if (ret == 0) 
		{
			printk("initial counter=%u\n", counter);
		} 
		else
		{
			printk("initial counter Error: %d\n", ret);
			
		}

	 }
	 
}*/
void update_counter_in_nvram(void) {
  
    int ret;
	printk("===update_counter_in_nvram===\r\n");
	if(counter>=DeviceMaximumRunningCount){
		counter=DeviceMaximumRunningCount+10;
	}
//	ret = nvram_config_set(KEY_BOOT_COUNT, &counter, sizeof(counter));
//	if (ret == 0) 
//		{
//			printk("Boot count updated to: %u\n", counter);
//		} 
//		else
//		{
//			printk("Failed to save boot count! Error: %d\n", ret);
//		}
	
  
	printk("==========end  update_counter_in_nvram ==============\r\n");
}

int checkDeviceLifeCycle(void){
	return 0;  /* JOHN01: lifecycle limit disabled for battery-life testing */
}
