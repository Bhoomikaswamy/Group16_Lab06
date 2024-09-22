#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

int duty;

void GPIO_PORTF_setup(void);
void PWM_setup(void);
void GPIOPortF_Handler(void);

volatile int i = 0;

void GPIO_PORTF_setup(void) {
    SYSCTL_RCGCGPIO_R |= (1 << 5); // Enable clock for PORTF
    GPIO_PORTF_LOCK_R = 0x4C4F434B; // Unlock PORTF
    GPIO_PORTF_CR_R = 0x1F; // Allow changes to PF0-PF4
    GPIO_PORTF_PUR_R = 0x11; // Pull-up resistors for PF0, PF4
    GPIO_PORTF_DEN_R = 0x1F; // Digital enable PF0-PF4
    GPIO_PORTF_DIR_R = 0x0E; // PF1, PF2, PF3 as outputs

    // Configure interrupts for switches
    GPIO_PORTF_IM_R &= ~0x11; // Disable interrupts on PF0 and PF4
    GPIO_PORTF_IS_R &= ~0x11; // Edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x11; // Single edge
    GPIO_PORTF_IEV_R &= ~0x11; // Falling edge
    GPIO_PORTF_ICR_R |= 0x11; // Clear interrupt flags
    GPIO_PORTF_IM_R |= 0x11; // Unmask interrupts

    NVIC_PRI7_R &= 0xFF3FFFFF; // Set priority
    NVIC_EN0_R |= (1 << 30); // Enable interrupt for PORTF
}

void PWM_setup(void) {
    SYSCTL_RCGCPWM_R |= (1 << 1); // Enable PWM1
    SYSCTL_RCGCGPIO_R |= (1 << 5); // Enable clock for PORTF
    GPIO_PORTF_AFSEL_R |= (1 << 2); // Enable alternate function on PF2
    GPIO_PORTF_PCTL_R |= 0x500; // Configure PF2 for PWM

    PWM1_3_CTL_R = 0; // Disable PWM generator during configuration
    PWM1_3_GENA_R = 0x8C; // Set PWM output high when counter matches
    PWM1_3_LOAD_R = 160; // Set load value for 100 kHz frequency
    PWM1_3_CMPA_R = (duty * 160) / 100; // Set initial duty cycle
    PWM1_3_CTL_R |= 0x01; // Enable PWM generator
    PWM1_ENABLE_R |= 0x040; // Enable PWM output
}

void main(void) {
    GPIO_PORTF_setup();
    PWM_setup();
    duty = 50; // Start with 50% duty cycle
    PWM1_3_CMPA_R = (duty * 160) / 100; // Set initial PWM compare value

    while (1) {
        // Do nothing, waiting for interrupts
    }
}

void GPIOPortF_Handler(void) {
    GPIO_PORTF_ICR_R = 0x11; // Clear interrupt flags

    if (GPIO_PORTF_MIS_R & 0x10) { // If Switch 1 (PF4) is pressed
        // Logic for increasing duty cycle
        if (duty < 100) {
            duty += 5; // Increase duty cycle by 5%
        }
    } else if (GPIO_PORTF_MIS_R & 0x01) { // If Switch 2 (PF0) is pressed
        // Logic for decreasing duty cycle
        if (duty > 5) {
            duty -= 5; // Decrease duty cycle by 5%
        }
    }

    PWM1_3_CMPA_R = (duty * 160) / 100; // Update PWM compare value
}
