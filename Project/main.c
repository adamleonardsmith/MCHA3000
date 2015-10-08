#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> 
#include <avr/pgmspace.h> 

#include "uart.h"
#include "line_buffer.h"
#include "circ_buffer.h"
#include "encoder.h"
#include "controller.h"
#include "comand_table.h"
#include "motor.h"

#define MAX_CMDS 50

// Globals
static LB_T lb;		// line buffer declaration


int main(void)
{
	
	// INITIALISATIONS
	float x = 0.0;
	float y = 0.0;
	float theta = 0.0;
	float vel = 0.0;
	float velref = 0.0;
	float Mvel = 0.0;
	char* word_array[MAX_CMDS];
	int no_of_words = 0;
	int error = 1;


	DDRC |= 1 << 5; 	// PortC.5 as output 


	lb_init(&lb);		// init line buffer lb 
	uart_init(); 		// init USART
	enc_init();			// init Encoder
	ctrl_init();		// init Controller
	motor_init();		// init Motor
	sei();  			// enable interrupts

	printf_P(PSTR("Sup Bitches\nThis is Command\n"));


	for (;/*ever*/;)
	{
		while (uart_avail())
		{
			char c = uart_getc();		//gets character from circular buffer

			if (lb_append(&lb, c) == LB_BUFFER_FULL)		// Add character "c" to line buffer, report status(putc) and handle special characters(append)
			{
				lb_init(&lb); // Clear line  buffer, discards input
				printf_P(PSTR("\nMax line length exceeded\n"));
			}
		}
		error = 1;

		// Process command if line buffer is terminated by a line feed or carriage return
		if (lb_line_ready(&lb))		//if not empty and has null terminator
		{ 
			for (int j = 0; j < NUM_CMDS; j++)	//re-setting word_array to zero
			{
				word_array[j] = 0;
			}
			
			no_of_words = string_parser( lb_gets(&lb), word_array);		// gets serial, puts into word_array 

			for (int i=0; cmd_table[i].cmd != NULL; ++i)							// 
			{
				if( !strcmp(word_array[0], cmd_table[i].cmd))
		     	{
					error = 0;
                    cmd_table[i].func(no_of_words, word_array);
		       	}	
			}
			lb_init(&lb);

			// Error checking
			if(!no_of_words)
			{
				printf_P(PSTR("No Command Entered\n"));
			}
			if(error)
			{
				printf_P(PSTR("Invalid Command\n"));
			}



		/*	// Note: The following is a terrible way to process strings from the user
			//       See recommendations section of the lab guide for a better way to
			//       handle commands with arguments, which scales well to a large
			//       number of commands.
			if (!strncmp_P(lb_gets(&lb), PSTR("help"), 4))
			{
				printf_P(PSTR(
					"MCHA3000 RS232 lab help.\n"
					"Replace these lines with your own help instructions.\n"));
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("x="), 2))			// takes 'x' co-ordinate
			{
				x = atof(lb_gets_at(&lb, 2));
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("x?"), 2))			// prints 'x' co-ordinate to serial
			{
				printf_P(PSTR("x is %f\n"), x);
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("y="), 2))			// takes 'y' co-ordinate
			{
				y = atof(lb_gets_at(&lb, 2));
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("xy?"), 3))			// prints 'x'*'y' to serial
			{
				printf_P(PSTR("%f\n"), x*y);
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("theta="), 6))		// HIL: takes 'theta'
			{
				theta = atof(lb_gets_at(&lb, 6));
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("vel="), 4))			// HIL: takes 'vel'
			{
				vel = atof(lb_gets_at(&lb, 4));
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("velref="), 7))		// HIL: takes 'velref'
			{
				velref = atof(lb_gets_at(&lb, 7));
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("ctrl?"), 5))		// HIL: initialises feedback loop for cascade controller 
			{															// and prints control action to serial
				float outer_loop = velocity_controller(velref - vel);
				float inner_loop = angle_controller(outer_loop - theta);
				printf_P(PSTR("%g\n"), inner_loop);
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("ecount?"), 7))		// prints enc_count
			{
				printf_P(PSTR("Encoder1 Count =  %d\n"), enc_read1());
				printf_P(PSTR("Encoder2 Count =  %d\n"), enc_read2());
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("ereset"), 6))		// Resets enc_count then prints count
			{
				enc_reset();
				printf_P(PSTR("Encoder1 Count =  %d\n"), enc_read1());
				printf_P(PSTR("Encoder2 Count =  %d\n"), enc_read2());
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("mvel="), 5))		// Motor On/Off
			{
				Mvel=atof(lb_gets_at(&lb, 5));
				motor_vel(Mvel);
				printf_P(PSTR("Motor Velocity = %f\n"), Mvel);
			}
			else if (!strncmp_P(lb_gets(&lb), PSTR("I"), 1))			// Motor Current
			{
				printf_P(PSTR("Motor Current =  %f\n"), motor_current());
			}
			else														// WARNING: Unknown command
			{
				printf_P(PSTR("Unknown command: \"%s\"\n"), lb_gets(&lb));
			}

			lb_init(&lb);	// Reset line buffer 
			*/
		}
	}
	return 0;
}

