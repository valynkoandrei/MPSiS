#include <msp430.h>

volatile int x = 0;

volatile int bIsPush = 0;
volatile const int period = 10000;

int main(void)
{
	//переводим сторожевой таймер в интервальный режим
	WDTCTL = WDTPW + WDTTMSEL + WDTCNTCL + WDTIS2;

	P8DIR |= BIT1;                          		// P8.1 set as output
	P2DIR &= !(BIT2); 								// p2.2 (S2) set as input
	P1DIR &= !(BIT7); 								// p1.7 (S1) set as input
	P2REN |= BIT2;									// Permission
	P1REN |= BIT7;									// Permission
	P2OUT |= BIT2;
	P1OUT |= BIT7;
	P8OUT |= !BIT1;


	P1IFG &= ~BIT7;
	P1IES |= BIT7;
	P1IE |= BIT7;

	P2IFG &= ~BIT2;
	P2IES |= BIT2;
    P2IE |= BIT2;

    SFRIE1 |=  WDTIE;

  __bis_SR_register(LPM0_bits + GIE);       // Enter LPM0, enable interrupts
  __no_operation();                         // For debugger
}


#pragma vector=PORT2_VECTOR
__interrupt void PORT2_ISR(void)
{
	P2IE &= ~BIT2; //запрещаем прерывания
	if(!(P2IN & BIT2))
	{
		if(bIsPush == 1) bIsPush=0;
		else bIsPush=1;
	}
		P2IE |= BIT2; //разрешаем прерывания
		P2IFG &= ~BIT2;
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
		P1IE &= ~BIT7; //запрещаем прерывания
		if(!(P1IN & BIT7))
		{
			if(bIsPush == 1) bIsPush=0;
			else bIsPush=1;
		}
		P1IE |= BIT7; //разрешаем прерывания
		P1IFG &= ~BIT7;
}

#pragma vector = WDT_VECTOR
__interrupt void watchdog_timer(void){
	//очищаем флаг прерывания от таймера
	SFRIFG1 &= ~BIT0;

	x++;
	if(bIsPush)
	{
		if(x / period == 0)
		{
			x=0;
			P8OUT ^= BIT1;
		}
	}
	else
	{
		x=0;
		P8OUT &= !(BIT1);
	}
}


