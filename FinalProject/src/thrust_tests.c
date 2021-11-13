#include <stdlib.h>
#include "ctest.h"
#include "thrust.h"

CTEST_DATA(thrust) {
    struct craft_thrust_struct *craft_thrust_data;
    int thrustToSet;
};

CTEST_SETUP(thrust) {
}

CTEST2(thrust, max_thrust_test) {
    data->thrustToSet = thrust_max;
    update_thrust_data(data->craft_thrust_data, data->thrustToSet);
    ASSERT_EQUAL(data->thrustToSet, data->craft_thrust_data->current_thrust);
}

CTEST2(thrust, min_thrust_test) {
    data->thrustToSet = thrust_min;
    update_thrust_data(data->craft_thrust_data, data->thrustToSet);
    ASSERT_EQUAL(data->thrustToSet, data->craft_thrust_data->current_thrust);
}

CTEST2(thrust, thrust_none_test) {
    data->thrustToSet = thrust_none;
    update_thrust_data(data->craft_thrust_data, data->thrustToSet);
    ASSERT_EQUAL(data->thrustToSet, data->craft_thrust_data->current_thrust);
}