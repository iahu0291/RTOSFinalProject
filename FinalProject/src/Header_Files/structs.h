/*
 * structs.h
 *
 *  Created on: Oct 22, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_STRUCTS_H_
#define SRC_HEADER_FILES_STRUCTS_H_

// Game Info structs
struct game_settings_struct{
  int version;
  int gravity; // Virtual pixels/tick^2
  int vehicleMass;
  int xMin, xMax, yMin, yMax; //Total virtual pixels on screen
  int optionSelected;
  int initialFuelMass;
  int conversionEfficiency;
  int fuelEnergyDensity;
  int maxVLandingSpeed, maxHLandingSpeed; 
  int blackoutAccel, blackoutDuration;
  int initXVel, initYVel;
  int initHPos;
  int rotationQuanta, rotationSpeedQuanta;
};

enum game_states{
  game_ship_flying = 0,
  game_ship_crashed = 1,
  game_ship_landed = 2
};


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
  int current_direction; // Current angle in degrees, 0 being straight up
  int current_ang_momentum; //How fast the ship is currently spinning, positive is CW and negative is CCW
  int rotational_thrust; //Which rotational direction and how hard are we adjusting spin
};

// Position data structs

struct craft_position_struct{
  int current_x_position, current_y_position;
  int current_x_vel, current_y_vel;
};

// LED Control Structs
struct led_control_struct{
  int restartPeriod; // In ms
  int timeFromOnToOff; // In ms 
};


#endif /* SRC_HEADER_FILES_STRUCTS_H_ */
