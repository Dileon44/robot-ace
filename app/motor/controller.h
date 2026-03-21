#ifndef __CONTROLLER_H
#define __CONTROLLER_H

#define CONTROLLER_DEFAULTS \
	{ in, out, Kp, Controller_Calc, }

// struct PI_t {
//     float in;
//     float out;
//     float outPrev;
//     float outMax;
//     float outMin;
//     float Kp;
//     float Ki;
//     float Kd;
//     float dt;
//     float integral;
//     float integralMax;
//     float integralMin;
//     void (*calc)(volatile struct PI_t*);
// };

struct PI_t {
	float Kp;
	float Ki;
	float Kd;
	float dt;

	float in;
	float inPrev;
	float out;
	float outPrev;
	float outMax;
	float outMin;

	float integral;

	void (*calc)(volatile struct PI_t*);
};

void PID_Calc(volatile struct PI_t*);

#endif /* __CONTROLLER_H */