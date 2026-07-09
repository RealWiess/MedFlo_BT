#ifndef _PWM_TEST_H_
#define _PWM_TEST_H_
/*
 * hardware connection: GPIO18----LED
 */
#define PWM_REG_BASE_ADDR 0x40060000
#define PWM0_CTL 	(PWM_REG_BASE_ADDR + 0x000)
#define PWM0_CH_CTL0 	(PWM_REG_BASE_ADDR + 0x004)
#define PWM0_CNTMAX 	(PWM_REG_BASE_ADDR + 0x010)
#define PWM0_CMP0 	(PWM_REG_BASE_ADDR + 0x014)

#define PWM2_CTL	(PWM_REG_BASE_ADDR + 0X200)
#define PWM2_REPEAT	(PWM_REG_BASE_ADDR + 0X20C)
#define PWM2_CNTMAX	(PWM_REG_BASE_ADDR + 0x210)
#define PWM2_BTH_A	(PWM_REG_BASE_ADDR + 0x230)
#define PWM2_BTH_B	(PWM_REG_BASE_ADDR + 0x234)
#define PWM2_BTH_C	(PWM_REG_BASE_ADDR + 0x238)
#define PWM2_BTH_D	(PWM_REG_BASE_ADDR + 0x23c)
#define PWM2_BTH_E	(PWM_REG_BASE_ADDR + 0x240)
#define PWM2_BTH_F	(PWM_REG_BASE_ADDR + 0x244)
#define PWM2_BTH_HL	(PWM_REG_BASE_ADDR + 0x248)
#define PWM2_BTH_ST	(PWM_REG_BASE_ADDR + 0x24c)
//mfp config
#define PWM2		(3<<0)

int pwm_fixed_mode(void);
int pwm_breath_mode(void);
int pwm_programme_mode(void);

#define TEST_FOR_FIXED_MODE	0
#define TEST_FOR_BREATH_MODE	1
#define TEST_FOR_PROGRAMME_MODE 0

void test_for_pwm(void);

#endif /* _PWM_TEST_H_ */

