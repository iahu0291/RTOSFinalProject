/*
 * structs.h
 *
 *  Created on: Oct 22, 2021
 *      Author: Lena
 */

#ifndef SRC_HEADER_FILES_STRUCTS_H_
#define SRC_HEADER_FILES_STRUCTS_H_

struct speed_struct{
  int currentSpeed;
  int numIncreases;
  int numDecreases;
};

struct direction_struct{
  int currentDirection;
  int numLeftTurns;
  int numRightTurns;
  int lastChangeTimestamp;
};


#endif /* SRC_HEADER_FILES_STRUCTS_H_ */
