/*
 * structs.h
 *
 *  Created on: Oct 22, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_STRUCTS_H_
#define SRC_HEADER_FILES_STRUCTS_H_

// Thrust data structs
enum thrust_possibilities{
  thrust_none = 0,
  thrust_min = 1,
  thrust_max = 2
};

struct craft_thrust_struct {
  int current_thrust;
  int current_fuel;
};


// Direction data structs
enum rotational_thrust_options{
  hard_ccw = -2,
  soft_ccw = -1,
  no_rot = 0,
  soft_cw = 1,
  hard_cw = 2
};

struct craft_direction_struct{
  int current_direction; // Current angle in degrees
  int current_ang_momentum; //How fast the ship is currently spinning, positive is CW and negative is CCW
  int rotational_thrust; //Which rotational direction and how hard are we adjusting spin
};

// Position data structs

struct craft_position_struct{
  int current_x_position, current_y_position;
};


struct speed_struct{
  int currentSpeed;
  int numIncreases;
  int numDecreases;
};

struct direction_struct{
  int currentDirection;
  int numLeftTurns;
  int numRightTurns;
  int lastChangeTimestamp;
};


#endif /* SRC_HEADER_FILES_STRUCTS_H_ */
