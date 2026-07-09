/*
 * Copyright (c) 2018 Actions (Zhuhai) Technology Co., Ltd
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <kernel.h>
#include <init.h>
#include <sys/printk.h>

#include "sys_log.h"
LOG_MODULE_REGISTER(PWM_TEST, CONFIG_LOG_DEFAULT_LEVEL);

#include "drivers/pwm.h"
#include "drivers/cfg_drv/pinctrl_tai.h"

//#define TEST_FOR_FIXED_MODE      1
//#define TEST_FOR_BREATH_MODE     1
//#define TEST_FOR_REPEAT_MODE     1
#define TEST_FOR_PROGRAMME_MODE  1

void pwm_fixed_mode(void)
{
	const struct device *pwm = NULL;
	pwm = device_get_binding(CONFIG_PWM_NAME);
	if(pwm == NULL) {
		return ;
	}

	while(1){
		for (u16_t i=0; i<200; i++) {
			pwm_pin_set_cycles(pwm, 0, 199, i, 1);
			k_sleep(K_MSEC(5));
		}
		k_sleep(K_MSEC(250));
		for (u16_t j=199; j>0; j--) {
			pwm_pin_set_cycles(pwm, 0, 199, j, 1);
			k_sleep(K_MSEC(5));
		}
		k_sleep(K_MSEC(500));
	}
}

/* pwm group1-ch0 corresponding to pwm index 6 */
//void pwm_breath_mode()
//{
//	const struct device *pwm = NULL;
//	pwm_breath_ctrl_t ctrl;

//	/* get the pwm device */
//	pwm = device_get_binding(CONFIG_PWM_NAME);
//	if(pwm == NULL){
//		return;
//	}

//	ctrl.pwm_count_max = 255;
//	
//	ctrl.stage_high_wait = 200;
//	ctrl.stage_low_wait = 200;

//	ctrl.stage_a_step = 10;
//	ctrl.stage_b_step = 10;
//	ctrl.stage_c_step = 10;
//	ctrl.stage_d_step = 10;
//	ctrl.stage_e_step = 10;
//	ctrl.stage_f_step = 10;

//	ctrl.stage_a_repeat = 10;
//	ctrl.stage_b_repeat = 20;
//	ctrl.stage_c_repeat = 10;
//	ctrl.stage_d_repeat = 10;
//	ctrl.stage_e_repeat = 20;
//	ctrl.stage_f_repeat = 10;

//	ctrl.stage_a_pwm = 0;
//	ctrl.stage_b_pwm = 80;
//	ctrl.stage_c_pwm = 160;
//	ctrl.stage_d_pwm = 255;
//	ctrl.stage_e_pwm = 160;
//	ctrl.stage_f_pwm = 80;
//	ctrl.start_pwm   = 0;
//	ctrl.start_dir   = 0;
////	ctrl.multi_ch_en = 0;

//	ctrl.breath_en = 1;
//	pwm_set_breath_mode(pwm, 6, &ctrl);
//	k_sleep(K_MSEC(7000));

////	/* close breath mode */
////	ctrl.breath_en = 0;
////	pwm_set_breath_mode(pwm, 6, &ctrl);
////}

//#define REPEAT_MAX_CYCLE  16
////static pwm_repeat_ctrl_t     repeat_ctrl;
////static pwm_repeat_cb_data_t  repeat_cb_data;

//int pwm_repeat_cb(void *cb_data, u8_t reason)
//{
//	static u8_t repeat_cycle = 0;
//	pwm_repeat_cb_data_t *repeat_date = (pwm_repeat_cb_data_t *)cb_data;

//	if(repeat_cycle++ < REPEAT_MAX_CYCLE){
//		repeat_date->ctrl.period_cycles = 32000;
//		if(repeat_cycle < 9)
//			repeat_date->ctrl.pulse_cycles.single_pulse_cycles = 4000*repeat_cycle;
//		else
//			repeat_date->ctrl.pulse_cycles.single_pulse_cycles = 32000 - 4000*(repeat_cycle - 8);
//		repeat_date->ctrl.repeat = 2;
//		repeat_date->ctrl.polarity = 1;
//		repeat_date->reload_flag = 1;
//	}
//	else
//	{
//		repeat_date->reload_flag = 0;
//		repeat_cycle = 0;
//	}

//	return 0;
//}

//void pwm_repeat_mode()
//{
//	int ret;
//	const struct device *pwm = NULL;

//	/* get the pwm device */
//	pwm = device_get_binding(CONFIG_PWM_NAME);
//	if(pwm == NULL){
//		return ;
//	}

//	repeat_ctrl.period_cycles = 32000;
//	repeat_ctrl.pulse_cycles.single_pulse_cycles = 100;
//	repeat_ctrl.repeat = 2;
//	repeat_ctrl.polarity = 1;
//	repeat_ctrl.repeat_callback = pwm_repeat_cb;
//	repeat_ctrl.cb_data = &repeat_cb_data;
//	repeat_ctrl.multi_ch_en = 0;

//	ret = pwm_pin_repeat(pwm, 14, &repeat_ctrl);
//	if(ret < 0){
//		return ;
//	}
//}

#define PROG_MAX_CNT 2
#define RAM_LEN 17
static u16_t pwm_ram_buf[RAM_LEN] = {0, 4000, 8000, 12000, 16000, 20000, 24000, 28000, 32000, 28000, 24000, 20000, 16000, 12000, 8000, 4000, 0};
u16_t configed_cycle = 32000;
u16_t configed_repeat = 2;
static u16_t program_max_cnt = 0;
static pwm_program_ctrl_t program_ctrl;	

