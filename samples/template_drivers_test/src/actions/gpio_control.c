#include <zephyr.h>
#include <drivers/gpio.h>
#include <sys/printk.h>
#include "gpio_control.h"
#include "bt_le_op.h"
#include "soc.h"
#include <drivers/watchdog.h>
#include "common.h"



#define INPUT_PIN   18
#define LED_GREEN_OUTPUT_PIN   	20
#define LED_RED_OUTPUT_PIN   	21
#define LED2_RED_OUTPUT_PIN   	7 //gpio B


//#define GPIO_REG_BASE			0x40068000
//#define GPIO_CTL0				0x4
//#define GPIO_REG_CTL(base, pin)	((base) + GPIO_CTL0 + (pin) * 4)
//#define GPIOB_PIN(pin)	        (pin - 32)

static const struct device *gpio_dev;
static struct gpio_callback gpio_cb;
static struct k_work adv_update_work;
extern uint8_t g_u8_gpio18_status;
extern bool g_battery_low;   /* 低電量旗標，由 main.c 電池監控設定 */

void deActive_red_led(void);
void deActive_green_led(void);
void check_gpio_mfp(void);

void adv_update_handler(struct k_work *work) {
    bt_refresh_advertising();
}

static void gpio18_callback(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int val ;
    val = gpio_pin_get(dev, INPUT_PIN);
	bt_update_gpio18_status(val);
	printk("gpio8 callback\r\n");

	k_work_submit(&adv_update_work);
}
/*
void gpio_control_init(void){
	int ret;
	const struct device* gpio_dev = device_get_binding(CONFIG_GPIO_A_NAME);
	if (!gpio_dev) {
		printk("gpio device bind error!\n");
		return;
	}
    ret = gpio_pin_configure(gpio_dev, INPUT_PIN, GPIO_INPUT | GPIO_PULL_UP | GPIO_INT_DEBOUNCE);
	if (ret != 0) {
		printk("Failed to configure GPIO18 as input: %d\n", ret);
		return;
	}
	//config GPIO18 to normal gpio mode 
	gpio_pin_configure(gpio_dev, 20, GPIO_OUTPUT|GPIO_OUTPUT_INIT_HIGH);
	gpio_pin_configure(gpio_dev, 21, GPIO_OUTPUT|GPIO_OUTPUT_INIT_HIGH);

	printk("gpio-mfp:0x%x\n", sys_read32(GPIO18_CTL));
	printk("gpio-mfp:0x%x\n", sys_read32(GPIO20_CTL));
	printk("gpio-mfp:0x%x\n", sys_read32(GPIO21_CTL));
	gpio_pin_set(gpio_dev, 21, 0);
	gpio_pin_set(gpio_dev, 20, 0);
}*/
void gpio_control_init(void)
{
    int ret;
	
	
	k_work_init(&adv_update_work, adv_update_handler);
	
	printk("====Before initialization =====\n\r");
	printk("gpio20-mfp:0x%x\n", sys_read32(GPIO20_CTL));
	printk("gpio21-mfp:0x%x\n", sys_read32(GPIO21_CTL));
	printk("gpio7-mfp:0x%x\n", sys_read32(GPIO7_CTL));
	printk("gpio9-mfp:0x%x\n", sys_read32(GPIO9_CTL));
	printk("=================================\n\r");
	
	sys_write32(0x3000, GPIO20_CTL); 
	sys_write32(0x3000, GPIO21_CTL);
//	sys_write32(0x3000, GPIO9_CTL); 
//	sys_write32(0x3000, GPIO7_CTL);
   
	gpio_dev = device_get_binding(CONFIG_GPIO_A_NAME);
	if (!gpio_dev) {
		printk("Error: cannot bind to %s\n", CONFIG_GPIO_A_NAME);
		return;
	}
  
	ret = gpio_pin_configure(gpio_dev, INPUT_PIN, GPIO_INPUT | GPIO_PULL_UP | GPIO_INT_DEBOUNCE);
	if (ret != 0) {
		printk("Failed to configure GPIO18 as input: %d\n", ret);
		return;
	}

	ret = gpio_pin_interrupt_configure(gpio_dev, INPUT_PIN, GPIO_INT_EDGE_BOTH);
	if (ret != 0) {
		printk("Failed to configure interrupt on GPIO18: %d\n", ret);
		return;
	}

	gpio_init_callback(&gpio_cb, gpio18_callback, BIT(INPUT_PIN));
	ret = gpio_add_callback(gpio_dev, &gpio_cb);
	printk("add callback: %d\n", ret);
	
    printk("====  After initialization =====\n\r"); 
	ret = gpio_pin_configure(gpio_dev, LED_GREEN_OUTPUT_PIN, GPIO_OUTPUT_LOW); //#define LED_GREEN_OUTPUT_PIN   	20
	if (ret != 0) {
		printk("Failed to configure GPIO20 as output: %d\n", ret);
		return;
	}
	printk("gpio20-mfp:0x%x\n", sys_read32(GPIO20_CTL));
	gpio_pin_set(gpio_dev, LED_GREEN_OUTPUT_PIN, 0);
	
	
	ret = gpio_pin_configure(gpio_dev, LED_RED_OUTPUT_PIN, GPIO_OUTPUT_LOW);//#define LED_RED_OUTPUT_PIN   	21
	if (ret != 0) {
		printk("Failed to configure GPIO21 as output: %d\n", ret);
		return;
	}
	gpio_pin_set(gpio_dev, LED_RED_OUTPUT_PIN, 0);
	printk("gpio21-mfp:0x%x\n", sys_read32(GPIO21_CTL));
	
	/*ret = gpio_pin_configure(gpio_dev, 7, GPIO_OUTPUT_LOW);
	if (ret != 0) {
		printk("Failed to configure GPIO7 as output: %d\n", ret);
		return;
	}
	gpio_pin_set(gpio_dev, 7, 0);
	printk("gpio7-mfp:0x%x\n", sys_read32(GPIO7_CTL));
	
	ret = gpio_pin_configure(gpio_dev, 9, GPIO_OUTPUT_LOW);
	if (ret != 0) {
		printk("Failed to configure GPIO9 as output: %d\n", ret);
		return;
	}
	printk("gpio9-mfp:0x%x\n", sys_read32(GPIO9_CTL));
	gpio_pin_set(gpio_dev, 9, 0);
	*/
	printk("===========================\r\n");
}

