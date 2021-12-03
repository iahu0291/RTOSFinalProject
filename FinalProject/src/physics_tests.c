#include <stdlib.h>
#include "ctest.h"
#include "physics.h"

CTEST_DATA(physics) {
    struct craft_thrust_struct *thrust_data;
    struct craft_direction_struct *direction_data;
    struct craft_position_struct *position_data;
    struct game_settings_struct *settings;
};

CTEST_SETUP(physics) {
    data->thrust_data->current_thrust = thrust_none;
    data->direction_data->current_direction = 0;
    data->direction_data->current_rotation_rate = 0;
    data->position_data->current_x_position = data->settings->xMax/2;
    data->position_data->current_y_position = data->settings->yMax;
    data->position_data->current_x_vel = 0; 
    data->position_data->current_y_vel = 0;
    data->settings->gravity = 10;
    data->settings->yMin = 0;
    data->settings->yMax = 512;
    data->settings->xMin = 0;
    data->settings->xMax = 512;
}

CTEST2(physics, verify_falling) {
    tick_update_position(data->thrust_data, data->direction_data, data->position_data, data->settings);
    ASSERT_EQUAL(-10, data->position_data->current_y_vel);
    ASSERT_EQUAL(data->settings->yMax-10, data->position_data->current_y_position);
}

CTEST2(physics, verify_no_burn) {
    int before_burn = data->thrust_data->current_fuel;
    tick_burn_fuel(data->thrust_data, data->direction_data, data->settings);
    ASSERT_EQUAL(before_burn, data->thrust_data->current_fuel);
}

CTEST2(physics, verify_falling_2) {
    tick_update_position(data->thrust_data, data->direction_data, data->position_data, data->settings);
    ASSERT_EQUAL(-20, data->position_data->current_y_vel);
    ASSERT_EQUAL(data->settings->yMax-30, data->position_data->current_y_position);
}

CTEST2(physics, verify_falling_3) {
    tick_update_position(data->thrust_data, data->direction_data, data->position_data, data->settings);
    ASSERT_EQUAL(-30, data->position_data->current_y_vel);
    ASSERT_EQUAL(data->settings->yMax-60, data->position_data->current_y_position);
}

CTEST2(physics, verify_min_thrust_no_dir) {
    //Reset Position & Velocity
    data->position_data->current_x_position = data->settings->xMax/2;
    data->position_data->current_y_position = data->settings->yMax/2;
    data->position_data->current_x_vel = 0; 
    data->position_data->current_y_vel = 0;
    data->thrust_data->current_thrust = thrust_min;
    tick_update_position(data->thrust_data, data->direction_data, data->position_data, data->settings);
    int thrust_vel_expected = sqrt(2 * (10 * data->settings->conversionEfficiency) / (data->thrust_data->current_fuel + data->settings->vehicleMass));
    ASSERT_EQUAL(thrust_vel_expected - data->settings->gravity, data->position_data->current_y_vel);
    ASSERT_EQUAL((data->settings->yMax/2) - (thrust_vel_expected - data->settings->gravity), data->position_data->current_y_position);
}

CTEST2(physics, check_flying) {
    int status = check_landing(data->position_data, data->settings);
    ASSERT_EQUAL(0, status);
}


CTEST2(physics, verify_min_burn) {
    int before_burn = data->thrust_data->current_fuel;
    tick_burn_fuel(data->thrust_data, data->direction_data, data->settings);
    ASSERT_EQUAL(before_burn - 10, data->thrust_data->current_fuel);
}

CTEST2(physics, verify_max_thrust_no_dir) {
    //Reset Position & Velocity
    data->position_data->current_x_position = data->settings->xMax/2;
    data->position_data->current_y_position = data->settings->yMax/2;
    data->position_data->current_x_vel = 0; 
    data->position_data->current_y_vel = 0;
    data->thrust_data->current_thrust = thrust_min;
    tick_update_position(data->thrust_data, data->direction_data, data->position_data, data->settings);
    int thrust_vel_expected = sqrt(2 * (20 * data->settings->conversionEfficiency) / (data->thrust_data->current_fuel + data->settings->vehicleMass));
    ASSERT_EQUAL(thrust_vel_expected - data->settings->gravity, data->position_data->current_y_vel);
    ASSERT_EQUAL((data->settings->yMax/2) - (thrust_vel_expected - data->settings->gravity), data->position_data->current_y_position);
}

CTEST2(physics, verify_max_burn) {
    int before_burn = data->thrust_data->current_fuel;
    tick_burn_fuel(data->thrust_data, data->direction_data, data->settings);
    ASSERT_EQUAL(before_burn - 30, data->thrust_data->current_fuel);
}
