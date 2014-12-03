#include <msp430.h>

int i;

int
main(void) {

    // Setup Watchdog timer with ACLK
    // sourced from VLO (LFXT1S_2) at ~12kHz
    // so it will trigger interrupt within
    // interval:
    // (32/12) * 250ms) ~= ~670ms
    BCSCTL1 |= DIVA_0;
    BCSCTL3 |= LFXT1S_2;

    // Disable watchdog for now
    WDTCTL = WDTPW | WDTHOLD;

    // Set output direction on Pin 1.0 and 1.6
    // start from off
    P1DIR |= BIT0 | BIT6;
    P1OUT &= ~(BIT0 | BIT6);

    // Enable interrupt on Pin 1.3
    // on Hi-Lo transition
    P1IE  |= BIT3;
    P1IES |= BIT3;

    // Enable pull-up resistor on Pin 1.3
    P1OUT |= BIT3;
    P1REN |= BIT3;

    // Clear interrupt flag on Pin 1.3
    // so we can accept more interrupt
    P1IFG &= ~BIT3;

    i = 0;

    // Go sleep, but listen for interrupt
    __bis_SR_register(LPM3_bits | GIE);
    return 0;
}

// Port 1 interrupt service routine
void __attribute__ ((interrupt(PORT1_VECTOR)))
Port_1 (void) {
    // Debounce~
    // disable further interrupt from P1.3
    P1IE  &= ~BIT3;
    P1IFG  =  0;

    i++;
    if (i % 15 == 0)            // red + green
        P1OUT |= (BIT0 | BIT6);
    else if (i % 3 == 0) {      // red
        P1OUT &= ~BIT6;
        P1OUT |= BIT0;
    }
    else if (i % 5 == 0) {      // green
        P1OUT &= ~BIT0;
        P1OUT |= BIT6;
    }
    else {
        P1OUT &= ~(BIT0 | BIT6);
    }
    if (i >= 100) i = 0;

    // Enable Watchdog
    WDTCTL = WDT_ADLY_250;
    IFG1 &= ~WDTIFG;
    IE1  |=  WDTIE;
}

void __attribute__ ((interrupt(WDT_VECTOR)))
watchdog_timer (void) {
    // Re-enable button push sensing
    P1IE  |=  BIT3;
    P1IFG  =  0;

    // Stop watchdog
    WDTCTL = WDTPW | WDTHOLD;
    IE1   &= ~WDTIE;

    // blink blink
    P1OUT &= ~(BIT0 | BIT6);
    __delay_cycles(100000);
    P1OUT |= BIT6;
    __delay_cycles(100000);
    P1OUT &= ~BIT6;
}

