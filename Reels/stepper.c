#include <msp430.h> 
#include "reels.h"

void initStepper(void) {

	// Direction Pin
	P1DIR |= STEPPER_DIR;
	P1OUT |= STEPPER_DIR;

	// Stepper PWM and Enable
	P2DIR |= STEPPER_STEP + STEPPER_EN;
	P2SEL |= STEPPER_STEP;				// TA0.1 Output on 2.6
	P2SEL &= ~BIT7;						// TA0.1 Output on 2.6

	TA0CCR0 = 16000;					// 1kHz - 16000
	TA0CCR1 = 1600;						// 10% duty 1600
	TA0CCTL1 = OUTMOD_7;				// Reset/Set
	TA0CCTL0 = CCIE;					// Interrupt Enable
	TA0CTL = TASSEL_2 + MC_1;			// smclock and count up

	// Stepper Limit Switch
	P1DIR &= ~STEPPER_LIMIT;			// Limit switch input
	P1REN |= STEPPER_LIMIT;				// Enable Resistor
	P1OUT |= STEPPER_LIMIT;				// Pull Up Resistor

	// Init Stepper Struct
	stepper.position = 60;
	stepper.stepCount = 0;
	stepper.setPos = -1000;
	stepper.isEnabled = FALSE;
	stepper.direction = -1;
	stepper.flag = 0;

	findHome();							// Zero position of stepper arm
}


int findHome(void)
{
	volatile unsigned int n;
	_StepDirBackward();

	stepper.isEnabled = TRUE;

	for(n=0;n<80;n++){
		while(P1IN & STEPPER_LIMIT)
		{
			// While stepper limit not hit TODO: Fix findHome() blocking loop
		}
	}//for debounce loop

	stepper.isEnabled = FALSE;
	stepper.position = 0;
	stepper.setPos = 0;
	_StepDirForward();

	return 1;

}//findHome()


// Timer A0 interrupt service routine
// Clock signal for stepper motor
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A00 (void)
{
	// Check to make sure set position isn't out of bounds
	if(stepper.setPos < 0)
		stepper.setPos = 0;
	if(stepper.setPos > CLICKS_PER_WRAP)
		stepper.setPos = CLICKS_PER_WRAP;

	// Set direction of the stepper
	if(stepper.position == stepper.setPos){
		stepper.isEnabled = FALSE;
		stepper.flag = 0;
	}
	if(stepper.position > stepper.setPos){
		P1OUT &= ~STEPPER_DIR;
		stepper.direction=BACKWARD;
	}
	if(stepper.position < stepper.setPos){
		P1OUT |= STEPPER_DIR;
		stepper.direction=FORWARD;
	}

	if(stepper.isEnabled)
		P2OUT |= STEPPER_EN;		// Enable Motor
	else
		P2OUT &= ~STEPPER_EN;		// Disable Motor

	if(stepper.isEnabled){
		stepper.stepCount++;
		if(stepper.stepCount > STEPS_PER_CLICK){
			stepper.position += stepper.direction;	// Inc/Dec stepper position counter
			stepper.stepCount = 0;					// Reset step counter
		}
	}
}
