
#define MCU_CLOCK           1100000
#define PWM_FREQUENCY       50

#define SERVO_STEPS         180     // Maximum amount of steps in degrees (180 is common)
#define SERVO_MIN           450     // The minimum duty cycle for this servo
#define SERVO_MAX           2700    // The maximum duty cycle

#include <msp430_commons.c>

unsigned int PWM_Period = (MCU_CLOCK / PWM_FREQUENCY);  // PWM Period
unsigned int duty_cycle_at[SERVO_STEPS + 1];
unsigned int current_position, up;

void main (void) {
    // Shut off the watchdog (for now)
    WDTCTL = WDTPW + WDTHOLD;

    init_servo_angle_lut();

    Pin1_setup(P_OUT, BIT6);

    // Setup the PWM, with:
    // Up-mode & reset-set cycle
    PWM1_std_setup(PWM_Period, duty_cycle_at[0]);

    // select pin 1.2 for peripheral,
    // in this case for timer A as pwm output
    Pin1_select_peripheral(BIT2);

    // Setup P1.3, for triggering pwm
    Pin1_switch_setup(BIT3);

    current_position = 0;
    up = 1;

    __bis_SR_register(LPM0_bits | GIE);
}

// Port 1 interrupt service routine
#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(PORT1_VECTOR))) Port_1 (void)
#else
#error Compiler not supported!
#endif
{
    P1IES ^= BIT3;

    // Debounce~
    // disable further interrupt from P1.3
    P1IE  &= ~BIT3;
    P1IFG  =  0;

    P1OUT ^= BIT6;
    up ^= 1;

    // Enable Watchdog
    // Select SMCLK as source clock
    WDTCTL = WDT_MDLY_32;
    IFG1 &= ~WDTIFG;
    IE1  |=  WDTIE;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(WDT_VECTOR))) watchdog_timer (void)
#else
#error Compiler not supported!
#endif
{
    // Re-enable button push sensing
    P1IE  |=  BIT3;
    P1IFG  =  0;

    // Stop watchdog
    WDTCTL = WDTPW | WDTHOLD;
    IE1   &= ~WDTIE;
}

#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=TIMER0_A1_VECTOR
__interrupt void t0a1_isr (void)
#elif defined(__GNUC__)
void __attribute__ ((interrupt(TIMER0_A1_VECTOR))) t0a1_isr (void)
#else
#error Compiler not supported!
#endif
{
    if (up) {
        current_position = 0;
        TA0CCR1 = duty_cycle_at[current_position];
    } else {
        current_position = 180;
        TA0CCR1 = duty_cycle_at[current_position];
    }
    TA0CCTL1 &= ~CCIFG;
}
