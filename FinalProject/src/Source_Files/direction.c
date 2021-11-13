/*
 * direction.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#include "direction.h"

void update_thrust_data(struct craft_direction_struct* craft_direction_data, int new_direction){
  craft_direction_data->rotational_thrust = new_direction;
}
