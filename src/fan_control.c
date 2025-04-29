/*
  PWM output is on GPIO0 D0 pin, like signa_generator example.
  ADC is from A0
*/
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include "physical.h"
#include "address_map_arm.h"
#include "PID.h"
#include <math.h>



// === FPGA side ===
#define HW_REGS_BASE ( 0xff200000 )
#define HW_REGS_SPAN ( 0x00200000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )

/* Controller parameters */
#define PID_PARAM_KP  9
#define PID_PARAM_KI  0.025
#define PID_PARAM_KD  20
 
/* address and field definition */
#define PWM_DVSR_REG        0x0
#define PWM_DUTY_REG        0x10
 
// character buffer
volatile unsigned int * vga_char_ptr = NULL ;
void *vga_char_virtual_base;
 
 /* Prototypes for functions used for video display */
void wait_for_vsync(volatile int *);
void get_screen_specs (volatile int *);
void clear_screen(void);
void plot_pixel(int, int, int);
void drawline (int, int, int, int, int);
void VGA_text (int, int, char *);
void VGA_text_clear();

int pixel_buffer_start;					// location of the pixel buffer video memory


int main(void)
{
    
	int fd = -1;			   // used to open /dev/mem for access to physical addresses
	void *LW_virtual;          // used to map physical addresses for the light-weight bridge
	void *SDRAM_virtual;
	volatile int * pixel_ctrl_ptr;	// virtual address of pixel buffer DMA controller
	volatile unsigned int *adc;
	int res = 4;
	volatile bool change = true;
	volatile bool _100 = true;
	volatile bool _75 = true;
	volatile bool _50 = true;
	volatile bool _25 = true;
	volatile bool _0 = true;
	volatile unsigned int ADC_Value;
	
// === FPGA ===
	volatile unsigned int *PWM_addr = NULL;
	
 	/* ARM PID Instance, float_32 format */
	PIDController PID;
	
	// Create virtual memory access to the FPGA light-weight bridge by calling fpga_init() function
	if ((fd = open_physical (fd)) == -1)
		return (-1);
	else if ((LW_virtual = map_physical (fd, LW_BRIDGE_BASE, LW_BRIDGE_SPAN)) == NULL)
		return (-1);
	
	// Create virtual memory access to the SDRAM memory
	SDRAM_virtual = mmap( NULL, SDRAM_SPAN, ( PROT_READ | PROT_WRITE ), MAP_SHARED, fd, SDRAM_BASE);
    if (SDRAM_virtual == NULL)
        return (0);
	
	// === get VGA char addr =====================
	// get virtual addr that maps to physical
	vga_char_virtual_base = mmap( NULL, FPGA_CHAR_SPAN, ( 	PROT_READ | PROT_WRITE ), MAP_SHARED, fd, FPGA_CHAR_BASE );	
	if( vga_char_virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap2() failed...\n" );
		close( fd );
		return(1);
	}
    
    // Get the address that maps to the FPGA LED control 
	vga_char_ptr =(unsigned int *)(vga_char_virtual_base);
	
	PWM_addr = 	(volatile unsigned int *)(LW_virtual + PWM_BASE);
	
	// get virtual address pointer to the pixel buffer DMA controller
	pixel_ctrl_ptr = (unsigned int *) (LW_virtual + PIXEL_BUF_CTRL_BASE);

	/* Initialize a virtual address pointer to the pixel buffer, used by drawing functions. 
	 * Since our program uses the same address for both the front and back buffers, we only 
	 * have to set this pointer to the pixel buffer memory once, and then use it throughout the 
	 * program wherever we need to write to the pixels */
	pixel_buffer_start = (int) SDRAM_virtual;

	clear_screen ( );					// blank the VGA screen

	/* Initialize the location of the back pixel buffer in the pixel buffer controller.
	 * This is the same address that we used for the front pixel buffer, which means that there 
	 * is only one pixel buffer used in this program. We need to store the physical address
	 * of the pixel buffer in the pixel buffer DMA controller, as explained previously */
    *(pixel_ctrl_ptr + 1) = SDRAM_BASE;
	
	/* Set PID parameters */
	/* Set this for your needs */
	PID.Kp = PID_PARAM_KP;		/* Proporcional */
	PID.Ki = PID_PARAM_KI;		/* Integral */
	PID.Kd = PID_PARAM_KD;		/* Derivative */
	
	/* Initialize PID system */
	PIDController_Init(&PID, 1);
	
	
	// get address for adc
	adc = ((unsigned int *)(LW_virtual + ADC_BASE));
	
	/* Duty cycle for PWM */
	volatile int PWM_Duty = 0;
	float temp_c = 0;
    float REFERENCE_TEMP = 25.0;
	float temp_f = 0;
	volatile float duty = 0;
	
	/* create a message to be displayed on the VGA 
          and LCD displays */
	char text_top_row[40] = "DE1-SoC ARM/FPGA\0";
	char text_bottom_row[40] = "Liberty ENGC401\0";
	char num_string[20];
	VGA_text (10, 1, text_top_row);
	VGA_text (10, 2, text_bottom_row);
	
	// turn on auto-update
	*(adc+1) = 0x1;
	
	*(PWM_addr + PWM_DVSR_REG) = 0x2;
	while (1){
		//ADC to temp
		//temp_c = (volt - 482.3) / 7.11;
		//calculate duty
		
		//manual control for testing
		//printf("Enter temp in c: ");
        //scanf("%f", &temp_c);
		
		
		ADC_Value = *(adc) & 0xfff;
		temp_c = (ADC_Value - 482.3) / 7.11;
		duty = PIDController_Update(&PID, REFERENCE_TEMP, temp_c);
		temp_f =(temp_c * 9/5) + 32;
		printf("pid calculated duty = %f%%, temp = %f c, temp = %f f\n",duty, temp_c, temp_f);
        
		
		
		/* Check overflow, duty cycle in percent */
        if (duty >= 87){
			_100 = true;
			if(_75 || _50 || _25 || _0){
				PWM_Duty = 1 * pow(2.0, (float)(res));
				change = true;
				_75 = false; 
				_50 = false;  
				_25 = false; 
				_0 = false;
				*(PWM_addr + PWM_DUTY_REG) = (int)PWM_Duty;
			}
		}
		else if (duty >= 63 && duty <87 ){
			_75 = true;
			if(_100 || _50 || _25 || _0){
				PWM_Duty = 0.75 * pow(2.0, (float)(res));
				change = true;
				_100 = false; 
				_50 = false;  
				_25 = false; 
				_0 = false;
				*(PWM_addr + PWM_DUTY_REG) = (int)PWM_Duty;
			}
		}
		else if (duty >= 37 && duty < 63){
			_50 = true;
			if(_100 || _75 || _25 || _0){
				PWM_Duty = 0.50 * pow(2.0, (float)(res));
				change = true;
				_100 = false; 
				_75 = false; 
				_25 = false; 
				_0 = false;
				*(PWM_addr + PWM_DUTY_REG) = (int)PWM_Duty;
			}
		}
		else if (duty >= 20 && duty < 37){
			_25 = true;
			if(_100 || _75 || _50 || _0){
				PWM_Duty = 0.25 * pow(2.0, (float)(res));
				change = true;
				_100 = false; 
				_75 = false; 
				_50 = false;  
				_0 = false;
				*(PWM_addr + PWM_DUTY_REG) = (int)PWM_Duty;
			}
		}
		else if (duty <20){
			_0 = true;
			if(_100 || _75 || _50 || _25){
				PWM_Duty = 0;
				change = true;
				_100 = false; 
				_75 = false; 
				_50 = false;  
				_25 = false; 
				*(PWM_addr + PWM_DUTY_REG) = (int)PWM_Duty;
			}
		}
		
		/*
		*Draws pwm_out wave form
		*/
		int color = 0x00ff00;
		if (change){
			//100% x = 800 y = high 
			if (_100){
				clear_screen();
				drawline(0,150,800,150,color);
				sprintf(num_string, "Duty Cycle: 100%%");
				PWM_Duty = 100;
			}
			//75% x = 600 y = high after x = 601 y = low
			else if (_75){
				clear_screen();
				drawline(0,150,600,150,color);
				drawline(600,150,601,400,color);
				drawline(601,400,800,400,color);
				sprintf(num_string, "Duty Cycle: 75%%   ");
				PWM_Duty = 75;
			}
			//50% x = 400 y = high after x = 401 y = low
			else if (_50){
				clear_screen();
				drawline(0,150,400,150,color);
				drawline(400,150,401,400,color);
				drawline(401,400,800,400,color);
				sprintf(num_string, "Duty Cycle: 50%%   ");
				PWM_Duty = 50;
			}
			//25% x = 200 y = high after x = 201 y = low
			else if (_25){
				clear_screen();
				drawline(0,150,200,150,color);
				drawline(200,150,201,400,color);
				drawline(201,400,800,400,color);
				sprintf(num_string, "Duty Cycle: 25%%   ");
				PWM_Duty = 25;
			}
			//0% x = 800 y = low
			else if (_0){
				clear_screen();
				drawline(0,400,800,400,color);
				sprintf(num_string, "Duty Cycle: 0%%   ");
				PWM_Duty = 0;
			}
			VGA_text (10, 3, num_string);
			change = false;
		}
		
		printf("duty = %d%%, temp = %f c, temp = %f f\n",PWM_Duty, temp_c, temp_f);
		usleep(2000000);
		system("clear");
		
	}
	// release the physical-memory mapping
	
	// close /dev/mem 
	
	return 0;
}

