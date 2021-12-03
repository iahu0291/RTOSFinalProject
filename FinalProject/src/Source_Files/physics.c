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
  int xAccel = 0;
  int yAccel = 0;
  if(!thrust_data->blacked_out){
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

  //  direction_data->current_direction = nextDirection;
    update_direction_data(direction_data, nextDirection);


    // Calculate new velocity from thrust, direction, & gravity
    int thrustOutput = (thrust_data->current_thrust * settings->maxThrust) >> 1;
    int totalMass = settings->vehicleMass + thrust_data->current_fuel;
    // TODO: Make a final choice: Divide by totalMass b4 function, reducing accuracy,
    // Or divide by totalMass after function twice, reducing speed.
    divide_thrust(thrustOutput / totalMass, direction_data->current_direction, &xAccel, &yAccel);
  }
  position_data->current_x_vel += xAccel;
  position_data->current_y_vel += yAccel - settings->gravity;

  if((xAccel^2)+(yAccel^2) >= settings->blackoutAccel){
      // Start Blackout Timer

      thrust_data->blacked_out = 1;
  }

  // Calculate new position from velocity
  position_data->current_x_position += position_data->current_x_vel;
  position_data->current_y_position += position_data->current_y_vel;

}

void tick_burn_fuel(struct craft_thrust_struct *thrust_data, struct craft_direction_struct *direction_data, struct game_settings_struct *settings){
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

unsigned int get_led_ctrls(struct craft_thrust_struct *thrust_data, struct game_settings_struct *settings){
  unsigned int led0_dutyCycle, led1_dutyCycle, led0_period, led1_period;
  unsigned int returnVal = 0;
  led0_dutyCycle = 32 * thrust_data->current_thrust;
  led0_period = 16;
  if(!thrust_data->blacked_out){
    led1_dutyCycle = ((thrust_data->current_thrust * settings->maxThrust) / settings->blackoutAccel) * 64;
    led1_period = 16;
  }
  else{
    led1_dutyCycle  = 32;
    led1_period = 333;
  }
  returnVal += led1_period << LED1_PERIOD_SHIFT;
  returnVal += led1_dutyCycle << LED1_DUTY_CYCLE_SHIFT;
  returnVal += led0_period << LED0_PERIOD_SHIFT;
  returnVal += led0_dutyCycle;

  return returnVal;
}

void divide_thrust(int thrust, int direction, int *xThrust, int* yThrust){
  int directionLookup = ((direction + 4) % 256) >> 3; //Direction divided by 8
  *xThrust = (thrust * xMultVals[directionLookup]) >> 10;
  *yThrust = (thrust * yMultVals[directionLookup]) >> 10;
}
