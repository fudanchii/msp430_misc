
#define MCU_CLOCK           1100000
#define SERVO_FREQUENCY     50

#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           450     // The minimum duty cycle for this servo
#define SERVO_MAX           2700    // The maximum duty cycle

#include <msp430/commons.h>
#include <msp430/g2553/pwm.h>

unsigned int PWM1_Period = (MCU_CLOCK / SERVO_FREQUENCY);   // PWM Period
unsigned int PWM1_duty_cycle_at[SERVO_STEPS + 1];
unsigned int current_position;

void main (void) {
    basic_setup();
    INIT_SERVO_ANGLE_LUT(PWM1_duty_cycle_at, SERVO);
    PWM1_nointr_setup(PWM1_Period, PWM1_duty_cycle_at[0]);
    Pin1_peripheral_select(BIT2);
    current_position = 0;
    __bis_SR_register(LPM0_bits | GIE);
}

// Port 1 interrupt service routine
ISR_Vector(PORT1_VECTOR)
ISR_Define(Port_1(void)) {
    Switch_edgeflip();

    // Debounce~
    // disable further interrupt from P1.3
    Switch_disable();

    GreenLED_flip();
    current_position ^= 180;
    PWM1_set_duty_cycle(PWM1_duty_cycle_at[current_position]);

    // Enable Watchdog
    // Select SMCLK as source
    Watchdog_enable(WDT_MDLY_32);
}

ISR_Vector(WDT_VECTOR)
ISR_Define(watchdog_timer(void)) {
    Switch_enable();

    // Stop watchdog
    Watchdog_disable();
}
