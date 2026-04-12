/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file c66x_gpio.h
 *
 * @brief This file is the header file for GPIO module.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2020-08-09
 *
 **/

#ifndef C66X_GPIO_H_
#define C66X_GPIO_H_

/* ================ Macros ================ */
/*
 *  GPIO_0 -- GPIO_15 if for c6678
 *  GPIO_0 -- GPIO_31 if for c665x
 */
#define GPIO_0                  (0)
#define GPIO_1                  (1)
#define GPIO_2                  (2)
#define GPIO_3                  (3)
#define GPIO_4                  (4)
#define GPIO_5                  (5)
#define GPIO_6                  (6)
#define GPIO_7                  (7)
#define GPIO_8                  (8)
#define GPIO_9                  (9)
#define GPIO_10                 (10)
#define GPIO_11                 (11)
#define GPIO_12                 (12)
#define GPIO_13                 (13)
#define GPIO_14                 (14)
#define GPIO_15                 (15)
#define GPIO_16                 (16)
#define GPIO_17                 (17)
#define GPIO_18                 (18)
#define GPIO_19                 (19)
#define GPIO_20                 (20)
#define GPIO_21                 (21)
#define GPIO_22                 (22)
#define GPIO_23                 (23)
#define GPIO_24                 (24)
#define GPIO_25                 (25)
#define GPIO_26                 (26)
#define GPIO_27                 (27)
#define GPIO_28                 (28)
#define GPIO_29                 (29)
#define GPIO_30                 (30)
#define GPIO_31                 (31)

#define GPIOBANKNUM             (0)

#define GPIO_LOW                (0)
#define GPIO_HIGH               (1)
#define INVALID_GPIO_NUMBER     (2)
#define INVALID_GPIO_DIRECTION  (3)
#define INVALID_GPIO_STATE      (4)

typedef enum _gpio_direction
{
    GPIO_OUT = 0, ///< gpio output mode
    GPIO_IN ///< gpio input mode
} gpio_direction;

/* ================ Function declarations ================ */
void gpio_set_direction(uint32_t gpio_num, gpio_direction direction);
void gpio_set_databus_direction(gpio_direction direction);
void gpio_set_output(uint32_t gpio_num);
void gpio_clear_output(uint32_t gpio_num);
uint8_t gpio_read_input(uint32_t gpio_num);
void gpio_write_databus(uint8_t val);
uint8_t gpio_read_databus(void);
void gpio_enable_global_interrupt(void);
void gpio_disable_global_interrupt(void);
void gpio_set_risingedge_interrupt(uint32_t gpio_num);
void gpio_clear_risingedge_interrupt(uint32_t gpio_num);
void gpio_set_fallingedge_interrupt(uint32_t gpio_num);
void gpio_set_fallingedge_interrupt(uint32_t gpio_num);
void gpio_clear_fallingedge_interrupt(uint32_t gpio_num);

#endif /* #ifndef C66X_GPIO_H_ */
