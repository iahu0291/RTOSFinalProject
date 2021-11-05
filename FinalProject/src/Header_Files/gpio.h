/*
 * gpio.h
 *
 *  Created on: Sep 7, 2021
 *      Author: Nick
 */

#ifndef SRC_HEADER_FILES_GPIO_H_
#define SRC_HEADER_FILES_GPIO_H_

//***********************************************************************************
// Include files
//***********************************************************************************
#include "em_gpio.h"
#include "bspconfig.h"

//***********************************************************************************
// defined files
//***********************************************************************************

// LED 0 pin is
#define LED0_port   gpioPortF
#define LED0_pin    4u
#define LED0_default  false   // Default false (0) = off, true (1) = on
// LED 1 pin is
#define LED1_port   gpioPortF
#define LED1_pin    5u
#define LED1_default  false // Default false (0) = off, true (1) = on

#define PSH0_port   gpioPortF
#define PSH0_pin    6u
#define PSH0_mode   true

#define PSH1_port   gpioPortF
#define PSH1_pin    7u
#define PSH1_mode   true
//***********************************************************************************
// global variables
//***********************************************************************************


//***********************************************************************************
// function prototypes
//***********************************************************************************
void gpio_open(void);


#endif /* SRC_HEADER_FILES_GPIO_H_ */
