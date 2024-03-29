#include <msp430.h>
#include "serial_handler.h"
#include "reels.h"
#include "i2c.h"

/*
 * main.c for Reels Board
 */

int str2num(char *,int );
int input_handler (char *, char *);
void num2str(int ,char *,int );
void all_stop_fun(void);
unsigned char TXData[12],RXData[12];		//Buffers for the Slave of ths device
volatile int TXData_ptr=0,RXData_ptr=0;		//Pointers and flags for the slave device

volatile int cur_reel_depth, reel_dir, set_reel_depth, k ;
volatile char status_code, interrupt_code, avg_pointer;
volatile char pwm_pullup_flag, ALL_STOP_FLAG, reel_flag, autolevel_flag, get_level_flag;
volatile unsigned int timeout_count1, timeout_count2, pwmread = 0, pwmval = 0;
volatile int set_angle, avg_angle[SAMPLES], lvl_compare;
float current_angle, average;

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

	volatile char identify[]="HReel";
	volatile int n, ok2send=0;

	__delay_cycles(50000);
	i2c_slave_init(0x48);  //Set slave address to 0x48
	__delay_cycles(50000);
	i2c_init();
	__delay_cycles(50000);
	uart_init(4);   // set uart baud rate to 9600
	__delay_cycles(50000);
	BCSCTL1 = CALBC1_16MHZ;                    // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;
	for (k=0;k<200;k++)
		__delay_cycles(50000);


	initReel();
	__delay_cycles(50000);
	init_ADXL();
	__delay_cycles(50000);

	for (k=0;k<200;k++)
		__delay_cycles(50000);

	__bis_SR_register(GIE);
	i2cTXData[0]=3;			//Dummy values first input to i2cTXData
	i2cTXData[1]=10;
	i2cTXData[2]=12;
	while(1){
		if (eos_flag){					// if data received from the UART

			eos_flag=0;
			if (rx_data_str[0]=='I'){	// If it is the identifying character return the device ID
				for (n=0;n<9;n++)
					tx_data_str[n]=identify[n];
				uart_write_string(0,9);
			}
			else{
				ok2send=input_handler(rx_data_str,tx_data_str);
				if (ok2send)
					uart_write_string(0,ok2send);

			}
			rx_flag=0;
		}
		if (i2crxflag){					// if data received as the I2C slave
			__delay_cycles(50000);
			ok2send=input_handler(i2cRXData+1,i2cTXData);
			i2crxflag=0;

		}



		if (!ALL_STOP_FLAG){
			if(reel_flag){
				status_code = goToClick(set_reel_depth);
			}
			if(autolevel_flag)
				autoLevel();
		}

		// Level the reel regardless of auto stop
		if(get_level_flag){
			lvl_compare++;
			if(lvl_compare > LOW_PASS){
				getLevel();
				lvl_compare = 0;
			}
			get_level_flag =0;
		}

		if (ALL_STOP_FLAG){
			TA0CCR1 = 0;
			TA0CCR2 = 0;
			TA1CCR1 = 0;
			TA1CCR2 = 0;
			reel_flag = 0;
			reel_dir = 0;
			autolevel_flag = 0;
		}

	}
}

int input_handler (char *instring, char *outstring){
	int retval=0;
	switch (instring[0]){
	case 'R':
		if (instring[1]=='D'){				// Set values for the depth of clicks the reel will go to
			set_reel_depth=str2num(instring+2,3);
			reel_flag=1;
			timeout_count1 = 0;
			timeout_count2 = 0;
			interrupt_code = 0;
			if(!autolevel_flag)
				autolevel_flag = 2;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		break;
	case 'C':
		if (instring[1]=='D'){				// Get clicks
			num2str(cur_reel_depth,outstring,3);
			retval=3;
		}
		break;
	case 'P':
		if (instring[1]=='U'){				// Pull Up Reel
			set_reel_depth=-6;
			reel_flag=1;
			timeout_count1 = 0;
			timeout_count2 = 0;
			interrupt_code = 0;
			if(!autolevel_flag)
				autolevel_flag = 2;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		break;
	case 'L':	// Set Reel Level
		if (instring[1]=='U'){	// Up position
			autolevel_flag = 1;
			set_angle = REELING_ANGLE;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		if (instring[1]=='D'){ // Down position
			autolevel_flag = 1;
			set_angle = -REELING_ANGLE;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		if (instring[1]=='L'){ // Level the reel
			autolevel_flag = 1;
			set_angle = 0;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		if (instring[1]=='T'){ // Travel mode, send the reel all the way down
			autolevel_flag = 1;
			set_angle = -20;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		if (instring[1]=='S'){	// Stop all leveling
			autolevel_flag = 0;
			TA1CCR1=PWM_NEU;
			retval=0;
			ALL_STOP_FLAG=0;
		}
		if (instring[1]=='A'){	// Auto level -> set level based on clicks
			autolevel_flag = 2;
			ALL_STOP_FLAG=0;
			retval = 0;
		}
		if (instring[1]=='Q'){	// Query
			if (instring[2]=='A'){	// Get average angle of reel
				num2str((int)average,outstring,3);
				retval=3;
			}
			if (instring[2]=='S'){	// Get level the reel is set to
				num2str(set_angle,outstring,3);
				retval=3;
			}
		}
		break;
	case 'S':
		ALL_STOP_FLAG=1;
		all_stop_fun();
		retval=0;
		break;
	case 'I':	// Comms check
		outstring[0]= 'O';
		outstring[1]= 'k';
		retval=2;
		break;
	case 'Q':	// Query status
		outstring[0]=(0x30+ALL_STOP_FLAG);
		outstring[1]=(0x30+interrupt_code);
		outstring[2]=(0x30+status_code);
		retval=3;
		break;
	default:	// Invalid command, repeat back the command
		outstring[0]= instring[0];
		outstring[1]= instring[1];
		outstring[2]= instring[2];
		retval=3;
		break;
	}

	return retval;

}

int str2num(char *instring,int n){
	int k,outval=0;
	for (k=0;k<n;k++){
		outval*=10;
		outval+=(instring[k]-0x30);
	}
	return outval;
}

void num2str(int inval,char *outstring,int n){
	int k, neg = 0, divider=1,tval=0,cval;
	if (inval<0){
		inval*=-1;
		neg = 1;
	}
	for (k=0;k<(n);k++){
		divider*=10;
	}
	tval=inval;
	for (k=0;k<n;k++){
		divider/=10;
		cval=tval/divider;
		outstring[k]=cval+0x30;
		tval-=(cval*divider);
	}
	if (neg)
		outstring[0]='-';

}

void all_stop_fun(void){
	// turn off all pwm channels and all motors
	TA0CCR1 = 0;
	TA0CCR2 = 0;
	TA1CCR1 = 0;
	TA1CCR2 = 0;
	reel_flag = 0;
	reel_dir = 0;
	autolevel_flag = 0;

	status_code = 4;
}


