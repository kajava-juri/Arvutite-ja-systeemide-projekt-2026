#ifndef PID_CONTROL_H
#define PID_CONTROL_H

/**
 * @file pid_control.h
 * @brief PID control functions
 * source of the implementation: 
 */

// system struct for storing system parameters
typedef struct {

    float m; // mass of the car
    float v; // current velocity of the car

} car_t;

// PID struct for storing gains, integrals, and previous error
typedef struct {

    // add more members if needed
    float Kp;     // proportional gain
    float Ki;     // integral gain
    float Kd;     // derivative gain
    float integral; // integral term
    float derivative; // derivative term
    float output;    // controller output
} pid_t;

#endif // PID_CONTROL_H