
/***************************************************************************//**
 * @file
 * @brief Blink examples functions
 *******************************************************************************
 * # License
 * <b>Copyright 2020 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * The licensor of this software is Silicon Laboratories Inc. Your use of this
 * software is governed by the terms of Silicon Labs Master Software License
 * Agreement (MSLA) available at
 * www.silabs.com/about-us/legal/master-software-license-agreement. This
 * software is distributed to you in Source Code format and is governed by the
 * sections of the MSLA applicable to Source Code.
 *
 ******************************************************************************/


#include <tasks.h>
#include "os.h"
#include "capsense.h"
#include "em_emu.h"
#include "structs.h"
#include "glib.h"
#include "dmd.h"
#include "sl_board_control.h"
#include "thrust.h"
#include "direction.h"
#include "physics.h"
#include "lcdfunc.h"
#include "ledfunc.h"

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static OS_TCB thrust_input_tcb, direction_input_tcb, led0_ctrl_tcb, led1_ctrl_tcb,
              idle_tcb, lcd_display_tcb, physics_tcb, led0_on_tcb, led1_on_tcb, endgame_tcb;
static CPU_STK  thrust_input_stack[THRUST_INPUT_TASK_STACK_SIZE],
                direction_input_stack[DIRECTION_INPUT_TASK_STACK_SIZE],
                led0_ctrl_stack[LED0_CTRL_TASK_STACK_SIZE],
                led1_ctrl_stack[LED1_CTRL_TASK_STACK_SIZE],
                led0_on_stack[LED0_ON_TASK_STACK_SIZE],
                led1_on_stack[LED1_ON_TASK_STACK_SIZE],
                idle_stack[IDLE_TASK_STACK_SIZE],
                lcd_display_stack[LCD_DISPLAY_TASK_STACK_SIZE],
                physics_stack[PHYSICS_TASK_STACK_SIZE],
                endgame_stack[ENDGAME_TASK_STACK_SIZE];

//static OS_Q led_message_queue;
static OS_SEM button_semaphore, slider_semaphore, lcd_semaphore, physics_semaphore;
static OS_SEM led0_on_semaphore, led1_on_semaphore;
static OS_FLAG_GRP endgame_flag_grp;
static OS_TMR direction_timer, physics_timer, blackout_timer;
static OS_TMR led0_on_timer, led0_off_timer;
static OS_TMR led1_on_timer, led1_off_timer;

static OS_MUTEX thrust_mutex, direction_mutex, position_mutex;
static int timerStarted = 0;

static GLIB_Context_t glib_context;
static int currentLine = 0;

static struct craft_thrust_struct thrust_data;
static struct craft_direction_struct direction_data;
static struct craft_position_struct position_data;
static struct led_control_struct led0_data, led1_data;
static struct game_settings_struct settings_data;

enum endgame_flags{
    endgame_flag_crashed = 1,
    endgame_flag_landed = 2
};

int capsenseMeasurement = 0;

/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void thrust_input_task(void *arg);
static void direction_input_task(void *arg);
static void direction_timer_callback(OS_TMR *p_tmr, void *p_arg);
static void led0_ctrl_task(void *arg);
static void led1_ctrl_task(void *arg);
static void led0_on_task(void *arg);
static void led1_on_task(void *arg);
static void led0_on_cb(OS_TMR *p_tmr, void *arg);
static void led1_on_cb(OS_TMR *p_tmr, void *arg);
static void led0_off_cb(OS_TMR *p_tmr, void *arg);
static void led1_off_cb(OS_TMR *p_tmr, void *arg);
static void idle_task(void *arg);
static void physics_task(void *arg);
static void physics_timer_callback(OS_TMR *p_tmr, void *p_arg);
static void blackout_timer_callback(OS_TMR *p_tmr, void *p_arg);
static void lcd_display_task(void *arg);
static void endgame_task(void *arg);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 * Initialize blink example.
 ******************************************************************************/
