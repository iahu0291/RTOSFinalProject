#include <stdlib.h>
#include "ctest.h"
#include "ledfunc.h"

CTEST_DATA(led_tests) {
    struct led_control_struct *led_ctrl;
    int period, duty_cycle;
};

CTEST_SETUP(led_tests) {
}

CTEST2(led_tests, verify_period_1) {
    data->period = 10;
    data->duty_cycle = 8;
    update_led_control_struct(data->led_ctrl, data->period, data->duty_cycle);
    ASSERT_EQUAL(10, data->led_ctrl->restartPeriod); //Verify correct restart period
}

CTEST2(led_tests, verify_high_duty_cycle) {    
    data->period = 10;
    data->duty_cycle = 8;
    update_led_control_struct(data->led_ctrl, data->period, data->duty_cycle);
    ASSERT_EQUAL(8, data->led_ctrl->timeFromOnToOff); //Verify correct restart period
}

CTEST2(led_tests, verify_period_2) {
    data->period = 20;
    data->duty_cycle = 8;
    update_led_control_struct(data->led_ctrl, data->period, data->duty_cycle);
    ASSERT_EQUAL(20, data->led_ctrl->restartPeriod); //Verify correct restart period
}

CTEST2(led_tests, verify_high_duty_cycle_2) {
    data->period = 20;
    data->duty_cycle = 8;
    update_led_control_struct(data->led_ctrl, data->period, data->duty_cycle);
    ASSERT_EQUAL(16, data->led_ctrl->timeFromOnToOff); //Verify correct restart period
}

CTEST2(led_tests, verify_low_duty_cycle) {
    data->period = 10;
    data->duty_cycle = 2;
    update_led_control_struct(data->led_ctrl, data->period, data->duty_cycle);
    ASSERT_EQUAL(2, data->led_ctrl->timeFromOnToOff); //Verify correct restart period
}

CTEST2(led_tests, verify_low_duty_cycle_2) {
    data->period = 20;
    data->duty_cycle = 2;
    update_led_control_struct(data->led_ctrl, data->period, data->duty_cycle);
    ASSERT_EQUAL(4, data->led_ctrl->timeFromOnToOff); //Verify correct restart period
}
