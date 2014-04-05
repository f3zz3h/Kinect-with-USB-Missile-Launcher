#ifndef TRACKING_H_
#define TRACKING_H_

#include <XnCppWrapper.h>
#include "launcher.h"

typedef struct mapped_posistion {
    float x;
    float y;
    float z;
} xyz;
extern xyz mapped_xyz;
extern bool moving;
extern pthread_mutex_t moving_mutex;

#define X_INDEX 0
#define Y_INDEX 1
#define Z_INDEX 2

#define MAX_X_ML 5.7
#define MID_X_ML 2.85
#define MAX_Y_ML 1.5
#define MID_Y_ML 1

#define KINECT_FOV_MAX 3.451666667
#define KINECT_FOV_MIN 2.248333333

#define MAX_PLAYER_X 720
#define MIN_PLAYER_X -720
#define MAX_PLAYER_Y 480

extern float current_position[2];

void deinit_ml();
int init_ml();
float map_x(float player_x_pos);
void* move_to_position(void*);
void test();
float* positiontOfTarget(XnUserID player);

#endif
