/*
 * ledfunc.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#include "ledfunc.h"

//dutyCycle scales from 0-32, with 32 being a 100% duty cycle and 0 being 0%
void update_led_control_struct(struct led_control_struct *led_ctrl, int period, int dutyCycle){
  led_ctrl->restartPeriod = period;
  int timeOnOff = ((period * dutyCycle) >> 5);

  led_ctrl->timeFromOnToOff = timeOnOff;

}

void input_led_ctrl_data(struct led_control_struct *led_ctrl, int led_ctrl_int){
  update_led_control_struct(led_ctrl,
                            (led_ctrl_int & LED0_PERIOD_BITMASK) >> LED0_PERIOD_SHIFT,
                            (led_ctrl_int & LED0_DUTY_CYCLE_BITMASK) >> LED0_DUTY_CYCLE_SHIFT);

}
