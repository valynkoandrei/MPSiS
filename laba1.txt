#include <msp430.h>
int main(void)

{
	WDTCTL = WDTPW+WDTHOLD;                 		// Stop WDT
	P8DIR |= BIT1;                          		// P8.1 set as output
	P2DIR &= !(BIT2); 								// p2.2 (S2) set as input
	P1DIR &= !(BIT7); 								// p1.7 (S1) set as input
	P2REN |= BIT2;									// Permission
	P1REN |= BIT7;									// Permission
	P2OUT |= BIT2;
	P1OUT |= BIT7;
	P8OUT &= !BIT1;

	while(1)
	{

		while((P2IN & BIT2) && (P1IN & BIT7))       // when the buttons  is off
		{
			P8OUT &= BIT1;
		}
		P8OUT |= BIT1 ;
		while(!(P2IN & BIT2) || !(P1IN & BIT7))     //  when the button is on
		{
			for(i = 25000; i > 0; i--);				// delay
			P8OUT ^= BIT1;							// xor for led2
		}
		while((P2IN & BIT2) && (P1IN & BIT7))       // when the buttons is off
		{
			for(i = 25000; i > 0; i--);				// delay
			P8OUT ^= BIT1;							// xor for led2
		}
		while (!(P2IN & BIT2) || !(P1IN & BIT7))	// when the button is on
		{
			P8OUT &= BIT1;
			P8OUT &= BIT2;
		}
	}
}
