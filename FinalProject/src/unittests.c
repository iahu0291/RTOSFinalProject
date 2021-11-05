#include <stdlib.h>
#include "ctest.h"
#include "priority.h"

CTEST_DATA(priority) {
    struct task_t task[3];
    int size;
};

CTEST_SETUP(priority) {
    int execution[] = {1, 2, 3};
    int priority[] = {1, 2, 3};
    data->size = sizeof(execution) / sizeof(execution[0]);
    init(data->task, execution, priority, data->size);
    priority_schedule(data->task, data->size);
}

CTEST2(priority, test_process) {
    for (int i = 0; i < data->size; i++) {
        ASSERT_EQUAL(i, (int)data->task[i].process_id);
    }
}

CTEST2(priority, avg_wait_time_1) {
    ASSERT_EQUAL((1+2+4)/3, calculate_average_wait_time(data->task, data->size));
}

CTEST2(priority, avg_turnaround_time_1) {
    ASSERT_EQUAL((2+5+6)/3, calculate_average_turn_around_time(data->task, data->size));
}

CTEST_DATA(priority2) {
    struct task_t task[5];
    int size;
};

CTEST_SETUP(priority2) {
    int execution[] = {3, 3, 3, 3, 3};
    int priority[] = {1, 2, 3, 4, 5};
    data->size = sizeof(execution) / sizeof(execution[0]);
    init(data->task, execution, priority, data->size);
    priority_schedule(data->task, data->size);
}

CTEST2(priority2, setup_2) {
    for (int i = 0; i < data->size; i++) {
        ASSERT_EQUAL(i, (int)data->task[i].process_id);
    }
}

CTEST2(priority2, avg_wait_time_2) {
    ASSERT_EQUAL((0+3+6+9+12)/5, calculate_average_wait_time(data->task, data->size));
}

CTEST2(priority2, avg_turnaround_time_2) {
    ASSERT_EQUAL((3+6+9+12+15)/5, calculate_average_turn_around_time(data->task, data->size));
}

CTEST_DATA(priority3) {
    struct task_t task[3];
    int size;
};

CTEST_SETUP(priority3) {
    int execution[] = {6, 8, 5};
    int priority[] = {5, 4, 3};
    data->size = sizeof(execution) / sizeof(execution[0]);
    init(data->task, execution, priority, data->size);
    priority_schedule(data->task, data->size);
}
//I: 0  0  0  0  0  2  0  2  1  1  1  1  1  1  1  1  2  2  2
//P0:5  5  5  10 10 10 40 40 40 40 40 40 40 40 40 40 40 40 40
//P1:4  4  4  4  4  4  4  4  36 36 36 36 36 36 36 36 36 36 36
//P2:3  3  3  3  3  24 24 24 24 24 24 24 24 24 24 24 24 24 24
//R0:5  4  3  2  1  1  0  
//R1:8  8  8  8  8  8  8  8  7  6  5  4  3  2  1  0  
//R2:5  5  5  5  5  4  4  3  3  3  3  3  3  3  3  3  2  1  0
//T: 1  2  3  4  5  6  7  8  9  10 11 12 13 14 15 16 17 18 19
CTEST2(priority3, setup_3) {
    for (int i = 0; i < data->size; i++) {
        ASSERT_EQUAL(i, (int)data->task[i].process_id);
    }
}

CTEST2(priority3, avg_wait_time_3) {
    ASSERT_EQUAL((1 + 8 + 13)/3, calculate_average_wait_time(data->task, data->size));
}

CTEST2(priority3, avg_turnaround_time_3) {
    ASSERT_EQUAL((7+16+19)/3, calculate_average_turn_around_time(data->task, data->size));
}
// TODO add additional tests to help debug
