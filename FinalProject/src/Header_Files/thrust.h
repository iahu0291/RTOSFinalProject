/*
 * thrust.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_THRUST_H_
#define SRC_HEADER_FILES_THRUST_H_

enum thrust_possibilities{
  thrust_none = 0,
  thrust_min = 1,
  thrust_max = 2
};

struct craft_thrust_struct {
  int current_thrust;
  int current_fuel;
};

// Function Prototypes

void update_thrust_data(struct craft_thrust_struct* craft_thrust_data, int new_thrust);

#endif /* SRC_HEADER_FILES_THRUST_H_ */
