#include <stdlib.h>
#include "ctest.h"
#include "direction.h"

CTEST_DATA(direction) {
    struct craft_direction_struct *direction_data;
    int rotThrustToSet;
};

CTEST_SETUP(direction) {
    data->direction_data->rotational_thrust = 0;
    data->rotThrustToSet = hard_cw;
}

CTEST2(direction, hard_cw_test) {
    data->rotThrustToSet = hard_cw;
    update_thrust_data(data->direction_data, data->rotThrustToSet);
}

CTEST2(direction, soft_cw_test) {
    data->rotThrustToSet = soft_cw;
    update_thrust_data(data->direction_data, data->rotThrustToSet);
}

CTEST2(direction, no_rot_test) {
    data->rotThrustToSet = no_rot;
    update_thrust_data(data->direction_data, data->rotThrustToSet);
}

CTEST2(direction, soft_ccw_test) {
    data->rotThrustToSet = soft_ccw;
    update_thrust_data(data->direction_data, data->rotThrustToSet);
}

CTEST2(direction, hard_ccw_test) {
    data->rotThrustToSet = hard_ccw;
    update_thrust_data(data->direction_data, data->rotThrustToSet);
}
