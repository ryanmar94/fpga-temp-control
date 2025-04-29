#ifndef PID_CONTROLLER_H
#define PID_CONTROLLER_H

typedef struct {
    float A0;          /**< The derived gain, A0 = Kp + Ki + Kd . */
    float A1;          /**< The derived gain, A1 = -Kp - 2Kd. */
    float A2;          /**< The derived gain, A2 = Kd . */
	float state[3];    /**< The state array of length 3. */
	/* Controller gains */
	float Kp;
	float Ki;
	float Kd;
} PIDController;

void  PIDController_Init(PIDController *pid, int resetStateFlag);
float PIDController_Update(PIDController *pid, float setpoint, float measurement);

#endif
