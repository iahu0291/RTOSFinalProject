/*
 * physics.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_PHYSICS_H_
#define SRC_HEADER_FILES_PHYSICS_H_

#include "direction.h"
#include "thrust.h"

// LED Ctrl Bitmasks
#define LED0_DUTY_CYCLE_BITMASK   0b111111
#define LED0_DUTY_CYCLE_SHIFT     0
#define LED0_PERIOD_BITMASK       0b1111111111000000
#define LED0_PERIOD_SHIFT         6

#define LED1_DUTY_CYCLE_BITMASK   0b1111110000000000000000 // LED0_DUTY_CYCLE_BITMASK << 16
#define LED1_DUTY_CYCLE_SHIFT     16
#define LED1_PERIOD_BITMASK       0b11111111110000000000000000000000 // LED0_PERIOD_BITMASK << 16
#define LED1_PERIOD_SHIFT         22

// Function Prototypes
void tick_update_position(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct craft_position_struct *position_data, struct game_settings_struct *settings);

void tick_burn_fuel(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct game_settings_struct *settings);

int check_landing(struct craft_position_struct *position_data, struct game_settings_struct *settings);

unsigned int get_led_ctrls(struct craft_thrust_struct *thrust_data, struct game_settings_struct *settings);

void divide_thrust(int thrust, int angle, int* xThrust, int* yThrust);

#endif /* SRC_HEADER_FILES_PHYSICS_H_ */
