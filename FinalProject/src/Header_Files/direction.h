/*
 * direction.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_DIRECTION_H_
#define SRC_HEADER_FILES_DIRECTION_H_

#include "structs.h"

void update_direction_data(struct craft_direction_struct* craft_direction_data, int new_direction);

void update_direction_thrust_data(struct craft_direction_struct* craft_direction_data, int new_direction);
#endif /* SRC_HEADER_FILES_DIRECTION_H_ */
