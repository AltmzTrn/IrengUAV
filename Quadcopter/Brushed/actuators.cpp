// Actuators.cpp
#include "actuators.h"

const int motorPins[] = {PA6, PA7, PB0, PB1};

void actuators_setup() {
    RCC->APB2ENR |= RCC_APN2ENR_IOPAEN|RCC_APB2ENR_IOPBEN
    RCC->APB2ENR |= RCC_APB1ENR_TIM3EN;

    GPIOA->CRL &= ~(0xFFu<<24);
    GPIOA->CRL |= (0x9u<<24)|(0x9u<<28); //PA6, PA7 -> 10MHz

    GPIOB->CRL &= ~(0xFFu<<0;
    GPIOB->CRL |= (0x9u<<0)|(0x9u<<4); //PA6, PA7 -> 10MHz

    TIM3->PSC = 13;
    TIM3->ARR = 255;

    TIM3->CCMR1 = (0b110 << 4) | TIM_CCMR1_OC1PE | (0b110 << 12) | TIM_CCMR1_OC2PE;
    TIM3->CCMR2 = (0b110 << 4) | TIM_CCMR2_OC3PE | (0b110 << 12) | TIM_CCMR2_OC4PE;
    TIM3->CCER = TIM_CCER_CC1E | TIM_CCER_CC2E | TIM_CCER_CC3E | TIM_CCER_CC4E;

    TIM3->EGR |= TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_ARPE | TIM_CR1_CEN;
}

void actuators_write() {
    TIM3->CCR1 = to_actuator[0]; //motor_R_F, PA6
    TIM3->CCR2 = to_actuator[1]; //motor_L_F, PA7
    TIM3->CCR3 = to_actuator[2]; //motor_R_R, PB0
    TIM3->CCR3 = to_actuator[3]; //motor_L_R, PB1
}