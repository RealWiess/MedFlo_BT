 #ifndef _COMMON_H_
#define _COMMON_H_

#define GPIO_BASE_ADDR	0x40068000

#define GPIO18_CTL	(GPIO_BASE_ADDR + 0x004c)
#define GPIO_OUTPUT_EN	(1<<6)
#define GPIO_INPUT_EN	(1<<7)
#define GPIO_ODAT0 	(GPIO_BASE_ADDR + 0x0100)
//mfp config
#define PWM0_0P		(1<<0)

#define GPIO0_CTL	(GPIO_BASE_ADDR + 0x0004)
#define GPIO1_CTL	(GPIO_BASE_ADDR + 0x0008)
#define GPIO2_CTL	(GPIO_BASE_ADDR + 0x000c)
#define GPIO3_CTL	(GPIO_BASE_ADDR + 0x0010)
#define GPIO5_CTL	(GPIO_BASE_ADDR + 0x0018)
#define GPIO6_CTL	(GPIO_BASE_ADDR + 0x001c)
#define GPIO7_CTL	(GPIO_BASE_ADDR + 0x0020)
#define GPIO8_CTL	(GPIO_BASE_ADDR + 0x0024)
#define GPIO9_CTL	(GPIO_BASE_ADDR + 0x0028)
#define GPIO13_CTL	(GPIO_BASE_ADDR + 0x0038)
#define GPIO18_CTL	(GPIO_BASE_ADDR + 0x004c)
#define GPIO20_CTL	(GPIO_BASE_ADDR + 0x0054)
#define GPIO21_CTL	(GPIO_BASE_ADDR + 0x0058)
#define GPIO22_CTL	(GPIO_BASE_ADDR + 0x005c)
#define GPIO26_CTL	(GPIO_BASE_ADDR + 0x006c)

#define CMU_DIGITAL_REGISTER_ADDR	0x40001000
#define CMU_DEVCLKEN1	(CMU_DIGITAL_REGISTER_ADDR + 0x000C)

void gpio_config_init(uint32_t addr, uint32_t data);
void gpio_set_high_val(uint32_t addr, uint32_t bit);
void gpio_set_low_val(uint32_t addr, uint32_t bit);
void watchdog_isr_feed(void);
void watchdog_disable(void);
#endif /* _COMMON_H_ */

