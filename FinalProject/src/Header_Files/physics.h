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



//Easy Sin/Cos Lookup

extern const int xMultVals[32];
extern const int yMultVals[32];

//extern const int xMultVals[32] = {1024,
//    1004,
//    946,
//    851,
//    724,
//    569,
//    392,
//    200,
//    0,
//    -200,
//    -392,
//    -569,
//    -724,
//    -851,
//    -946,
//    -1004,
//    -1024,
//    -1004,
//    -946,
//    -851,
//    -724,
//    -569,
//    -392,
//    -200,
//    0,
//    200,
//    392,
//    569,
//    724,
//    851,
//    946,
//    1004};
//extern const int yMultVals[32] = {0,
//    200,
//    392,
//    569,
//    724,
//    851,
//    946,
//    1004,
//    1024,
//    1004,
//    946,
//    851,
//    724,
//    569,
//    392,
//    200,
//    0,
//    -200,
//    -392,
//    -569,
//    -724,
//    -851,
//    -946,
//    -1004,
//    -1024,
//    -1004,
//    -946,
//    -851,
//    -724,
//    -569,
//    -392,
//    -200};

// Function Prototypes
void tick_update_position(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct craft_position_struct *position_data, struct game_settings_struct *settings);

void tick_burn_fuel(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct game_settings_struct *settings);

int check_landing(struct craft_position_struct *position_data, struct game_settings_struct *settings);

unsigned int get_led_ctrls(struct craft_thrust_struct *thrust_data, struct game_settings_struct *settings);

void divide_thrust(int thrust, int angle, int* xThrust, int* yThrust);

#endif /* SRC_HEADER_FILES_PHYSICS_H_ */
