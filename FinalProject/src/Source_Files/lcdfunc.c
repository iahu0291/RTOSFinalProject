/*
 * lcdfunc.c
 *
 *  Created on: Nov 5, 2021
 *      Author: Lena
 */

#include "lcdfunc.h"

unsigned int translatePosition(struct craft_position_struct *position, struct game_settings_struct *settings){
  unsigned int xPosFinal, yPosFinal;
  xPosFinal = (position->current_x_position * 128) / settings->xMax;
  yPosFinal = ((settings->yMax - position->current_y_position) * 128) / settings->yMax;
  return (xPosFinal << 16) + yPosFinal;
}

unsigned int translateDirection(struct craft_direction_struct *direction, struct game_settings_struct *settings){
  return (( ((direction->current_direction + 4) % 256) >> 3 ) + 16) % 32 ;
}

void displayShipPolygon(GLIB_Context_t* context, struct craft_position_struct *position, struct craft_direction_struct *direction, struct game_settings_struct *settings){
  int polygonPoints[6];
  unsigned int positionOutput = translatePosition(position, settings);
  int xPos = (positionOutput & LCD_XPOS_MASK) >> 16;
  int yPos = positionOutput & LCD_YPOS_MASK;
  int directionVal = translateDirection(direction, settings);
  unsigned int ship_bow_coords = move_point_along_angle(directionVal, xPos, yPos, 24);
  unsigned int ship_port_coords = move_point_along_angle((directionVal + 8) % 32, xPos, yPos, 8);
  unsigned int ship_starboard_coords = move_point_along_angle((directionVal + 24) % 32, xPos, yPos, 8);

  polygonPoints[0] = (ship_bow_coords & LCD_XPOS_MASK) >> 16;
  polygonPoints[1] = (ship_bow_coords & LCD_YPOS_MASK);
  polygonPoints[2] = (ship_port_coords & LCD_XPOS_MASK) >> 16;
  polygonPoints[3] = (ship_port_coords & LCD_YPOS_MASK);
  polygonPoints[4] = (ship_starboard_coords & LCD_XPOS_MASK) >> 16;
  polygonPoints[5] = (ship_starboard_coords & LCD_YPOS_MASK);
  GLIB_drawPolygonFilled(context, 3, &polygonPoints);
}

void displayEndgameScreen(GLIB_Context_t *context, char *str){
  GLIB_drawStringOnLine(context,
                        str,
                        2,
                        GLIB_ALIGN_CENTER,
                        0,
                        0,
                        true);
  DMD_updateDisplay();
}

unsigned int move_point_along_angle(int angle, int xStart, int yStart, int distance){
  int xEnd = xStart;
  int yEnd = yStart;
  xEnd += (distance * xMultVals[angle]) >> 10;
  yEnd += (distance * yMultVals[angle]) >> 10;
  unsigned int returnVal = (xEnd << 16) + yEnd;
  return returnVal;
}
