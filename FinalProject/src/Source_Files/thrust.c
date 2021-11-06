/*
 * thrust.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

enum thrust_possibilities{
  thrust_none = 0,
  thrust_min = 1,
  thrust_max = 2
};

struct craft_thrust_struct {
  int current_thrust;
  int current_fuel;
};

