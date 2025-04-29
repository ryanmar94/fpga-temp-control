fan_control: fan_control.c physical.c physical.h PID.h PID.c
	gcc -Wall -o fan_control fan_control.c physical.c PID.c
