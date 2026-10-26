#ifndef PID_CONTROL_H
#define PID_CONTROL_H

/**
 * @file pid_control.h
 * @brief PID control functions
 * source of the implementation: 
 */

// PID struct for storing gains, integrals, and previous error
typedef struct {

    // add more members if needed
    float Kp;     // proportional gain
    float Ki;     // integral gain
    float Kd;     // derivative gain
    float integral; // integral term
    float derivative; // derivative term
    float output;    // controller output
    float output_min; // minimum output limit
    float output_max; // maximum output limit
    float prev_error; // previous error for derivative calculation
} pid_ctrl_t;

float pid_controller(float setpoint, float measured, pid_ctrl_t *pid, float dt);
void pid_init(pid_ctrl_t *pid, float Kp, float Ki, float Kd, float output_min, float output_max);

#endif // PID_CONTROL_H