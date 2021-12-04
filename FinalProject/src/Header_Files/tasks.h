/*
 * buttoninput.h
 *
 *  Created on: Sep 10, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_TASKS_H_
#define SRC_HEADER_FILES_TASKS_H_

#include "gpio.h"
#define THRUST_INPUT_TASK_STACK_SIZE      512
#define THRUST_INPUT_TASK_PRIO            20

#define DIRECTION_INPUT_TASK_STACK_SIZE   512
#define DIRECTION_INPUT_TASK_PRIO         20

#define LED0_CTRL_TASK_STACK_SIZE         512
#define LED0_CTRL_TASK_PRIO               20

#define LED0_ON_TASK_STACK_SIZE           512
#define LED0_ON_TASK_PRIO                 20

#define LED1_CTRL_TASK_STACK_SIZE         512
#define LED1_CTRL_TASK_PRIO               20

#define LED1_ON_TASK_STACK_SIZE           512
#define LED1_ON_TASK_PRIO                 20

#define IDLE_TASK_STACK_SIZE              512
#define IDLE_TASK_PRIO                    50

#define LCD_DISPLAY_TASK_STACK_SIZE       512
#define LCD_DISPLAY_TASK_PRIO             15

#define PHYSICS_TASK_STACK_SIZE           512
#define PHYSICS_TASK_PRIO                 10

#define ENDGAME_TASK_STACK_SIZE           512
#define ENDGAME_TASK_PRIO                 20

//Function prototypes
void tasks_init(void);
void capsense_poll(void);

#endif /* SRC_HEADER_FILES_TASKS_H_ */
