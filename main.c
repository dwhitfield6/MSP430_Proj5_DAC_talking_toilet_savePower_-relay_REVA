#include <msp430.h>
#include "sounddata.h"


#define LED0 BIT3
#define LED1 BIT5
#define debug BIT1
#define relay1 BIT4
#define relay2 BIT6
#define CS BIT0  //1.5 is SPI clock 1.7 is MOSI
#define MOSI BIT7  //1.5 is SPI clock 1.7 is MOSI
#define SCLK BIT5  //1.5 is SPI clock 1.7 is MOSI
#define INPUT_1 INCH_0
#define water BIT2


int MSB =0;
int LSB =0;
int count =0;
unsigned int sample =0;
int i=0;
int adcvalue1 =0;



void play(void);

unsigned int analogRead(unsigned int pin) {
 ADC10CTL0 = ADC10ON + ADC10SHT_2 + SREF_0;
 ADC10CTL1 = ADC10SSEL_0 + pin;
 if (pin==INCH_0){
 ADC10AE0 = 0x01;
 }
 else if(pin==INCH_5){
 ADC10AE0 = 0x20;
 }
 ADC10CTL0 |= ENC + ADC10SC;
 while (1) {
 if (((ADC10CTL0 & ADC10IFG)==ADC10IFG)) {
 ADC10CTL0 &= ~(ADC10IFG +ENC);
 break;
 }
 }
 return ADC10MEM;
 }


int main(void)
{
    WDTCTL = WDTPW + WDTHOLD;                 // Stop WDT

//    P1DIR |= (MOSI + SCLK);
    P2DIR |= (LED0 + LED1 + CS + debug + water + relay1);
    P2OUT |= (LED0 + LED1 + CS + debug + water);
    _delay_cycles(1000000);
    P2OUT |= ~(LED0 + LED1);
    P1DIR |= relay2;

    TA1CCTL0 |= CCIE;                             // CCR0 interrupt enabled
    TA1CTL |= TASSEL_2 + MC_1 + ID_2;           // SMCLK/8, upmode
    TA1CCR0 =  3000;

    P1SEL |= MOSI + SCLK;
    P1SEL2 |= MOSI + SCLK;

    UCB0CTL1 = UCSWRST;
    UCB0CTL0 |= UCMSB + UCMST + UCSYNC + UCCKPH; // 4-pin, 8-bit SPI master
    UCB0CTL1 |= UCSSEL_2;                     // SMCLK
    UCB0BR0 = 1;                          // /2
    UCB0BR1 = 0;                              //
    UCB0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**

    _BIS_SR(LPM0_bits + GIE);



}

void play(void)
{
	sample =0;
	TA1CCTL0 &= ~CCIE;
	P2OUT |= relay1;
	P1OUT |= relay2;
	_delay_cycles(100000);
	for(i = NUM_ELEMENTS;i>0;i--)
	{
	MSB = 0x70 | ((data[sample] & 0XF0) >> 4);
	LSB = ((data[sample] & 0X0F) << 4);
	P2OUT &= ~CS;
	UCB0TXBUF = MSB ;
	while (UCB0STAT & UCBUSY);
	UCB0TXBUF = LSB ;
	while (UCB0STAT & UCBUSY);
	P2OUT |= CS;
	sample++;
	}
	//P2OUT ^= debug;
	P2OUT &= ~relay1;
	P1OUT &= ~relay2;
	TA1CCTL0 |= CCIE;
}

// Timer_A3 Interrupt Vector (TAIV) handler
#pragma vector = TIMER1_A0_VECTOR
__interrupt void Timer1_A0 (void)
{



	adcvalue1 = analogRead( INPUT_1 );

	if(adcvalue1 < 200)
	{
	P2OUT |= (LED1);
	P2OUT &= ~(LED0);
	_delay_cycles(1000000);
	play();
	P2OUT |= (LED0);
	P2OUT &= ~(LED1);
	_delay_cycles(1000000);
	}

}
