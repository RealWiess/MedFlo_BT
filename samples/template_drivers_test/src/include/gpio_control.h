#ifndef GPIO_CONTROL_H
#define GPIO_CONTROL_H

void gpio_control_init(void);
void init_gpio_entrysleep(void);
void init_gpio_exitsleep(void);
void turn_green_led_off(void);
void turn_green_led_on(void);
void turn_red_led_off(void);
void turn_red_led_on(void);
void display_led(void);

#endif /* GPIO_CONTROL_H */
