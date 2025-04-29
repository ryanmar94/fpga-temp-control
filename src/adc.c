#include "address_map_arm.h"
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/mman.h>
#include "physical.h"
#include <time.h>

#define ADC_BASE 0x00004000
#define GPIO0_BASE 0x00000060
#define TIMER0_BASE 0x00002000
#define TIMER1_BASE 0x00002020

// === FPGA side ===
#define HW_REGS_SPAN ( 0x00200000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

int main(void) {
	
	int fd = -1;			   // used to open /dev/mem for access to physical addresses
	void *LW_virtual;          // used to map physical addresses for the light-weight bridge
	volatile unsigned int *adc=NULL;
	volatile unsigned int *GPIO0_ptr=NULL;
	unsigned int ADC_Value;
	struct timespec ts;
	float voltage, temp;
	
	// Create virtual memory access to the FPGA light-weight bridge by calling fpga_init() function
	if ((fd = open_physical (fd)) == -1)
		return (-1);
	else if ((LW_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
		return (-1);
	
	//Addressing
	adc=(volatile unsigned int *)(LW_virtual + ADC_BASE);
	GPIO0_ptr = (volatile unsigned int *) (LW_virtual + GPIO0_BASE);
	// Set gpio_0 to output
	*(GPIO0_ptr + 1) =  0xFFFF;
	
	printf("Start...\n");
	while (1){
		*(adc + 1) = 0x1;
		ADC_Value = *(adc) & 0xfff;
		temp = (ADC_Value - 482.3) / 7.11;
		voltage = (float)ADC_Value / 819.0;
		if (temp > 25){
			*GPIO0_ptr = 0 << 1;
		}
		else {
			*GPIO0_ptr = 1 << 1;
		}
		system ("clear");
		printf("%.2f volts  %d %f C \n", voltage, ADC_Value, temp);
		usleep(500000);
	}
}