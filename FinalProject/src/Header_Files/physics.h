/*
 * physics.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_PHYSICS_H_
#define SRC_HEADER_FILES_PHYSICS_H_

#include "structs.h"

// Function Prototypes
void tick_update_position(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct craft_position_struct *position_data, struct game_settings_struct *settings);

void tick_burn_fuel(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct game_settings_struct *settings);

int check_landing(struct craft_position_struct *position_data, struct game_settings_struct *settings);

#endif /* SRC_HEADER_FILES_PHYSICS_H_ */
