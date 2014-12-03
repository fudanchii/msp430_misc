#include <msp430.h>

int
main() {
    WDTCTL = WDTPW | WDTHOLD;

    P1DIR = BIT0;
    P1OUT = BIT0 | BIT3;

    P2DIR = 0xff;
    P2OUT = 0;

    P1IE  = BIT3;
    P1IES = BIT3;

    P1REN = BIT3;

    P1IFG &= ~BIT3;

    __bis_SR_register(LPM4_bits | GIE);
}

void __attribute__ ((interrupt(PORT1_VECTOR)))
port1_ISR() {
    P1OUT ^= BIT0;
    P1IFG &= ~BIT3;
}
