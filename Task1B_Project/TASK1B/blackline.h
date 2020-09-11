/*
 * blackline.h
 */


#ifndef BLACKLINE_H_
#define BLACKLINE_H_
#include "blackline.c"

void velocity (unsigned char left_motor, unsigned char right_motor);
void angle_rotate(unsigned int Degrees);
void forward_mm(unsigned int DistanceInMM);
void soft_left (void);
void left_degrees(unsigned int Degrees);
void soft_right (void);
void right_degrees(unsigned int Degrees);
//important
void init_devices (void);
void forward (void);
void left (void);
void right (void);
void precision_left (void);
void precision_right (void);
void stop (void);
void run(void);
#endif
