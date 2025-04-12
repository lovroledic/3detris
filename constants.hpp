#ifndef CONSTANTS_H
#define CONSTANTS_H

enum Axis {
    AXIS_X,
    AXIS_Y,
    AXIS_Z
};
enum Transformation {
    NONE,
    TRANS_FORWARD,
    TRANS_BACKWARD,
    TRANS_LEFT,
    TRANS_RIGHT,
    ROT_CW,
    ROT_CCW
};

#endif