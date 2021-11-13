/*
 * thrust.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */
#include "thrust.h";


void update_thrust_data(struct craft_thrust_struct *thrust_data, int new_thrust){
  thrust_data->current_thrust = new_thrust;
}
