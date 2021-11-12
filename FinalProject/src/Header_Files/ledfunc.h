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

#endif /* SRC_HEADER_FILES_LEDFUNC_H_ */
