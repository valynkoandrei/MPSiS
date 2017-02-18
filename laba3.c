#include <msp430.h> 

/*
 * main.c
 */

char lowPowerMode = 0;	// 0 - отключен, 1 - выключен
volatile unsigned int lowVoltageMode = 0; // 0 - отключен, 1 - выключен

void increaseVcoreLevel (unsigned int level) {
	PMMCTL0_H = PMMPW_H;	// открыть PMM регистры дл€ доступа
	SVSMHCTL = SVSHE | SVSHRVL0 * level | SVMHE | SVSMHRRL0 * level;	// управление SVM/SVS на входе
	SVSMLCTL = SVSLE | SVMLE | SVSMLRRL0 * level;	// установка SVM в новый уровень

	while (!(PMMIFG & SVSMLDLYIFG));	// ожидание пока установитс€ SVM

	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);	// очистка ранее установленных флагов
	PMMCTL0_L = PMMCOREV0 * level;	// установка Vcore в новый уровень

	if ((PMMIFG & SVMLIFG))	// ожидание пока будет достигнут новый уровень
		while (!(PMMIFG & SVMLVLRIFG));

	SVSMLCTL = SVSLE | SVSLRVL0 * level | SVMLE | SVSMLRRL0 * level;	// управление SVM/SVS на выходе
	PMMCTL0_H = 0;	// закрыть PMM регистры дл€ доступа
}

void decreaseVcoreLevel (unsigned int level) {
	PMMCTL0_H = PMMPW_H;	// открыть PMM регистры дл€ доступа
	SVSMLCTL = SVSLE | SVSLRVL0 * level | SVMLE | SVSMLRRL0 * level;	// управление SVM/SVS на входе

	while (!(PMMIFG & SVSMLDLYIFG));	// ожидание пока установитс€ SVM

	PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);	// очистка ранее установленных флагов
	PMMCTL0_L = PMMCOREV0 * level;	// установка Vcore в новый уровень
	PMMCTL0_H = 0;	// открыть PMM регистры дл€ доступа
}

#pragma vector = PORT1_VECTOR
__interrupt void PORT1_ISR(void) {
	switch(P1IFG & BIT7) {
		case BIT7:
			P1IFG &= ~BIT7;	// сброс флага прерывани€
			if(lowVoltageMode) {
				P1OUT &= ~BIT1;	// выключение светодиода 1
				increaseVcoreLevel(2);
				increaseVcoreLevel(3);
				UCSCTL4 &= ~SELM__XT1CLK;	// отключаем старый источник (XT1CLK) дл€ MCLK
				UCSCTL4 &= ~SELA__XT1CLK;	// отключаем старый источник (XT1CLK) дл€ ACLK
				UCSCTL4 |= SELM__DCOCLK | SELA__DCOCLK;	// источник дл€ MCLK и ACLK - DCOCLK
				P1OUT |= BIT2;	// выключение светодиода 2
			}
			else {
				P1OUT &= ~BIT2; // выключение светодиода 2
				UCSCTL4 &= ~SELA__DCOCLK;	// отключаем старый источник (DCOCLK) дл€ ACLK
				UCSCTL4 &= ~SELM__DCOCLK;	// отключаем старый источник (DCOCLK) дл€ MCLK
				UCSCTL4 |= SELM__XT1CLK | SELA__XT1CLK;	// источник дл€ MCLK и ACLK - XT1CLK
				decreaseVcoreLevel(2);
				decreaseVcoreLevel(1);
				P1OUT |= BIT1;	// выключение светодиода 1
			}
			lowVoltageMode ^= BIT0;
			return;
		default:
			P1IFG = 0;
			return;
		}
}

#pragma vector = PORT2_VECTOR
__interrupt void PORT2_ISR(void) {
	switch(P2IFG & BIT2) {
		case BIT2:
			P2IFG &= ~BIT2;	// сброс флага прерывани€
			P1OUT ^= BIT0;
			P8OUT ^= BIT2;

			if(lowPowerMode) {
				_BIC_SR_IRQ(SCG0 + CPUOFF);	// выход в LPM1
			}
			else {
				_BIS_SR_IRQ(SCG0 + CPUOFF); // вход в LPM1 (отключание системы тактового генератора и процессора)
			}
			lowPowerMode ^= BIT0;
			return;
		default:
			P2IFG = 0;
			return;
		}
}

#pragma vector = WDT_VECTOR
__interrupt void WDT_ISR(void) {
	P1OUT ^= BIT5;
}

int main(void) {
	volatile int i = 0;
    //WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	
    P1DIR |= BIT0 | BIT1 | BIT2 | BIT5;	// настройка светодиодов на выход
    P8DIR |= BIT2;
    P7DIR |= BIT7;
    P7SEL |= BIT7;

    P1OUT |= BIT0 | BIT2 | BIT5;	// включение светодиода LED1, LED5, LED8
    P1OUT &= ~BIT1;	// выключение светодиода LED4
    P8OUT &= ~BIT2;	// выключение светодиода LED3


    P1DIR &= ~BIT7;
	P2DIR &= ~BIT2;

	P1REN |= BIT7;	//разрешение подт€гивающего резистора
	P2REN |= BIT2;

	P1OUT |= BIT7;	//настройка подт€гивающего резистора
	P2OUT |= BIT2;

	P1IES |= BIT7;	//прерывание по переходу из 1 в 0(нажатие кнопки)
	P1IFG &= ~BIT7; //обнуление флага прерывани€ кнопки
	P1IE |= BIT7;	//разрешение прерывани€

	P2IES |= BIT2;	//настройка прерывани€ дл€ второй кнопки
	P2IFG &= ~BIT2;
	P2IE |= BIT2;

	increaseVcoreLevel(1);
	increaseVcoreLevel(2);
	increaseVcoreLevel(3);

	UCSCTL4 |= BIT0 | BIT1;
	UCSCTL4 &= ~BIT2;
	UCSCTL1 = DCORSEL_0;	// выбираем диапазон частот
	UCSCTL2 = FLLN2 |FLLN4;	// N = 19 (уровень частоты в выбранном диапазоне)
	UCSCTL4 |= SELA__DCOCLK | SELM__DCOCLK;	// источник дл€ SMLK - DCOCLK

	WDTCTL = WDTPW+WDTTMSEL+WDTCNTCL+WDTIS2+WDTSSEL0;	//WDTPW - пароль, WDTTMSEL - режим(интервальный),
	   	   	   	   	   	   	   	   	   	   	   	   	    //WDTCNTCL - сброс, WDTIS2 - задержка
														//WDTSSEL0 - источник дл€ WDT(ACLK)

	SFRIE1 |= WDTIE;	//разрешение прерывани€ таймера

	_enable_interrupt();	//включение маскированных прерываний

	while(1) {
	}

	return 0;
}

