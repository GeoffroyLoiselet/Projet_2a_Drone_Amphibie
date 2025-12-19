#include "motor.h"
#include "main.h"



void motor_init(void){
	TIM3->CCR1 = calc_CCR(0);
}

int calc_CCR(int percentage){ //percentage of the power of the motor --> CCR should be between 0.1*MOTOR_ARR and 0.2*MOTOR_ARR
	return MOTOR_ARR*(0.1*percentage/100+0.1);
}


void motor_power(int percentage){
	TIM3->CCR1 = calc_CCR(percentage);

}