int pwm_program_cb(void *cb_data, u8_t reason)
{
	if(++program_max_cnt >= PROG_MAX_CNT){
		return 0;
	}

	/* fill the next dma buf data, used repeat cnt as test */
	for(u8_t i = 0; i < RAM_LEN; i++){
		pwm_ram_buf[i] /=2;
	}

	/* update the next dma buf */
	pwm_set_program_mode(device_get_binding(CONFIG_PWM_NAME), 12, &program_ctrl);

	return 0;
}

void pwm_programme_mode(void)
{
	int ret;
	const struct device *pwm = NULL;
	
	/* get the pwm device */
	pwm = device_get_binding(CONFIG_PWM_NAME);
	if(pwm == NULL){
		return ;
	}

	program_max_cnt = 0;

	/* register callback */
	program_ctrl.ram_buf = pwm_ram_buf;
	program_ctrl.ram_buf_len = 2 * RAM_LEN; /* unit: byte */
	program_ctrl.program_callback = pwm_program_cb;
	program_ctrl.multi_ch_en = 0; /* disable other chans in this group, if want to enable other chan, set corresponding bit */
	program_ctrl.use_cntmax_reg_subst_ram = configed_cycle;
	program_ctrl.use_repeat_reg_subst_ram = configed_repeat;

	ret = pwm_set_program_mode(pwm, 12, &program_ctrl);
	if(ret < 0){
		return ;
	}
}

/*
** ATB1113's CONFIG_PWM_MFP can be configed for reference: 
** gpio0_pwm_g0c0p_node, gpio26_pwm_g1c0_node, \
** gpio6_pwm_g2_node, gpio8_pwm_g4_node
*/
void test_for_pwm(void)
{
#if	TEST_FOR_FIXED_MODE
	pwm_fixed_mode();
#elif	TEST_FOR_BREATH_MODE
	pwm_breath_mode();
#elif TEST_FOR_REPEAT_MODE
	pwm_repeat_mode();
#elif TEST_FOR_PROGRAMME_MODE
	pwm_programme_mode();	
#endif
}
/**********************Bug reproduction***************************/
#define BUG_REPRODUCTION  1
#if BUG_REPRODUCTION 

#include "soc_timer.h"

typedef unsigned int k_uint32;

#define app_printf			printk

void app_enable_irq(void)
{
	__asm__ volatile("cpsie i" : : : "memory");
	// __asm__ volatile("cpsie i");
}

void app_disable_irq(void)
{
	__asm__ volatile("cpsid i" : : : "memory");
	// __asm__ volatile("cpsid i");
}


k_uint32 app_clock_time(void)
{
	return (z_tick_get_32() << 1);
}

k_uint32 app_update_delay_ms_time(k_uint32 ui_time)
{
	k_uint32 ui_tick = app_clock_time();
	k_uint32 ui_comp_tick = 0;
	if(ui_time > ui_tick)
	{
		ui_comp_tick = (0xFFFFFFFF - ui_time + ui_tick);
	}
	else
	{
		ui_comp_tick = (ui_tick - ui_time);
	}
	return ui_comp_tick;
}

void app_delayus_use_software(k_uint32 ui_time_param)
{
    if (ui_time_param <= 1)
    {
        return;
    }
	if(ui_time_param < 6000)
	{
		k_busy_wait(ui_time_param);
	}
	else
	{
		k_busy_wait(ui_time_param);
		//k_sleep(K_USEC(ui_time_param));
	}
}

void app_base_tick_test(void)
{
	k_uint32 local_irq_save;
	k_uint32 ui_start_tick = 0,ui_end_tick = 0,ui_handle_tick;
	ui_start_tick = app_clock_time();
	app_delayus_use_software(10000);
	ui_end_tick = app_clock_time();
	ui_handle_tick = app_update_delay_ms_time(ui_start_tick);
	app_printf("base_tick_test=%d,%d(10ms)h=%d;\r\n",ui_start_tick,ui_end_tick,ui_handle_tick);
	ui_start_tick = acts_rtc_get_cnt();
	app_delayus_use_software(10000);
	ui_end_tick = acts_rtc_get_cnt();
	app_printf("base_tick_rtc_test=%d,%d(10ms);\r\n",ui_start_tick,ui_end_tick);
	// CONFIG_SYS_CLOCK_HW_CYCLES_PER_SEC
	local_irq_save = irq_lock();
	ui_start_tick = app_clock_time();
	app_delayus_use_software(10000);
	ui_end_tick = app_clock_time();
	app_printf("base_tick_lock_test=%d,%d(10ms);\r\n",ui_start_tick,ui_end_tick);
	ui_start_tick = acts_rtc_get_cnt();
	app_delayus_use_software(10000);
	ui_end_tick = acts_rtc_get_cnt();
	app_printf("base_tick_lock_rtc_test=%d,%d(10ms);\r\n",ui_start_tick,ui_end_tick);
	irq_unlock(local_irq_save);

	#if 1
	app_disable_irq();
	local_irq_save = irq_lock();
	ui_start_tick = app_clock_time();
	app_delayus_use_software(10000);
	ui_end_tick = app_clock_time(); // disable IRQ  app_clock_time()  is not changed
	app_printf("base_tick_irq_test=%d,%d(10ms);\r\n",ui_start_tick,ui_end_tick);
	ui_start_tick = acts_rtc_get_cnt();
	app_delayus_use_software(10000);
	ui_end_tick = acts_rtc_get_cnt();
	app_printf("base_tick_irq_rtc_test=%d,%d(10ms);\r\n",ui_start_tick,ui_end_tick);
	app_enable_irq();
	app_delayus_use_software(10000);
	#endif
}




#endif /* BUG_REPRODUCTION */