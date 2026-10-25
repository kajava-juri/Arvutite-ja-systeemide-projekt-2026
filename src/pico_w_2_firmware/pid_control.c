#include "pid_control.h"

float pid_controller(float setpoint, float measured, float preverror, pid_t *pid, float dt) {

    // compute error
    float error = setpoint - measured;

    // compute proportional term
    float proportional = pid->Kp*error;

    // compute integral term
    float integra = ((preverror+error)/2)*dt;
    float integral = (pid->Ki)*integra;
    pid->integral = integral;

    // compute derivative term
    float deriterm = (error - preverror)/dt;
    float derivative = pid->Kd*deriterm;
    pid->derivative = derivative;

    // compute total output
    float total_output = proportional +  integral + derivative;

    //return
    return total_output;

}