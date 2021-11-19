/*
 * ledfunc.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

//dutyCycle scales from 0-64, with 64 being a 100% duty cycle and 0 being 0%
void update_led_control_struct(struct led_control_struct *led_ctrl, int period, int dutyCycle){
  led_ctrl->restartPeriod = period;

  led_ctrl->timeFromOnToOff = ((period * dutyCycle) >> 6);
}
