#include "pid_control.h"

// code implemented based on https://medium.com/@soham.phanse/a-simple-pid-controller-in-c-0570277a989a
// and https://iacsengineering.com/integral-windup/

float pid_controller(float setpoint, float measured, pid_ctrl_t *pid, float dt)
{

    // compute error
    float error = setpoint - measured;

    // compute proportional term
    float proportional = pid->Kp*error;

    // compute integral term
    float integra = ((pid->prev_error+error)/2)*dt;
    float integral = (pid->Ki)*integra;
    pid->integral += integral;

    // compute derivative term
    float deriterm = (error - pid->prev_error)/dt;
    float derivative = pid->Kd*deriterm;
    pid->derivative = derivative;

    // compute total output
    float total_output = proportional +  pid->integral + derivative;

    // source: https://iacsengineering.com/integral-windup/
    if (total_output > pid->output_max) {
        total_output = pid->output_max;
        pid->integral = pid->integral - (pid->Ki * error * dt); // Reverse integration
    } else if (total_output < pid->output_min) {
        total_output = pid->output_min;
        pid->integral = pid->integral - (pid->Ki * error * dt); // Reverse integration
    }

    pid->prev_error = error;

    //return
    return total_output;

}

void pid_init(pid_ctrl_t *pid, float Kp, float Ki, float Kd, float output_min, float output_max)
{
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0.0f;
    pid->derivative = 0.0f;
    pid->output = 0.0f;
    pid->output_min = output_min;
    pid->output_max = output_max;
}