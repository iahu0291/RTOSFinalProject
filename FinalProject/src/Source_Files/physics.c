/*
 * physics.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */
#include "physics.h"

// LOCAL VARIABLES

static const int xMultVals[] ={1004,
    946,
    851,
    724,
    569,
    392,
    200,
    0,
    -200,
    -392,
    -569,
    -724,
    -851,
    -946,
    -1004,
    -1024,
    -1004,
    -946,
    -851,
    -724,
    -569,
    -392,
    -200,
    0,
    200,
    392,
    569,
    724,
    851,
    946,
    1004};

static const int yMultVals[] = {200,
    392,
    569,
    724,
    851,
    946,
    1004,
    1024,
    1004,
    946,
    851,
    724,
    569,
    392,
    200,
    0,
    -200,
    -392,
    -569,
    -724,
    -851,
    -946,
    -1004,
    -1024,
    -1004,
    -946,
    -851,
    -724,
    -569,
    -392,
    -200};



void tick_update_position(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct craft_position_struct *position_data, struct game_settings_struct *settings){

  // Calculate new rotation rate from angular thrust
  switch(direction_data->rotational_thrust){
    case hard_ccw:
      direction_data->current_rotation_rate -= settings->rotationSpeedQuantaMax;
      break;
    case soft_ccw:
      direction_data->current_rotation_rate -= settings->rotationSpeedQuantaMin;
      break;
    case soft_cw:
      direction_data->current_rotation_rate += settings->rotationSpeedQuantaMin;
      break;
    case hard_cw:
      direction_data->current_rotation_rate += settings->rotationSpeedQuantaMax;
      break;
  }


  // Calculate new rotation angle from angular rotation rate
  int nextDirection = direction_data->current_direction + direction_data->current_rotation_rate;
  if(nextDirection >= 256){
    nextDirection -= 256;
  } // nextDirection will range from 0 - 255. 0 will represent pointing right

  direction_data->current_direction = nextDirection;


  // Calculate new velocity from thrust, direction, & gravity
  int thrustOutput = (thrust_data->current_thrust * settings->maxThrust) >> 1;
  struct x_y_struct xyAccel;
  int totalMass = settings->vehicleMass + thrust_data->current_fuel;
  // TODO: Make a final choice: Divide by totalMass b4 function, reducing accuracy,
  // Or divide by totalMass after function twice, reducing speed.
  divide_thrust(thrustOutput / totalMass, direction_data->current_direction, &xyAccel);
  position_data->current_x_vel += xyAccel.x;
  position_data->current_y_vel += xyAccel.y - settings->gravity;


  // Calculate new position from velocity
  position_data->current_x_position += position_data->current_x_vel;
  position_data->current_y_position += position_data->current_y_vel;

}

void tick_burn_fuel(struct craft_thrust_struct *thrust_data, struct game_settings_struct *settings){
  int kg_fuel_burned = ((thrust_data->current_thrust * settings->maxThrust) >> 1) // Thrust in Newtons
      / settings->conversionEfficiency; // N/kg
  thrust_data->current_thrust -= kg_fuel_burned;
}

int check_landing(struct craft_position_struct *position_data, struct game_settings_struct *settings){
  if(position_data->current_x_position > settings->xMax ||
      position_data->current_x_position < settings->xMin ||
      position_data->current_y_position > settings->yMax){
    return game_ship_crashed;
  }
  else if(position_data->current_y_position > settings->yMin){
    return game_ship_flying;
  }
  else{
    if(abs(position_data->current_x_vel) > settings->maxHLandingSpeed ||
        abs(position_data->current_y_vel) > settings->maxVLandingSpeed){
      return game_ship_crashed;
    }
    else{
        return game_ship_landed;
    }
  }
}

void divide_thrust(int thrust, int direction, struct x_y_struct *xyStruct){
  int directionLookup = ((direction + 4) % 256) >> 3; //Direction divided by 8
  xyStruct->x = (thrust * xMultVals[directionLookup]) >> 10;
  xyStruct->y = (thrust * yMultVals[directionLookup]) >> 10;
}
