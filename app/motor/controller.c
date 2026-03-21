#include "controller.h"

// void PID_Calc(volatile struct PI_t* pid) {
//     pid->integral += pid->in * pid->dt;

//     if (pid->integral > pid->integralMax) {
//         pid->integral = pid->integralMax;
//     } else if (pid->integral < pid->integralMin) {
//         pid->integral = pid->integralMin;
//     }

//     pid->out = (pid->Kp * pid->in) +
//                (pid->Ki * pid->integral) +
//                (pid->Kd * (pid->in - pid->outPrev) / pid->dt);

//     if (pid->out > pid->outMax) {
//         pid->out = pid->outMax;
//     } else if (pid->out < pid->outMin) {
//         pid->out = pid->outMin;
//     }

//     pid->outPrev = pid->out;
// }

void PID_Calc(volatile struct PI_t* pid) {
	float outBeforeSatur = pid->Kp * (pid->in + pid->integral);

	if (outBeforeSatur > pid->outMax) {
		pid->out = pid->outMax;
	} else if (outBeforeSatur < pid->outMin) {
		pid->out = pid->outMin;
	} else {
		pid->out = outBeforeSatur;
	}

	if (!(pid->out != outBeforeSatur && (pid->in * outBeforeSatur >= 0))) {
		pid->integral += pid->Ki * pid->dt * (pid->in + pid->inPrev) / 2.0f;
	}

	pid->outPrev = pid->out;
}
