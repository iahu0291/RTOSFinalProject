/*
 * lcdfunc.h
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_LCDFUNC_H_
#define SRC_HEADER_FILES_LCDFUNC_H_

#include "glib.h"
#include "dmd.h"
#include "sl_board_control.h"
#include "physics.h"

#define LCD_XPOS_MASK 0b11111111111111110000000000000000
#define LCD_YPOS_MASK 0b00000000000000001111111111111111

unsigned int translatePosition(struct craft_position_struct *position, struct game_settings_struct *settings);

unsigned int translateDirection(struct craft_direction_struct *direction, struct game_settings_struct *settings);

void displayShipPolygon(GLIB_Context_t* context, struct craft_position_struct *position, struct craft_direction_struct *direction, struct game_settings_struct *settings);

unsigned int move_point_along_angle(int angle, int xStart, int yStart, int distance);

void displayEndgameScreen(GLIB_Context_t* context, char *str);

#endif /* SRC_HEADER_FILES_LCDFUNC_H_ */
