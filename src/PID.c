#include "PID.h"
#include <string.h>

void PIDController_Init(PIDController *pid, int resetStateFlag) {
  /* Derived coefficient A0 */
  pid->A0 = pid->Kp + pid->Ki + pid->Kd;

  /* Derived coefficient A1 */
  pid->A1 = (-pid->Kp) - ((float) 2.0 * pid->Kd);

  /* Derived coefficient A2 */
  pid->A2 = pid->Kd;
  
  /* Check whether state needs reset or not */
  if(resetStateFlag)
  {
    /* Clear the state buffer.  The size will be always 3 samples */
    memset(pid->state, 0, 3u * sizeof(float));
  }
}

float PIDController_Update(PIDController *pid, float setpoint, float measurement) {

	/*
	* Error signal
	*/
    float error = measurement - setpoint;
    float out;

    /* y[n] = y[n-1] + A0 * x[n] + A1 * x[n-1] + A2 * x[n-2]  */
    out = (pid->A0 * error) +
      (pid->A1 * pid->state[0]) + (pid->A2 * pid->state[1]) + (pid->state[2]);
 

    /* Update state */
    pid->state[1] = pid->state[0];
    pid->state[0] = error;
    pid->state[2] = out;

    /* return to application */
    return (out);  
}
