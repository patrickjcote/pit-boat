#include <msp430.h>
#include "reels.h"



void initReel(){

	//Limit Switch input
	P1DIR &= ~BIT4;				// Limit switch input on 1.4
	P1IE |=  BIT4;				// 1.4 interrupt enabled
	P1IES |= BIT4;				// 1.4 Hi/lo edge
	P1REN |= BIT4;				// Enable Pull Up (P1.4)
	P1IFG &= ~BIT4;				// P1.4 IFG clear
	//Click Count input
	P2DIR &= ~BIT0;				// Click count input on 2.0
	P2IE |=  BIT0;				// P2.0 interrupt enabled
	P2IES |= BIT0;				// P2.0 Hi/lo edge
	P2REN |= BIT0;				// Enable Pull Up (P2.0)
	P2IFG &= ~BIT0;				// P2.0 IFG clear
	//PWM Init
	TA1CCR0 = 40000;
	TA1CCR1 = 0;
	TA1CCR2 = 0;
	TA1CCTL1 = OUTMOD_7;
	TA1CCTL2 = OUTMOD_7;
	TA1CTL = TASSEL_2 + MC_1 + ID_3;
	//PWM Outputs
	P2DIR |= BIT4;				//Motor Control on P2.4
	P2SEL |= BIT4;				//TA1.2 Output on P2.4
	P2DIR |= BIT2;				// P2.2 Actuator PWM
	P2SEL |= BIT2;				//TA1.1 Output to  P2.2
	//Init Globals
	cur_reel_depth = 0;
	set_reel_depth = 0;
	reel_dir = 0;
	reel_flag = 0;
	ALL_STOP_FLAG = 1;
	status_code = 0;
	interrupt_code = 0;

	__bis_SR_register(GIE);

}//init_reel()

int goToClick(int setClick){

	timeout_count1++;
	if(timeout_count1 > REEL_TIMEOUT_1){
		timeout_count1 = 0;
		timeout_count2++;
	}
	if(timeout_count2 > REEL_TIMEOUT_2){
		ALL_STOP_FLAG = 1;
		return 3;
	}

	if(cur_reel_depth != setClick){
		if(cur_reel_depth > setClick){
			reel_dir = 1;
			TA1CCR2 = PWM_MIN;
			setReelLevel(3);
			return reel_dir;
		}
		if(cur_reel_depth < setClick){
			reel_dir = 2;
			TA1CCR2 = PWM_MAX;
			setReelLevel(2);
			return reel_dir;
		}
	}
	else{
		reel_dir = 0;
		reel_flag = 0;
		TA1CCR2 = PWM_NEU;
		setReelLevel(0);
		ALL_STOP_FLAG = 1;
		return 0;
	}

	return 0;
}//goToClick()

int setReelLevel(int set_reel_level){
	if(set_reel_level == 3){
		volatile int currentWrap;
		currentWrap = (cur_reel_depth / TURNS_PER_WRAP)+1;

		if(currentWrap % 2)
			TA1CCR1=PWM_MIN;
		else
			TA1CCR1=PWM_MAX;
		return 1;
	}
	if(set_reel_level == 2){ //Reeling down
		TA1CCR1 = PWM_MIN;
	}
	if(set_reel_level == 0 || ALL_STOP_FLAG == 1)
	{
		TA1CCR1 == 0;
	}

	return 0;

}


// --------------------------------  Interrupts ----------------------
// Port 1 ISR
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
		//Hardware interrupt for limit switch
		if(reel_dir == 1 && reel_flag == 1){
			cur_reel_depth = 0;
			reel_dir = 0;
			reel_flag = 0;
			status_code = 0;
			interrupt_code = 1;  //Limit switch hit
		}
		else{
			reel_flag = 0;
			interrupt_code = 2; // Limit switch error
		}

		ALL_STOP_FLAG = 1;
		P1IFG &= ~BIT4;
}

// Port 2 ISR
#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{

		//Hardware interrupt for click counter
		if(reel_dir == 2)
			cur_reel_depth++;
		if(reel_dir == 1)
			cur_reel_depth--;
		if(cur_reel_depth < MIN_CLICKS || cur_reel_depth > MAX_CLICKS){
			ALL_STOP_FLAG = 1;
			interrupt_code = 3; //Clicks out of bounds
		}

		timeout_count1 = 0;
		timeout_count2 = 0;

		P2IFG &= ~BIT0;

}
