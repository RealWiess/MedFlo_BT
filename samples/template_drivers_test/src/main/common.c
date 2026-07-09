#include <zephyr.h>
#include <sys/printk.h>
#include "soc.h"
#include <drivers/watchdog.h>

void gpio_config_init(uint32_t addr, uint32_t data)
{
	*(volatile uint32_t *)addr = data;
}

void gpio_set_high_val(uint32_t addr, uint32_t bit)
{
	uint32_t temp = *(volatile uint32_t *)addr;
	*(volatile uint32_t *)addr = temp | (1 << bit);
}

void gpio_set_low_val(uint32_t addr, uint32_t bit)
{
	uint32_t temp = *(volatile uint32_t *)addr;
	*(volatile uint32_t *)addr = temp & ~(1 << bit);
}

int __weak btdrv_exit(void)
{
	return 0;
}

#define WD_CTL_EN				(1 << 4)
#define WD_CTL_CLKSEL_MASK		(0x7 << 1)
#define WD_CTL_CLKSEL_SHIFT		(1)
#define WD_CTL_CLKSEL(n)		((n) << WD_CTL_CLKSEL_SHIFT)
#define WD_CTL_CLR				(1 << 0)
#define WD_CTL_IRQ_PD			(1 << 6)
#define WD_CTL_IRQ_EN			(1 << 5)

/* feed watchdog in isr */
void watchdog_isr_feed(void)
{
#ifndef CONFIG_WDG_ISR_FEED_UPDATE_TIMEOUT
	sys_write32(sys_read32(WD_CTL) | WD_CTL_CLR, WD_CTL);
#else
	sys_write32((sys_read32(WD_CTL) & ~WD_CTL_CLKSEL_MASK)
		| WD_CTL_CLKSEL(CONFIG_WDG_ISR_FEED_UPDATE_TIMEOUT) | WD_CTL_CLR, WD_CTL);
#endif
}

void watchdog_disable(void)
{
	sys_write32(0x0, WD_CTL);
	printk("watchdog_disable\n");
}

