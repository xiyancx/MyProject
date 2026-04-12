/**
 * Copyright (C) 2013 Guangzhou Tronlong Electronic Technology Co., Ltd. - www.tronlong.com
 *
 * @file c66x_gpio.c
 *
 * @brief This file contains the lower level function to access GPIO.
 *
 * @author Tronlong <support@tronlong.com>
 *
 * @version V1.0
 *
 * @date 2020-08-09
 *
 **/

/* Compiler Header files */
#include <stdint.h>

/* CSL Header file */
#include <ti/csl/cslr_gpio.h>
#include <ti/csl/csl_gpio.h>
#include <ti/csl/csl_gpioAux.h>

#include "c66x_gpio.h"

/**
 * @brief This function configures the specified GPIO's direction
 *
 * @param gpio_num GPIO number to configure
 *
 * @param direction GPIO direction to configure(GPIO_OUT/GPIO_IN)
 *
 * @return NULL
 */
void gpio_set_direction(uint32_t gpio_num, gpio_direction direction)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    if(direction == GPIO_OUT) {
        /* Set gpio pin as output mode */
        CSL_GPIO_setPinDirOutput(h_gpio, gpio_num);
    } else {
        /* Set gpio pin as input mode */
        CSL_GPIO_setPinDirInput (h_gpio, gpio_num);
    }
}

/**
 * @brief This function configures the GPIO[7:0]'s direction
 *
 * @param direction GPIO direction to configure(GPIO_OUT/GPIO_IN)
 *
 * @return NULL
 */
void gpio_set_databus_direction(gpio_direction direction)
{
    uint32_t pin_num;

    for(pin_num = GPIO_0; pin_num <= GPIO_7; pin_num++) {
        /* Set gpio pin as output/input mode */
        gpio_set_direction(pin_num, direction);
    }
}

/**
 * @brief This function sets the specified GPIO's pin state to 1
 *
 * @param gpio_num GPIO number to configure
 *
 * @return NULL
 *
 * @note The specified GPIO should be configured as output
 */
void gpio_set_output(uint32_t gpio_num)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    CSL_GPIO_setOutputData(h_gpio, gpio_num);
}

/**
 * @brief This function Clears the specified GPIO's pin state to 0
 *
 * @param gpio_num GPIO number to configure
 *
 * @return NULL
 *
 * @note The specified GPIO should be configured as output
 */
void gpio_clear_output(uint32_t gpio_num)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    CSL_GPIO_clearOutputData(h_gpio, gpio_num);
}

/**
 * @brief This function gets the specified GPIO's pin state
 *
 * @param gpio_num GPIO number to read input state
 *
 * @return Input state of GPIO if success, else GPIO status
 *
 * @note The specified GPIO should be configured as input
 */
uint8_t gpio_read_input(uint32_t gpio_num)
{
    uint8_t inData = 0;
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    CSL_GPIO_getInputData(h_gpio, gpio_num, &inData);

    return inData;
}

/**
 * @brief This function sets the GPIO[7:0] pins state
 *
 * @param val Value to set as output
 *
 * @return NULL
 *
 * @note The GPIO[7:0] should be configured as output
 */
void gpio_write_databus(uint8_t val)
{
    uint32_t pin_num;
    uint8_t value;

    for(pin_num = GPIO_0; pin_num <= GPIO_7; pin_num++) {
        value = (val >> pin_num) & 0x1;
        if (value == GPIO_HIGH) {
            gpio_set_output(pin_num);
        } else {
            gpio_clear_output(pin_num);
        }
    }
}

/**
 * @brief This function gets the GPIO[7:0] pins state
 *
 * @param void
 *
 * @return Input state of GPIO[7:0]
 *
 * @note The GPIO[7:0] should be configured as input
 */
uint8_t gpio_read_databus(void)
{
    uint32_t pin_num;
    uint8_t value, bitval;

    /* initialize variables */
    value = 0;
    for(pin_num = GPIO_0; pin_num <= GPIO_7; pin_num++) {
        bitval = gpio_read_input(pin_num);
        value |= bitval << pin_num;
    }

    return value;
}

/**
 * @brief This function Enables GPIO interrupts to CPU
 *
 * @param void
 *
 * @return NULL
 */
void gpio_enable_global_interrupt(void)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    /* GPIOREGS->BINTEN |= 0x01 */
    CSL_GPIO_bankInterruptEnable(h_gpio, GPIOBANKNUM);
}

/**
 * @brief This function Disables GPIO interrupts to CPU
 *
 * @param void
 *
 * @return NULL
 */
void gpio_disable_global_interrupt(void)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    /* GPIOREGS->BINTEN = 0x00 */
    CSL_GPIO_bankInterruptDisable(h_gpio, GPIOBANKNUM);
}

/**
 * @brief This function sets specified GPIO's rising edge interrupt
 *
 * @param gpio_num GPIO number to configure
 *
 * @return NULL
 */
void gpio_set_risingedge_interrupt(uint32_t gpio_num)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    /* GPIOREGS->SET_RIS_TRIG |= (1 << gpio_num) */
    CSL_GPIO_setRisingEdgeDetect(h_gpio, gpio_num);
}

/**
 * @brief This function clears specified GPIO's rising edge interrupt
 *
 * @param gpio_num GPIO number to configure
 *
 * @return NULL
 */
void gpio_clear_risingedge_interrupt(uint32_t gpio_num)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    /* GPIOREGS->CLR_RIS_TRIG |= (1 << gpio_num) */
    CSL_GPIO_clearRisingEdgeDetect(h_gpio, gpio_num);
}

/**
 * @brief This function sets specified GPIO's falling edge interrupt
 *
 * @param gpio_num GPIO number to configure
 *
 * @return NULL
 */
void gpio_set_fallingedge_interrupt(uint32_t gpio_num)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    /* GPIOREGS->SET_FAL_TRIG |= (1 << gpio_num) */
    CSL_GPIO_setFallingEdgeDetect(h_gpio, gpio_num);
}

/**
 * @brief This function clears specified GPIO's falling edge interrupt
 *
 * @param gpio_num GPIO number to configure
 *
 * @return NULL
 */
void gpio_clear_fallingedge_interrupt(uint32_t gpio_num)
{
    CSL_GpioHandle h_gpio;

    /* Open the CSL GPIO Module 0 */
    h_gpio = CSL_GPIO_open(0);

    /* GPIOREGS->CLR_FAL_TRIG |= (1 << gpio_num) */
    CSL_GPIO_clearFallingEdgeDetect(h_gpio, gpio_num);
}
