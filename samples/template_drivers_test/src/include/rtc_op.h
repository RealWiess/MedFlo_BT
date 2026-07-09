#ifndef RTC_OP_H
#define RTC_OP_H

#include <zephyr.h>
#include "msg_manager.h"
#include <device.h>
#define RTC_PERIOD_US (5000000);
void rtc_start(uint32_t period);
void rtc_init_once(void);
void rtc_setup_wakeup(uint32_t );

#endif /* RTC_OP_H */
