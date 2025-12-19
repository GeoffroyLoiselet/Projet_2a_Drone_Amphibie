/*
 * motor.h
 *
 *  Created on: Dec 19, 2025
 *      Author: david
 */

#ifndef INC_MOTOR_H_
#define INC_MOTOR_H_

#define MOTOR_TIMER TIM1
#define MOTOR_ARR 799

void motor_init(void);
int calc_CCR(int percentage);
void motor_power(int percentage);

#endif /* INC_MOTOR_H_ */
