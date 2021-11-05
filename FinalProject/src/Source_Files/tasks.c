
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
#include "fifo.h"
#include "glib.h"
#include "dmd.h"
#include "sl_board_control.h"

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

static OS_TCB thrust_input_tcb, direction_input_tcb, led0_ctrl_tcb, led1_ctrl_tcb,
              idle_tcb, lcd_display_tcb, physics_tcb, led0_on_tcb, led1_on_tcb;
static CPU_STK  thrust_input_stack[THRUST_INPUT_TASK_STACK_SIZE],
                direction_input_stack[DIRECTION_INPUT_TASK_STACK_SIZE],
                led0_ctrl_stack[LED0_CTRL_TASK_STACK_SIZE],
                led1_ctrl_stack[LED1_CTRL_TASK_STACK_SIZE],
                led0_on_stack[LED0_ON_TASK_STACK_SIZE],
                led1_on_stack[LED1_ON_TASK_STACK_SIZE],
                idle_stack[IDLE_TASK_STACK_SIZE],
                lcd_display_stack[LCD_DISPLAY_TASK_STACK_SIZE],
                physics_stack[PHYSICS_TASK_STACK_SIZE];

static OS_FLAG_GRP monitor_flag_grp, alert_flag_grp;
static OS_Q led_message_queue;
static OS_SEM button_semaphore, slider_semaphore, lcd_semaphore;
static OS_TMR direction_timer, collision_timer, lcd_timer;

static OS_MUTEX speed_mutex;
static OS_MUTEX direction_mutex;

static GLIB_Context_t glib_context;
static int currentLine = 0;

static struct speed_struct speed_data;
static struct direction_struct direction_data;
static struct fifo_t* fifo;
uint8_t speedUpReady, speedDownReady;
int capsenseMeasurement;

enum button_flag_enum{
  button_0_cycle = 1,
  button_1_cycle = 2,
};

enum monitor_flag_enum{
  speed_changed = 1,
  direction_changed = 2,
  collision_timer_up = 4,
  collision_timer_reset = 8,
};

enum led_alert_flag_enum{
  led0on = 1,
  led1on = 2,
  led0off = 4,
  led1off = 8
};
/*******************************************************************************
 *********************   LOCAL FUNCTION PROTOTYPES   ***************************
 ******************************************************************************/

static void thrust_input_task(void *arg);
static void direction_input_task(void *arg);
static void direction_timer_callback(OS_TMR *p_tmr, void *p_arg);
static void collision_timer_callback(OS_TMR *p_tmr, void *p_arg);
static void led0_ctrl_task(void *arg);
static void led1_ctrl_task(void *arg);
static void led0_on_task(void *arg);
static void led1_on_task(void *arg);
static void led0_off_cb(OS_TMR *p_tmr, void *arg);
static void led1_off_cb(OS_TMR *p_tmr, void *arg);
static void idle_task(void *arg);
static void physics_task(void *arg);
static void lcd_display_task(void *arg);

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
  // Create monitor flag group
  OSFlagCreate(&monitor_flag_grp,
               "Monitor Flag Group",
               0,
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));


  // Create alert flag group
  OSFlagCreate(&alert_flag_grp,
               "Alert Flag Group",
               0,
               &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create LED output message queue
  OSQCreate(&led_message_queue,
            "LED Message Queue",
            2,
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
              "LCD Timer Semaphore",
              0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));


  // Create slider Timer
  OSTmrCreate(&direction_timer,
              "Direction Task Timer",
              1,
              1,
              OS_OPT_TMR_PERIODIC,
              (OS_TMR_CALLBACK_PTR) direction_timer_callback,
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
  OSTmrCreate(&collision_timer,
              "Collision Warning Timer",
              50,
              0,
              OS_OPT_TMR_ONE_SHOT,
              (OS_TMR_CALLBACK_PTR) collision_timer_callback,
              (void*)0,
              &err);
  EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

  // Create FIFO for button transitions
  fifo = create_fifo(16);


  //Create speed mutex
  OSMutexCreate(&speed_mutex,
                "Speed data mutex",
                &err);

  //Create direction mutex
  OSMutexCreate(&direction_mutex,
                "Direction data mutex",
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




}

/***************************************************************************//**
 * Thrust input task.
 ******************************************************************************/
static void thrust_input_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;



    EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));

    while (1)
    {

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
    while (1)
    {

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

  while (1)
  {

  }
}



/***************************************************************************//**
 * LED0 control task.
 ******************************************************************************/
static void led0_ctrl_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    while (1)
    {
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}

/***************************************************************************//**
 * LED0 on task.
 ******************************************************************************/
static void led0_on_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    while (1)
    {
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}

/***************************************************************************//**
 * LED0 off function.
 ******************************************************************************/
static void led0_off_cb(OS_TMR *p_tmr, void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    while (1)
    {
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}




/***************************************************************************//**
 * LED1 control task.
 ******************************************************************************/
static void led1_ctrl_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    while (1)
    {
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}

/***************************************************************************//**
 * LED1 on task.
 ******************************************************************************/
static void led1_on_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    while (1)
    {
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}

/***************************************************************************//**
 * LED1 off function.
 ******************************************************************************/
static void led1_off_cb(OS_TMR *p_tmr, void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;
    while (1)
    {
        EFM_ASSERT((RTOS_ERR_CODE_GET(err) == RTOS_ERR_NONE));
    }
}


/***************************************************************************//**
 * LCD output task.
 ******************************************************************************/
static void lcd_display_task(void *arg)
{
    PP_UNUSED_PARAM(arg);
    RTOS_ERR err;

    char displayStr1[10];
    while (1)
    {

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
