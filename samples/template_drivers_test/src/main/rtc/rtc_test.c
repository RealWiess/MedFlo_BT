/*
 * Copyright (c) 2022 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr.h>
#include <sys/printk.h>
#include <device.h>
#include <drivers/spi.h>
#include <soc_regs.h>
#include <soc_timer.h>>
#include <soc_irq.h>
#include "common.h"
#include "rtc_test.h"
#include <string.h>



uint32_t rtc_period = 108000; // 108ms
static void _rtc_callback(void * arg)
{
    struct acts_rtc *rtc_base = (struct acts_rtc *)RTC_BASE;
    uint32_t last_load = rtc_period * (sys_clock_hw_cycles_per_sec() / 1000000);
    uint32_t cmp_val = acts_rtc_new_cmp_val(last_load - 1);

    if (!(rtc_base->rtc_ctl & RTC_CTL_RTC_VAL3_IRQ_PD))
        return;

    /* clear irq pending */
    rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_PD; 
    while (rtc_base->rtc_ctl & RTC_CTL_RTC_VAL3_IRQ_PD);

    printk("Ticks now : %lld\n",  z_tick_get());

    /* reload */
    rtc_base->rtc_ctl &= ~RTC_CTL_RTC_CMP3_EN;
    rtc_base->rtc_cmp3 = cmp_val;
    rtc_base->rtc_ctl |= RTC_CTL_RTC_CMP3_EN;
}

void rtc_start(uint32_t rtc_period)
{
    struct acts_rtc *rtc_base = (struct acts_rtc *)RTC_BASE;

    printk("########### rtc test ##########\n");

    /* calc cmp value */
    uint32_t last_load =  rtc_period * (sys_clock_hw_cycles_per_sec() / 1000000);
    uint32_t cmp_val = acts_rtc_new_cmp_val(last_load - 1);

    /* clear pending flag, set cmp value, and enable cmp timer */
    rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_PD;
    rtc_base->rtc_cmp3 = cmp_val; 
    rtc_base->rtc_ctl |= RTC_CTL_RTC_CMP3_EN;

    /* enable irq */
    IRQ_CONNECT(IRQ_ID_CMP3, 1, _rtc_callback, 0, 0);
    irq_enable(IRQ_ID_CMP3);
    rtc_base->rtc_ctl |= RTC_CTL_RTC_VAL3_IRQ_EN | RTC_CTL_RTC_VAL3_IRQ_PD; 
}

