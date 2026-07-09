#include <zephyr.h>
#include <sys/printk.h>
#include <string.h>
#include <device.h>
#include <soc_regs.h>
#include <soc_timer.h>
#include "rtc_op.h"
#include "bt_le_op.h"

extern uint32_t counter;
extern int checkDeviceLifeCycle(void);
extern  struct k_timer periodic_timer;
extern  struct k_timer led_periodic_timer;
uint32_t rtc_period = 5000000; /* 5000ms */
#define RTC_PERIOD_US (5000000);
/*
static void _rtc_callback(void *arg)
{
    struct acts_rtc *rtc_base = (struct acts_rtc *)RTC_BASE;
    uint32_t last_load = rtc_period * (sys_clock_hw_cycles_per_sec() / 1000000);
    uint32_t cmp_val = acts_rtc_new_cmp_val(last_load - 1);

    ///clear irq pending (S3 wakeup may have already cleared IRQ_PD before callback runs) 
    rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_PD;
    while (rtc_base->rtc_ctl & RTC_CTL_RTC_VAL3_IRQ_PD);

 //   printk("Ticks now : %lld\n", z_tick_get());

    ///reload 
    rtc_base->rtc_ctl &= ~RTC_CTL_RTC_CMP3_EN;
    rtc_base->rtc_cmp3 = cmp_val;
    rtc_base->rtc_ctl |= RTC_CTL_RTC_CMP3_EN;

    app_to_msg(MSG_BLE_STATE, START_ADV);
	k_timer_start(&periodic_timer, K_MSEC(24500), K_NO_WAIT);
	k_timer_start(&led_periodic_timer, K_MSEC(100), K_MSEC(100));
}
*/
static void _rtc_callback(void *arg)
{
    struct acts_rtc *rtc_base = (struct acts_rtc *)RTC_BASE;

  
    rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_PD;
    while (rtc_base->rtc_ctl & RTC_CTL_RTC_VAL3_IRQ_PD);

   
    rtc_base->rtc_ctl &= ~RTC_CTL_RTC_CMP3_EN;

  
    app_to_msg(MSG_BLE_STATE, START_ADV);
}

void rtc_init_once(void)
{
   
    IRQ_CONNECT(IRQ_ID_CMP3, 1, _rtc_callback, 0, 0);
    irq_enable(IRQ_ID_CMP3);
}

void rtc_start(uint32_t rtc_period)
{
    struct acts_rtc *rtc_base = (struct acts_rtc *)RTC_BASE;
    int ret;
    printk("########### rtc IRQ_ID_CMP3 ##########\n");
	printk("====  counter %x=====\r\n", counter);
    ret=checkDeviceLifeCycle();
	if(ret==0){
		 // calc cmp value 
		uint32_t last_load = rtc_period * (sys_clock_hw_cycles_per_sec() / 1000000);
		uint32_t cmp_val = acts_rtc_new_cmp_val(last_load - 1);
	 
//    printk("last_load %d\n", last_load);
//    printk("cmp_val: %d\n", cmp_val);

		//clear pending flag, set cmp value, and enable cmp timer 
		rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_PD;
		rtc_base->rtc_cmp3 = cmp_val;
		rtc_base->rtc_ctl |= RTC_CTL_RTC_CMP3_EN;
	
		//enable irq 
		IRQ_CONNECT(IRQ_ID_CMP3, 1, _rtc_callback, 0, 0);
		irq_enable(IRQ_ID_CMP3);
		rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_EN | RTC_CTL_RTC_VAL3_IRQ_PD;
	}
   
}

void rtc_setup_wakeup(uint32_t rtc_period_ms)
{
    struct acts_rtc *rtc_base = (struct acts_rtc *)RTC_BASE;
    
   
    if(checkDeviceLifeCycle() != 0) return;

    
    uint32_t last_load = rtc_period_ms * (sys_clock_hw_cycles_per_sec() / 1000000);
    uint32_t cmp_val = acts_rtc_new_cmp_val(last_load - 1);

    //clear pending flag, set cmp value, and enable cmp timer 
	rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_PD;
	rtc_base->rtc_cmp3 = cmp_val;
	rtc_base->rtc_ctl |= RTC_CTL_RTC_CMP3_EN;
 
     // calc cmp value 
    rtc_base->rtc_cmp3 = cmp_val;
    rtc_base->rtc_ctl |= (RTC_CTL_RTC_VAL3_IRQ_EN | RTC_CTL_RTC_CMP3_EN);
	//enable irq 
	rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_EN | RTC_CTL_RTC_VAL3_IRQ_PD;
}




