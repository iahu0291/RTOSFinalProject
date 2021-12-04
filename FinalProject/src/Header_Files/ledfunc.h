/*
 * ledfunc.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_LEDFUNC_H_
#define SRC_HEADER_FILES_LEDFUNC_H_

#include "structs.h"

void update_led_control_struct(struct led_control_struct *led_ctrl, int period, int dutyCycle);

void input_led_ctrl_data(struct led_control_struct *led_ctrl, int led_ctrl_int);

#endif /* SRC_HEADER_FILES_LEDFUNC_H_ */