void init_gpio_entrysleep(void){

}




void init_gpio_exitsleep(void){

}



void display_led(void){
	
	int val = gpio_pin_get(gpio_dev, INPUT_PIN); // IF(KEY==1)GREEN LED ON ELSE RED LED ON
	printk("KEY1 IN =%x\n\r",val);
	bt_update_gpio18_status(val);

	/* 低電量時關閉所有 LED 省電，繼續廣播直到沒電 */
	if (g_battery_low) {
		gpio_pin_set(gpio_dev, LED_GREEN_OUTPUT_PIN, 0);
		gpio_pin_set(gpio_dev, LED_RED_OUTPUT_PIN, 0);
		return;
	}

	if(val!=0){
		gpio_pin_set(gpio_dev, LED_GREEN_OUTPUT_PIN, 1);
		gpio_pin_set(gpio_dev, LED_RED_OUTPUT_PIN, 0);
//		gpio_pin_set(gpio_dev, 7, 0);
//		gpio_pin_set(gpio_dev, 9, 0);
//		printk("====green led on=====\n\r");
	}
	else{
		gpio_pin_set(gpio_dev, LED_GREEN_OUTPUT_PIN, 0);
		gpio_pin_set(gpio_dev, LED_RED_OUTPUT_PIN, 1);
//		gpio_pin_set(gpio_dev, 7, 0);
//		gpio_pin_set(gpio_dev, 9, 0);
//		printk("=====red led on======\n\r");
	}
	
}

void turn_led_all_off(void){
	
	gpio_pin_set(gpio_dev, LED_GREEN_OUTPUT_PIN, 0);
	gpio_pin_set(gpio_dev, LED_RED_OUTPUT_PIN, 0);
	gpio_pin_configure(gpio_dev, 7, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio_dev, 9, GPIO_OUTPUT_LOW);
	printk("====led all off =====\n\r");
}

void check_gpio_mfp(void){
	printk("====gpio state when wake up form S3  =====\n\r");
	printk("gpio20-mfp:0x%x\n", sys_read32(GPIO20_CTL));
	printk("gpio21-mfp:0x%x\n", sys_read32(GPIO21_CTL));
	printk("gpio7-mfp:0x%x\n", sys_read32(GPIO7_CTL));
	printk("gpio9-mfp:0x%x\n", sys_read32(GPIO9_CTL));
	
}

void init_gpio_led(void){
	sys_write32(0x3000, GPIO20_CTL); 
	sys_write32(0x3040, GPIO21_CTL);
//	sys_write32(0x3040, GPIO7_CTL); 
//	sys_write32(0x3040, GPIO9_CTL);
//	gpio_pin_set(gpio_dev, 20, 0);
//	gpio_pin_set(gpio_dev, 21, 0);
//gpio_pin_set(gpio_dev, 7, 0);
//	gpio_pin_set(gpio_dev, 9, 0);
	gpio_pin_configure(gpio_dev, LED_GREEN_OUTPUT_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio_dev, LED_RED_OUTPUT_PIN, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio_dev, 7, GPIO_OUTPUT_LOW);
	gpio_pin_configure(gpio_dev, 9, GPIO_OUTPUT_LOW);
	check_gpio_mfp();
		

}