/* Function to blank the VGA screen */
void clear_screen( )
{
	int i;
    int *pixel_ptr;
    pixel_ptr = (int *) pixel_buffer_start;
 
    for (i = 0; i < 800*480; i++)
        pixel_ptr[i] = 0;
}
 
void plot_pixel(int x, int y, int line_color)
{
    int *pixel_ptr;
    pixel_ptr = (int *) pixel_buffer_start;
 
    pixel_ptr[y*800 + x] = line_color;
}
 
void drawline(int x0, int y0, int x1, int y1, int color){
	
	int dx, dy, err, x, y, sx, sy;

	dx = abs(x1-x0);
	dy = abs(y1-y0);
	sx = x0 < x1 ? 1 : -1;
	sy = y0 < y1 ? 1 : -1;
	x=x0;
	y=y0;

	err=dx-dy;

	while(x != x1 || y != y1){
		plot_pixel(x,y,color);
		int e2 = 2*err;
		if(e2 > -dy){
			err -= dy;	
			x+= sx;
		}
		if(e2 < dx){
			err += dx;	
			y += sy;
		}
	}
	plot_pixel(x1,y1,color);// draw final point
}

/****************************************************************************************
 * Subroutine to send a string of text to the VGA monitor 
****************************************************************************************/
void VGA_text(int x, int y, char * text_ptr)
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset;
	/* assume that the text string fits on one line */
	offset = (y << 7) + x;
	while ( *(text_ptr) )
	{
		// write to the character buffer
		*(character_buffer + offset) = *(text_ptr);	
		++text_ptr;
		++offset;
	}
}

/****************************************************************************************
 * Subroutine to clear text to the VGA monitor 
****************************************************************************************/
void VGA_text_clear()
{
  	volatile char * character_buffer = (char *) vga_char_ptr ;	// VGA character buffer
	int offset, x, y;
	for (x=0; x<70; x++){
		for (y=0; y<40; y++){
	/* assume that the text string fits on one line */
			offset = (y << 7) + x;
			// write to the character buffer
			*(character_buffer + offset) = ' ';		
		}
	}
}