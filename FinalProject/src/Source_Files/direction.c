/*
 * direction.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */
enum rotational_thrust_options{
  hard_ccw = -2,
  soft_ccw = -1,
  no_rot = 0,
  soft_cw = 1,
  hard_cw = 2
};

struct craft_direction_struct{
  int currentDirection; // Current angle in degrees
  int currentAngMomentum; //How fast the ship is currently spinning, positive is CW and negative is CCW
  int rotational_thrust; //Which rotational direction and how hard are we adjusting spin
};