void tasks_init(void)
{
  //LCD Initialization
  uint32_t status;

  status = sl_board_enable_display();
  EFM_ASSERT(status == SL_STATUS_OK);

  status = DMD_init(0);
  EFM_ASSERT(status == DMD_OK);

  status = GLIB_contextInit(&glib_context);
  EFM_ASSERT(status == GLIB_OK);
  glib_context.backgroundColor = White;
  glib_context.foregroundColor = Black;
  GLIB_clear(&glib_context);
  GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_FontNarrow6x8);

  GLIB_drawStringOnLine(&glib_context,
                        "Initializing",
                        currentLine++,
                        GLIB_ALIGN_LEFT,
                        5,
                        5,
                        true);
  DMD_updateDisplay();

  RTOS_ERR err;

  // Define game settings
  settings_data.blackoutAccel = 12; // m/s^2
  settings_data.blackoutDuration = 30; //20ths of a second
  settings_data.gravity = 2; // m/s^2
  settings_data.vehicleMass = 2048; //kg
  settings_data.maxThrust = 32768; //N, will accelerate the ship at 8m/s^2 when full of fuel; alt 131072, for 32m/s^2
  settings_data.initialFuelMass = 2048; // kg
  settings_data.conversionEfficiency = 4096; // N/kg
  settings_data.optionSelected = slider_rotational_rate__buttons_burst;
  settings_data.xMin = 0;
  settings_data.xMax = 1024;
  settings_data.yMin = 0;
  settings_data.yMax = 1024;
  settings_data.maxHLandingSpeed = 16;
  settings_data.maxVLandingSpeed = 32;
  settings_data.initXVel = 0;
  settings_data.initYVel = -8;
  settings_data.initHPos = 512;
  settings_data.rotationSpeedQuantaMin = 1; //Begrees/second
  settings_data.rotationSpeedQuantaMax = 4; //Begrees/second
  // Note: A "Begree" is a unit of angle measurement representing 1/256th of 360 degrees, because it makes a couple divisions faster and I'm in a silly goofy mood



  //Initialize Data constructs
  //Thrust data
  thrust_data.blacked_out = 0;
  thrust_data.current_fuel = settings_data.initialFuelMass;
  thrust_data.current_thrust = thrust_none;

  //Direction data
  direction_data.current_direction = 64; //Directly up
  direction_data.current_rotation_rate = 0;
  direction_data.rotational_thrust = no_rot;

  //Position data
  position_data.current_x_position = settings_data.xMax / 2;
  position_data.current_y_position = settings_data.yMax / 2;
  position_data.current_x_vel = 0;
  position_data.current_y_vel = 0;

  //LED data
  led0_data.restartPeriod = 10;
  led0_data.timeFromOnToOff = 10;
  led1_data.restartPeriod = 10;
  led1_data.timeFromOnToOff = 10;


  // Create LED output message queue
