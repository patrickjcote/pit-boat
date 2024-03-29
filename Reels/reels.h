/*
 * reels.h
 *
 *  Created on: June 8, 2016
 *      Author: PCote
 */

#ifndef REELS_REELS_H_
#define REELS_REELS_H_


// Set which board is being programmed
#define DATA_BOARD 		1
#define HOSE_BOARD		0


// ------------------------------------------------------------------------------
// ------------------------------------------------------------------------------
// Data Reel Parameters
#if DATA_BOARD
#define BOARD_NAME 		"DReel"			// Data Reel
#define BOARD_ADDRESS 	0x49			// Data - 0x49
#define CLICKS_PER_WRAP 35				// Data - 35  ---  (Reel width)/(Cable OD)
#define STEPS_PER_CLICK 400  			// Data: 400
#define MOTOR_DOWN 		300				// Default PWM high limit (% * 10)
#define MOTOR_UP 		750				// Default PWM low limit  (% * 10)
#define REEL_TIMEOUT 	8				// Seconds allowable without sensed click
#endif

// Hose Reel Parameters
#if HOSE_BOARD
#define BOARD_NAME 		"HReel"			// Hose Reel
#define BOARD_ADDRESS 	0x48			// Hose - 0x48
#define CLICKS_PER_WRAP 40				// Hose - 40  ---  (Reel width)/(Cable OD)
#define STEPS_PER_CLICK 1295  			// Hose:1295
#define MOTOR_DOWN 		300				// Default PWM high limit (%*10)
#define MOTOR_UP 		500				// Default PWM low limit  (%*10)
#define REEL_TIMEOUT 	8				// Seconds allowable without sensed click
#endif

// Common Parameters
// I/O Pins
#define STEPPER_LIMIT 	BIT0			// P1.0
#define STEPPER_DIR 	BIT3			// P1.3
#define STEPPER_EN 		BIT2			// P2.2
#define STEPPER_STEP 	BIT6			// P1.6
#define BUMP_STOP 		BIT4			// P1.4
#define CLICK_COUNTER 	BIT0			// P2.0
#define MOTOR_SIGNAL 	BIT4			// P2.4


// Constants
#define PWM_NEU  		3000			// PWM Neutral limit
#define REEL_TIMEOUT_1 	50000			// Base for clicks timeout counter
#define MAX_CLICKS 		999				// Upper bound of allowable clicks
#define MIN_CLICKS 		-5				// Lower bound of allowable clicks
#define LIMIT_SWITCH_MIN 5				// Max # of clicks w/o Limit Switch error code
#define FORWARD 		1
#define BACKWARD 		-1
#define TRUE 			1
#define FALSE 			0
#define DOWN 			1
#define UP 				2
#define STOPPED 		0

// Macros
// Set Stepper Direction Forward, 5ms delay
#define _StepDirForward() P1OUT|=STEPPER_DIR;stepper.direction=FORWARD
// Set Stepper Direction Backward, 5ms delay
#define _StepDirBackward() P1OUT&=~STEPPER_DIR;stepper.direction=BACKWARD


// Prototypes
void initReel(void);
int goToClick(void);
void initStepper(void);
void goToStepPosition(int);
int findHome(void);
int input_handler (char *, char *);
int conv_char_hex(char *,int );
void num2str(int ,char *,int );
int str2num(char *,int );
void all_stop_fun(void);


// Structures
struct Reel{
	volatile unsigned int
	timeout1,
	timeout2,
	PWM,
	PWM_Up,
	PWM_Down;

	volatile int
	currentClick,
	currentWrap,
	setClick;

	volatile char
	direction,
	flag;
};

struct Stepper{
	volatile unsigned int
	stepCount;

	volatile signed int
	direction,
	position,
	setPos;

	volatile char
	isEnabled,
	flag;
};

// Externs
extern volatile char statusCode, interruptCode, allStopFlag;
extern struct Reel reel;
extern struct Stepper stepper;


#endif
