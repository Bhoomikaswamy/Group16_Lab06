#include <stdint.h>
#include <stdbool.h>
#include <tm4c123gh6pm.h>

#define DUTY_CYCLE_STEP 5   // Step size for increasing/decreasing duty cycle
#define MAX_DUTY_CYCLE 100  // Maximum duty cycle (100%)
#define MIN_DUTY_CYCLE 0    // Minimum duty cycle (0%)
#define PWM_FREQUENCY 100000 // 100 kHz
#define BASE_DELAY 16000     // Base delay for 50% duty cycle

volatile uint8_t duty_cycle = 50; // Initialize duty cycle to 50%

void GPIO_Init(void);
void GPIO_Interrupt_Init(void);
void GPIOPortF_Handler(void);
void Delay(uint32_t time);

int main(void) {
    GPIO_Init();               // Initialize GPIO
    GPIO_Interrupt_Init();     // Initialize GPIO interrupts

    while (1) {
        // Main loop does nothing; all work is done in the interrupt handler
    }
}

// GPIO Initialization function
void GPIO_Init(void) {
    SYSCTL_RCGCGPIO_R |= 0x20;  // Enable clock for Port F
    while ((SYSCTL_PRGPIO_R & 0x20) == 0) {};  // Wait for Port F to be ready

    GPIO_PORTF_LOCK_R = 0x4C4F434B;  // Unlock GPIO Port F
    GPIO_PORTF_CR_R = 0x11;          // Allow changes to PF4 (SW1) and PF0 (SW2)

    GPIO_PORTF_DIR_R |= 0x02;        // Set PF1 (Red LED) as output
    GPIO_PORTF_DIR_R &= ~0x11;       // Set PF4 (SW1) and PF0 (SW2) as inputs
    GPIO_PORTF_DEN_R |= 0x13;        // Enable digital functionality on PF1, PF4, and PF0
    GPIO_PORTF_PUR_R |= 0x11;        // Enable pull-up resistors on PF4 and PF0
}

// GPIO Interrupt Setup function
void GPIO_Interrupt_Init(void) {
    GPIO_PORTF_IS_R &= ~0x11;        // PF4 and PF0 are edge-sensitive
    GPIO_PORTF_IBE_R &= ~0x11;       // Interrupt is controlled by GPIOIEV
    GPIO_PORTF_IEV_R &= ~0x11;       // Falling edge trigger
    GPIO_PORTF_IM_R |= 0x11;         // Unmask interrupt for PF4 and PF0

    NVIC_EN0_R |= (1 << 30);         // Enable interrupt in NVIC for Port F
    GPIO_PORTF_ICR_R |= 0x11;        // Clear any prior interrupt
}

// GPIO Interrupt Handler for Port F
void GPIOPortF_Handler(void) {
    if (GPIO_PORTF_RIS_R & 0x10) {   // Interrupt on PF4 (SW1)
        GPIO_PORTF_ICR_R |= 0x10;    // Clear interrupt flag for PF4
        // Increase duty cycle by 5% if not already at maximum
        if (duty_cycle <= (MAX_DUTY_CYCLE - DUTY_CYCLE_STEP)) {
            duty_cycle += DUTY_CYCLE_STEP;
        }
        Control_LED();
    } else if (GPIO_PORTF_RIS_R & 0x01) {   // Interrupt on PF0 (SW2)
        GPIO_PORTF_ICR_R |= 0x01;    // Clear interrupt flag for PF0
        // Decrease duty cycle by 5% if not already at minimum
        if (duty_cycle >= (MIN_DUTY_CYCLE + DUTY_CYCLE_STEP)) {
            duty_cycle -= DUTY_CYCLE_STEP;
        }
        Control_LED();
    }
}

// Control LED based on duty cycle
void Control_LED(void) {
    GPIO_PORTF_DATA_R |= 0x02;  // Turn on LED
    Delay((duty_cycle * BASE_DELAY) / 100); // Delay for duty cycle
    GPIO_PORTF_DATA_R &= ~0x02; // Turn off LED
}

// Simple delay function
void Delay(uint32_t time) {
    volatile uint32_t count;
    for (count = 0; count < time; count++) {
        // Busy wait
    }
}