//  OSQCreate(&led_message_queue,
//            "LED Message Queue",
//            2,
//            &err);
//  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  OSFlagCreate(&endgame_flag_grp,
               "Endgame Flag Group",
               0,
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create button semaphore
  OSSemCreate(&button_semaphore,
              "Button Event Semaphore",
              0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create slider semaphore
  OSSemCreate(&slider_semaphore,
              "Capsense Timer Semaphore",
              0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create lcd semaphore
  OSSemCreate(&lcd_semaphore,
              "LCD Update Semaphore",
              0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create led0 on semaphore
  OSSemCreate(&led0_on_semaphore,
              "LED0 On Semaphore",
              0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
  // Create led1 on semaphore
  OSSemCreate(&led1_on_semaphore,
              "LED1 On Semaphore",
              0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create slider semaphore
    OSSemCreate(&physics_semaphore,
                "Physics Timer Semaphore",
                0,
                &err);
    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create slider Timer
  OSTmrCreate(&direction_timer,
              "Direction Task Timer",
              1,
              2,
              OS_OPT_TMR_PERIODIC,
              (OS_TMR_CALLBACK_PTR) direction_timer_callback,
              (void*)0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  OSTmrCreate(&physics_timer,
                "Physics Timer",
                1,
                1,
                OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR) physics_timer_callback,
                (void*)0,
                &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  OSTmrCreate(&blackout_timer,
                "Blackout Timer",
                settings_data.blackoutDuration,
                0,
                OS_OPT_TMR_ONE_SHOT,
                (OS_TMR_CALLBACK_PTR) blackout_timer_callback,
                (void*)0,
                &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LCD timer
//  OSTmrCreate(&lcd_timer,
//              "LCD Display Timer",
//              1,
//              1,
//              OS_OPT_TMR_PERIODIC,
//              (OS_TMR_CALLBACK_PTR) lcd_timer_callback,
//              (void*)0,
//              &err);
//  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create collision detection timer
//  OSTmrCreate(&collision_timer,
//              "Collision Warning Timer",
//              50,
//              0,
//              OS_OPT_TMR_ONE_SHOT,
//              (OS_TMR_CALLBACK_PTR) collision_timer_callback,
//              (void*)0,
//              &err);
//  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));


  //Create speed mutex
  OSMutexCreate(&thrust_mutex,
                "Thrust data mutex",
                &err);

  //Create direction mutex
  OSMutexCreate(&direction_mutex,
                "Direction data mutex",
                &err);

  OSMutexCreate(&position_mutex,
                "Position data mutex",
                &err);

  // Create thrust input Task
  OSTaskCreate(&thrust_input_tcb,
               "Thrust input task",
               thrust_input_task,
               DEF_NULL,
               THRUST_INPUT_TASK_PRIO,
               &thrust_input_stack[0],
               (THRUST_INPUT_TASK_STACK_SIZE / 10u),
               THRUST_INPUT_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create direction input task
  OSTaskCreate(&direction_input_tcb,
               "direction input task",
               direction_input_task,
               DEF_NULL,
               DIRECTION_INPUT_TASK_PRIO,
               &direction_input_stack[0],
               (DIRECTION_INPUT_TASK_STACK_SIZE / 10u),
               DIRECTION_INPUT_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LED0 ctrl task
  OSTaskCreate(&led0_ctrl_tcb,
               "LED0 ctrl task",
               led0_ctrl_task,
               DEF_NULL,
               LED0_CTRL_TASK_PRIO,
               &led0_ctrl_stack[0],
               (LED0_CTRL_TASK_STACK_SIZE / 10u),
               LED0_CTRL_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LED1 ctrl task
  OSTaskCreate(&led1_ctrl_tcb,
               "LED1 ctrl task",
               led1_ctrl_task,
               DEF_NULL,
               LED1_CTRL_TASK_PRIO,
               &led1_ctrl_stack[0],
               (LED1_CTRL_TASK_STACK_SIZE / 10u),
               LED1_CTRL_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LED0 on task
  OSTaskCreate(&led0_on_tcb,
               "LED0 on task",
               led0_on_task,
               DEF_NULL,
               LED0_ON_TASK_PRIO,
               &led0_on_stack[0],
               (LED0_ON_TASK_STACK_SIZE / 10u),
               LED0_ON_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LED1 ctrl task
  OSTaskCreate(&led1_on_tcb,
               "LED1 on task",
               led1_on_task,
               DEF_NULL,
               LED1_ON_TASK_PRIO,
               &led1_on_stack[0],
               (LED1_ON_TASK_STACK_SIZE / 10u),
               LED1_ON_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create idle task
  OSTaskCreate(&idle_tcb,
               "idle task",
               idle_task,
               DEF_NULL,
               IDLE_TASK_PRIO,
               &idle_stack[0],
               (IDLE_TASK_STACK_SIZE / 10u),
               IDLE_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LCD display task
  OSTaskCreate(&lcd_display_tcb,
               "LCD display task",
               lcd_display_task,
               DEF_NULL,
               LCD_DISPLAY_TASK_PRIO,
               &lcd_display_stack[0],
               (LCD_DISPLAY_TASK_STACK_SIZE / 10u),
               LCD_DISPLAY_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create physics task
  OSTaskCreate(&physics_tcb,
               "Physics task",
               physics_task,
               DEF_NULL,
               PHYSICS_TASK_PRIO,
               &physics_stack[0],
               (PHYSICS_TASK_STACK_SIZE / 10u),
               PHYSICS_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create endgame task
  OSTaskCreate(&endgame_tcb,
               "Endgame task",
               endgame_task,
               DEF_NULL,
               ENDGAME_TASK_PRIO,
               &endgame_stack[0],
               (ENDGAME_TASK_STACK_SIZE / 10u),
               ENDGAME_TASK_STACK_SIZE,
               0u,
               0u,
               DEF_NULL,
               (OS_OPT_TASK_STK_CLR),
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));




}

/***************************************************************************//**
 * Thrust input task.
 ******************************************************************************/
static void thrust_input_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;



//    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

    while (1)
    {
//        OSSemPend(); //Pend waiting for button input change semaphore
//        OSMutexPend(); //Pend waiting for thrust struct mutex access
        OSSemPend(&button_semaphore,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  (CPU_TS*) 0,
                  &err);
        OSMutexPend(&thrust_mutex,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    (CPU_TS*)0,
                    &err);
        if(!GPIO_PinInGet(PSH1_port, PSH1_pin)){ //Button 1 pressed, max thrust
            update_thrust_data(&thrust_data, thrust_max);
        }
        else if(!GPIO_PinInGet(PSH0_port, PSH0_pin)){ // Button 0 pressed, min thrust
            update_thrust_data(&thrust_data, thrust_min);
        }
        else{
            update_thrust_data(&thrust_data, thrust_none);
        }
//        OSMutexPost(); //Release thrust struct mutex lock
        OSMutexPost(&thrust_mutex,
                    OS_OPT_POST_NONE,
                    &err);

    }
}

/***************************************************************************//**
 * Capsense input task.
 ******************************************************************************/
static void direction_input_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;

    OSTmrStart(&direction_timer,&err);
    CAPSENSE_Init();

    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    int prevCapsenseMeas = 0;
    while (1)
    {
//        OSSemPend(); // Pend on direction timer callback semaphore
        OSSemPend(&slider_semaphore,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  (CPU_TS*)0,
                  &err);
        capsense_poll();
        if(prevCapsenseMeas != capsenseMeasurement){ // We need to update directiondata
//            OSMutexPend(); // Wait for direction data mutex lock
            OSMutexPend(&direction_mutex,
                        0,
                        OS_OPT_PEND_BLOCKING,
                        (CPU_TS*)0,
                        &err);
            if(capsenseMeasurement == -2){
                update_direction_thrust_data(&direction_data, hard_ccw);
            }
            else if(capsenseMeasurement == -1){
                update_direction_thrust_data(&direction_data, soft_ccw);
            }
            else if(capsenseMeasurement == 0){
                update_direction_thrust_data(&direction_data, no_rot);
            }
            else if(capsenseMeasurement == 1){
                update_direction_thrust_data(&direction_data, soft_cw);
            }
            else {
                update_direction_thrust_data(&direction_data, hard_cw);
            }
//            OSMutexPost(); // Release direction data mutex lock
            OSMutexPost(&direction_mutex,
                        OS_OPT_POST_NONE,
                        &err);
        }
        prevCapsenseMeas = capsenseMeasurement;
    }
}


/*****************************************************************************
 * @brief
 *   Capsense polling function
 *
 * @details
 *   Polls the capacitative sensor and if only the left or the right are pressed,
 *   sets the correct value of capsenseLeft or capsenseRight.
 *
 ****************************************************************************/
void capsense_poll(void){
  CAPSENSE_Sense();
  uint8_t farLeft = CAPSENSE_getPressed(0);
  uint8_t left = CAPSENSE_getPressed(1);
  uint8_t right = CAPSENSE_getPressed(2);
  uint8_t farRight = CAPSENSE_getPressed(3);
  capsenseMeasurement = 0;
  if((left || farLeft) && (right || farRight)){
      capsenseMeasurement = 0;
  }
  else if (left){
      capsenseMeasurement = -1;
  }
  else if (right){
      capsenseMeasurement = 1;
  }
  else if (farLeft){
      capsenseMeasurement = -2;
  }
  else if (farRight){
      capsenseMeasurement = 2;
  }
}

/*************************************************
 * Physics task.
 *************************************************/
static void physics_task(void *arg)
{
  PP_UNUSED_PARAM(arg);
  RTOS_ERR err;
  OSTmrStart(&physics_timer,&err);
  unsigned int prevledctrls = 0;
  while (1)
  {
    // Pend on physics timer
    OSSemPend(&physics_semaphore,
              0,
              OS_OPT_PEND_BLOCKING,
              (CPU_TS*)0,
              &err);

    // Pend on thrust, direction mutexes
    OSMutexPend(&thrust_mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                (CPU_TS*)0,
                &err);
    OSMutexPend(&direction_mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                (CPU_TS*)0,
                &err);

    // Burn Fuel if not blacked out
    if(!thrust_data.blacked_out){
      tick_burn_fuel(&thrust_data, &direction_data, &settings_data);
    }

    // Pend on position mutex
    OSMutexPend(&position_mutex,
                0,
                OS_OPT_PEND_BLOCKING,
                (CPU_TS*)0,
                &err);

    tick_update_position(&thrust_data, &direction_data, &position_data, &settings_data);
    if(thrust_data.blacked_out && (!timerStarted)){
      OSTmrStart(&blackout_timer, &err);
      timerStarted = 1;
    }
    // Post to lcd semaphore
    OSSemPost(&lcd_semaphore,
              OS_OPT_POST_ALL + OS_OPT_POST_NO_SCHED,
              &err);

    unsigned int ledCtrls = get_led_ctrls(&thrust_data,&settings_data);
    if(ledCtrls != prevledctrls){
        input_led_ctrl_data(&led0_data, ledCtrls);
        input_led_ctrl_data(&led1_data, ledCtrls >> 16);
    }

    switch(check_landing(&position_data, &settings_data)){
      case game_ship_crashed:
        OSFlagPost(&endgame_flag_grp,
                   endgame_flag_crashed,
                   OS_OPT_POST_FLAG_SET +
                   OS_OPT_POST_NO_SCHED,
                   &err);
        OSTmrStop(&physics_timer,
                  OS_OPT_TMR_NONE,
                  (void*)0,
                  &err);
        break;
      case game_ship_landed:
        OSFlagPost(&endgame_flag_grp,
                   endgame_flag_landed,
                   OS_OPT_POST_FLAG_SET +
                   OS_OPT_POST_NO_SCHED,
                   &err);
        OSTmrStop(&physics_timer,
                  OS_OPT_TMR_NONE,
                  (void*)0,
                  &err);
        break;
      default:
        break;
    }
    // Release hold on position, direction, thrust mutexes
    OSMutexPost(&position_mutex,
                OS_OPT_POST_NO_SCHED,
                &err);
    OSMutexPost(&direction_mutex,
                OS_OPT_POST_NO_SCHED,
                &err);
    OSMutexPost(&thrust_mutex,
                OS_OPT_POST_NONE,
                &err);
  }
}

/*************************************************
 * Endgame task.
 *************************************************/
static void endgame_task(void *arg)
{
  PP_UNUSED_PARAM(arg);
  RTOS_ERR err;
  while (1)
  {
    int flag = OSFlagPend(&endgame_flag_grp,
                          endgame_flag_crashed + endgame_flag_landed,
                          0,
                          OS_OPT_PEND_FLAG_SET_ANY +
                          OS_OPT_PEND_FLAG_CONSUME +
                          OS_OPT_PEND_BLOCKING,
                          (CPU_TS*)0,
                          &err);
//    OSSemSet(&lcd_semaphore,
//             0,
//             &err); //We don't want to update the LCD anymore
    update_led_control_struct(&led1_data, 20, 16);
    update_led_control_struct(&led0_data, 20, 0);
    switch(flag){
      case endgame_flag_crashed:
          displayEndgameScreen(&glib_context, "Crashed!");
        break;
      case endgame_flag_landed:
          displayEndgameScreen(&glib_context, "Landed!");
        break;
    }
  }
}


/***************************************************************************//**
 * LED0 control task.
 ******************************************************************************/
static void led0_ctrl_task(void *arg)
{
  //TODO
  PP_UNUSED_PARAM(arg);
  RTOS_ERR err;
//  while (1)
//  {
//      EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
//  }
}

/***************************************************************************//**
 * LED0 on task.
 ******************************************************************************/
static void led0_on_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;

    OSTmrCreate(&led0_on_timer,
                "LED0 On Timer",
                0,
                led0_data.restartPeriod,
                OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR) led0_on_cb,
                (void*)0,
                &err);
    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

    OSTmrCreate(&led0_off_timer,
                "LED0 Off Timer",
                led0_data.timeFromOnToOff,
                0,
                OS_OPT_TMR_ONE_SHOT,
                (OS_TMR_CALLBACK_PTR) led0_off_cb,
                (void*)0,
                &err);
    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

    OSTmrStart(&led0_on_timer,
               &err);

    while (1)
    {
        // Pend on LED0 On timer semaphore
        OSSemPend(&led0_on_semaphore,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  (CPU_TS*)0,
                  &err);
        OSTmrSet(&led0_on_timer,
                 0,
                 led0_data.restartPeriod,
                 (OS_TMR_CALLBACK_PTR) led0_on_cb,
                 (void*)0,
                 &err);
        if(led0_data.timeFromOnToOff){
            GPIO_PinOutSet(LED0_port, LED0_pin);
            OSTmrSet(&led0_off_timer,
                     led0_data.timeFromOnToOff,
                     0,
                     (OS_TMR_CALLBACK_PTR) led0_off_cb,
                     (void*)0,
                     &err);
        }
        OSTmrStart(&led0_off_timer, &err);
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}

/***************************************************************************//**
 * LED0 off function.
 ******************************************************************************/
static void led0_off_cb(OS_TMR *p_tmr, void *arg)
{
    PP_UNUSED_PARAM(arg);
    PP_UNUSED_PARAM(p_tmr);
    GPIO_PinOutClear(LED0_port, LED0_pin);
}

/***************************************************************************//**
 * LED0 on function.
 ******************************************************************************/
static void led0_on_cb(OS_TMR *p_tmr, void *arg)
{
    PP_UNUSED_PARAM(arg);
    PP_UNUSED_PARAM(p_tmr);
    RTOS_ERR err;
    OSSemPost(&led0_on_semaphore,
              OS_OPT_POST_ALL,
              &err);
    // Post to LED0 On timer semaphore
}




/***************************************************************************//**
 * LED1 control task.
 ******************************************************************************/
static void led1_ctrl_task(void *arg)
{
  //TODO
  PP_UNUSED_PARAM(arg);
  RTOS_ERR err;
//  while (1)
//  {
//      EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
//  }
}

/***************************************************************************//**
 * LED1 on task.
 ******************************************************************************/
static void led1_on_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    OSTmrCreate(&led1_on_timer,
                "LED1 On Timer",
                0,
                led1_data.restartPeriod,
                OS_OPT_TMR_PERIODIC,
                (OS_TMR_CALLBACK_PTR) led1_on_cb,
                (void*)0,
                &err);
    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

    OSTmrCreate(&led1_off_timer,
                "LED1 Off Timer",
                led1_data.timeFromOnToOff,
                0,
                OS_OPT_TMR_ONE_SHOT,
                (OS_TMR_CALLBACK_PTR) led1_off_cb,
                (void*)0,
                &err);
    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

    OSTmrStart(&led1_on_timer,
               &err);

    while (1)
    {
        // Pend on LED1 On timer semaphore
        OSSemPend(&led1_on_semaphore,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  (CPU_TS*)0,
                  &err);
        OSTmrSet(&led1_on_timer,
                 0,
                 led1_data.restartPeriod,
                 (OS_TMR_CALLBACK_PTR) led1_on_cb,
                 (void*)0,
                 &err);
        if(led1_data.timeFromOnToOff){
            GPIO_PinOutSet(LED1_port, LED1_pin);
            OSTmrSet(&led1_off_timer,
                     led1_data.timeFromOnToOff,
                     0,
                     (OS_TMR_CALLBACK_PTR) led1_off_cb,
                     (void*)0,
                     &err);
        }
        OSTmrStart(&led1_off_timer,
                   &err);
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}

/***************************************************************************//**
 * LED1 off function.
 ******************************************************************************/
static void led1_off_cb(OS_TMR *p_tmr, void *arg)
{
    PP_UNUSED_PARAM(arg);
    PP_UNUSED_PARAM(p_tmr);
    GPIO_PinOutClear(LED1_port, LED1_pin);
}

/***************************************************************************//**
 * LED1 on function.
 ******************************************************************************/
static void led1_on_cb(OS_TMR *p_tmr, void *arg)
{
    PP_UNUSED_PARAM(arg);
    PP_UNUSED_PARAM(p_tmr);
    RTOS_ERR err;
    OSSemPost(&led1_on_semaphore,
              OS_OPT_POST_ALL,
              &err);
    // Post to LED1 On timer semaphore
}


/***************************************************************************//**
 * LCD output task.
 ******************************************************************************/
static void lcd_display_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;

//    char displayStr1[10];
    while (1)
    {
        //Pend on LCD semaphore
        OSSemPend(&lcd_semaphore,
                  0,
                  OS_OPT_PEND_BLOCKING,
                  (CPU_TS*)0,
                  &err);


        OSMutexPend(&thrust_mutex,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    (CPU_TS*)0,
                    &err);
        OSMutexPend(&direction_mutex,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    (CPU_TS*)0,
                    &err);
        OSMutexPend(&position_mutex,
                    0,
                    OS_OPT_PEND_BLOCKING,
                    (CPU_TS*)0,
                    &err);
        GLIB_clear(&glib_context);
        DMD_DisplayGeometry* geometry;
        DMD_getDisplayGeometry(&geometry);
        displayShipPolygon(&glib_context, &position_data, &direction_data, &thrust_data, &settings_data);
        DMD_updateDisplay();

        OSMutexPost(&position_mutex,
                    OS_OPT_POST_NO_SCHED,
                    &err);
        OSMutexPost(&direction_mutex,
                    OS_OPT_POST_NO_SCHED,
                    &err);
        OSMutexPost(&thrust_mutex,
                    OS_OPT_POST_NO_SCHED,
                    &err);

//        GLIB_clear(&glib_context);
//        GLIB_setFont(&glib_context, (GLIB_Font_t *) &GLIB_FontNumber16x20);
//
//        GLIB_drawStringOnLine(&glib_context,
//                              displayStr1,
//                              currentLine++,
//                              GLIB_ALIGN_CENTER,
//                              0,
//                              5,
//                              true);
//        DMD_DisplayGeometry* geometry;
//        DMD_getDisplayGeometry(&geometry);
//        if(direction == 0){
//            // Draw arrow pointing straight. Line center: 64, 88
//            GLIB_drawLine(&glib_context, 64, 112, 64, 64);
//            GLIB_drawLine(&glib_context, 64, 64, 72, 72);
//            GLIB_drawLine(&glib_context, 64, 64, 56, 72);
//        }
//        else if(direction == 1){
//            // Draw arrow pointing slightly right
//            GLIB_drawLine(&glib_context, 81, 71, 47, 105);
//            GLIB_drawLine(&glib_context, 81, 71, 81, 82);
//            GLIB_drawLine(&glib_context, 81, 71, 70, 71);
//        }
//        else if(direction == 2){
//            // Draw arrow pointing very right
//            GLIB_drawPartialCircle(&glib_context, 64, 88, 24, 15);
//            GLIB_drawLine(&glib_context, 88, 88, 80, 80);
//            GLIB_drawLine(&glib_context, 88, 88, 96, 80);
//        }
//        else if(direction == -1){
//            // Draw arrow pointing slightly left
//            GLIB_drawLine(&glib_context, 47, 71, 81, 105);
//            GLIB_drawLine(&glib_context, 47, 71, 58, 71);
//            GLIB_drawLine(&glib_context, 47, 71, 47, 82);
//        }
//        else if(direction == -2){
//            // Draw arrow pointing very left
//            GLIB_drawPartialCircle(&glib_context, 64, 88, 24, 15);
//            GLIB_drawLine(&glib_context, 40, 88, 32, 80);
//            GLIB_drawLine(&glib_context, 40, 88, 48, 80);
//        }
//        DMD_updateDisplay();




        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }


}


/***************************************************************************//**
 * Idle task.
 ******************************************************************************/
static void idle_task(void *arg)
{
    PP_UNUSED_PARAM(arg);

    RTOS_ERR err;
    while (1)
    {
        EMU_EnterEM1();
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}



/*****************************************************************************
 * @brief
 *   Pushbutton 0 interrupt handling function
 *
 * @details
 *   Calls the pushbutton0 polling function on a pushbutton0 interrupt
 *
 ****************************************************************************/
void GPIO_EVEN_IRQHandler(void){
  RTOS_ERR err;
  GPIO_IntClear(1 << PSH0_pin);
  OSSemPost(&button_semaphore,
            OS_OPT_POST_ALL,
            &err);
//  if((!GPIO_PinInGet(PSH0_port, PSH0_pin)) && (GPIO_PinInGet(PSH1_port, PSH1_pin))){ //button 1 off & button 0 pressed
//      speedUpReady = 1;
//  }
}

/*****************************************************************************
 * @brief
 *   Pushbutton 1 interrupt handling function
 *
 * @details
 *   Calls the pushbutton1 polling function on a pushbutton1 interrupt
 *
 ****************************************************************************/
void GPIO_ODD_IRQHandler(void){
  RTOS_ERR err;
  GPIO_IntClear(1 << PSH1_pin);
  OSSemPost(&button_semaphore,
            OS_OPT_POST_ALL,
            &err);
//  if((!GPIO_PinInGet(PSH1_port, PSH1_pin)) && (GPIO_PinInGet(PSH0_port, PSH0_pin))){ //button 1 pressed & button 0 off
//      speedDownReady = 1;
//  }
}

/*****************************************************************************
 * @brief
 *   Direction timer callback function
 *
 * @details
 *   Posts to the capsense function when it's time to detect the direction again
 *
 ****************************************************************************/
static void direction_timer_callback(OS_TMR *p_tmr, void *p_arg){
  PP_UNUSED_PARAM(p_arg);
  PP_UNUSED_PARAM(p_tmr);
  RTOS_ERR err;
  OSSemPost(&slider_semaphore,
            OS_OPT_POST_ALL,
            &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
}

/*****************************************************************************
 * @brief
 *   Physics timer callback function
 *
 * @details
 *   Posts to the capsense function when it's time to detect the direction again
 *
 ****************************************************************************/
static void physics_timer_callback(OS_TMR *p_tmr, void *p_arg){
  PP_UNUSED_PARAM(p_arg);
  PP_UNUSED_PARAM(p_tmr);
  RTOS_ERR err;
  OSSemPost(&physics_semaphore,
            OS_OPT_POST_ALL,
            &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
}

/*****************************************************************************
 * @brief
 *   Blackout timer callback function
 *
 * @details
 *   Posts to the capsense function when it's time to detect the direction again
 *
 ****************************************************************************/
static void blackout_timer_callback(OS_TMR *p_tmr, void *p_arg){
  PP_UNUSED_PARAM(p_arg);
  PP_UNUSED_PARAM(p_tmr);
  RTOS_ERR err;
  OSMutexPend(&thrust_mutex,
              0,
              OS_OPT_PEND_BLOCKING,
              (CPU_TS*)0,
              &err);
  timerStarted = 0;
  thrust_data.blacked_out = 0;
  OSMutexPost(&thrust_mutex,
              OS_OPT_POST_NONE,
              &err);
//  OSSemPost(&blackout_over_semaphore,
//            OS_OPT_POST_ALL,
//            &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
}
