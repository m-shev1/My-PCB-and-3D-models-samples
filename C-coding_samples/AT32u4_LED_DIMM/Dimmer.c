/*
 * Dimmer.c
 *
 * Created: 6/20/2015 12:27:25 AM
 *  Author: mike
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>
#include "Dimmer.h"
#include "TWI_Master.h"

volatile union DIMM_C_Reg DIMM_C_Reg;
volatile int A_DIMM_MAX;
volatile int B_DIMM_MAX;
volatile int D_TIME;
/*******************************************************************************
  This function is smoothly switching ON/OFF OCR1A and OCR1B PWM-DIMM channels
  in manual or automatic modes
*******************************************************************************/
void Dimmer(void)
{	
	int DIMM_C;
//
	if (DIMM_C_Reg.is_running) return;	// Instantly escape the function if it was re-called before the finish.
	else 
	{
		DIMM_C_Reg.is_running = 1;			// Set the function running flag, to indicate that it is busy.
		DIMM_C_Reg.call_Dimmer = 0;
	}
//	
	if (OCR1A > OCR1B) DIMM_C = OCR1A;
	else DIMM_C = OCR1B;
//
	if (DIMM_C_Reg.AUTO_DIMM)
	{
		if (DIMM_C_Reg.DIMM_ON) D_TIME = D_TIME_A01;
		else D_TIME = D_TIME_A10;
	}
	else D_TIME = D_TIME_MAN;
// 
	while(DIMM_C_Reg.is_running)
	{
		if (DIMM_C >= A_DIMM_MAX) DIMM_C_Reg.A_C_DONE = 1;	//  Align A or B channels		//
			else DIMM_C_Reg.A_C_DONE = 0;					//	PWM-duties with current PWM	//
		if (DIMM_C >= B_DIMM_MAX) DIMM_C_Reg.B_C_DONE = 1;
			else DIMM_C_Reg.B_C_DONE = 0;
//
		if (DIMM_C_Reg.DIMM_ON)
		{
			if (DIMM_C_Reg.A_C_DONE && DIMM_C_Reg.B_C_DONE)
			{
				DIMM_C_Reg.is_running = 0;
				return; //Escape, if A and B dimm-channels got fully ON
			}
			else ++DIMM_C;
		}
		else if (DIMM_C) --DIMM_C; 
		else
			{
				DIMM_C_Reg.is_running = 0;
				return; //Escape, if A and B dimm-channels got fully OFF
			}		
//				
		if (!(DIMM_C_Reg.A_C_DONE)) OCR1A = DIMM_C;
		if (!(DIMM_C_Reg.B_C_DONE)) OCR1B = DIMM_C;
		if (DIMM_C_Reg.AUTO_DIMM && DIMM_C < EX_DIMM)
		{
			if (DIMM_C_Reg.DIMM_ON) --D_TIME;
			else ++D_TIME;
		}
		T_Delay(D_TIME);
	}
}
//
/*********************************************************************************
This Interrupt Service Routine (ISR) function triggered by DS3231 timer interrupt,
to activate the dimming control routine (Dimmer()) in manual mode, or for ajusting
dimming channels max PWM duty value.
*********************************************************************************/
ISR(INT3_vect)
{
	int A_temp, B_temp, i;
	sei();
//
	DIMM_C_Reg.AUTO_DIMM = 0;
	if(P_time(SETUP_D)) // Initiate dimming routine in manual (fast) mode
	{
		DIMM_C_Reg.DIMM_ON = ~DIMM_C_Reg.DIMM_ON;
		if (DIMM_C_Reg.is_running) DIMM_C_Reg.timer_en = 0;
		else DIMM_C_Reg.call_Dimmer = 1;
	}
	else                // Enter channels A&B Max_DIMM value setup
	{
		PORTF = 0x80;
		DIMM_C_Reg.is_running = 0;
		while(!(PIND & (1<<PD3)));
			cli();
			A_temp = Set_DIMM_MAX(Ch_A_ADC);
			B_temp = Set_DIMM_MAX(Ch_B_ADC);
//
		while ((OCR1A != A_temp) || (OCR1B != B_temp))
		{
			if (OCR1A < A_temp) OCR1A++;
			if (OCR1A > A_temp) OCR1A--;
			if (OCR1B < B_temp) OCR1B++;
			if (OCR1B > B_temp) OCR1B--;
			i = Short_TD;
			while(i) i--;
		}	
		while (PIND & (1<<PD3))
		{
			OCR1A = Set_DIMM_MAX(Ch_A_ADC);
			OCR1B = Set_DIMM_MAX(Ch_B_ADC);
		}
			A_DIMM_MAX = OCR1A;
			B_DIMM_MAX = OCR1B;
		while(!(PIND & (1<<PD3)));
	}
		PORTF = 0x00;
}
//
/*********************************************************************************
This Interrupt Service Routine (ISR) function triggered by DS3231 timer interrupt,
to activate the dimming control routine (Dimmer()) in an automatic mode
*********************************************************************************/
ISR(INT6_vect)
{
	unsigned char int_f;
	
	int_f = DS3231_Check_IntF();
	
	if (DIMM_C_Reg.is_running) int_f = 4;
	
	switch (int_f)
	{
		case ALARM_1:	// The time to switch light ON
			DIMM_C_Reg.AUTO_DIMM = 1;
			DIMM_C_Reg.DIMM_ON = 1;
			Dimmer();
			break;
		case ALARM_2:	// The time to switch light OFF
		case BOTH_AL:
			DIMM_C_Reg.AUTO_DIMM = 1;
			DIMM_C_Reg.DIMM_ON = 0;
			Dimmer();
			break;
		case 4:			// 
			break;
	}
}
//
/*****************************************************************************
This is the Interrupt Service Routine (ISR) function, and it provides 1358.7Hz
(0.736mS)clock ticks for the main time delay function (T_Delay);
*****************************************************************************/
ISR(TIMER0_COMPA_vect)
{
	DIMM_C_Reg.timer_flag = 0;
}
//
/****************************************************************************
The time delay function (T_Delay), provides count1x0.736mS time delay
****************************************************************************/
void T_Delay(int count1)
{
	DIMM_C_Reg.timer_en = 1;
	while(count1 && DIMM_C_Reg.timer_en)
	{
		DIMM_C_Reg.timer_flag = 1;
		while (DIMM_C_Reg.timer_flag);
		--count1;
	}
}
//
/****************************************************************************
This function calculates and returns the button pushing time(PD3 pin)
****************************************************************************/
unsigned char P_time(int count1)
{
	while(!(PIND & (1<<PD3)))
	{
		if(count1)
		{
			DIMM_C_Reg.timer_flag = 1;
			while (DIMM_C_Reg.timer_flag);
			count1--;
		}
		else return 0;
	}
	return 1;
}
//
void Setup(void)
{
// Setup I/O Ports
// B-port
	PORTB = (1<<PB7)|(1<<PB4)|(1<<PB3)|(1<<PB2)|(1<<PB1)|(1<<PB0); // Pull up PB0-4,7 pins
	DDRB = (1<<PB6)|(1<<PB5); // Set PB5-6 pins as outputs
// C-port
	PORTC = (1<<PC7); // Pull up PC7 pin
	DDRC = (1<<PC6); // Set PC6 pin as output
// D-port	
	PORTD = (1<<PD7)|(1<<PD6)|(1<<PD5)|(1<<PD3)|(1<<PD2); // Pull up PD1-7 pins
// E-port
	PORTE = (1<<PE6)|(1<<PE2); // Pull up PE2,6 pins
// F-port
	PORTF = (1<<PF1)|(1<<PF0); // Pull up PF0,1,6,7 pins
	DDRF = (1<<PF7)|(1<<PF6);
// Setup Timer0 to generate int. in 1358.7Hz rate
	OCR0A = 184;
	OCR0B = 92;
	TCCR0A = (1<<WGM01)|(1<<COM0B0); // Set Timer0 to CTC mode and toggle OC0B
	TCCR0B = (1<<CS01)|(1<<CS00); // Set Timer0 pre-scale=64
	TIMSK0 = (1<<OCIE0A); // Set interrupt on Timer0 A-compare
// Setup Timer1 as 15-bit PWM
	ICR1 = MAX_PWM;
	OCR1A = 0x0000;
	OCR1B = 0x0000;
	TCCR1A = (1<<COM1A1)|(1<<COM1B1)|(1<<WGM01); // Enable Fast PWM on OC1A and OC1B pins
	TCCR1B = (1<<WGM13)|(1<<WGM12)|(1 << CS10); // Set Timer1 to Fast PWM(TOP=ICR1), pre-scale=1
// Setup ADC
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); //Enable ADC and set the clock pre-scaler to 128
// Set Max PWM duties for A and B dimm-channels
	A_DIMM_MAX = Set_DIMM_MAX(Ch_A_ADC);
	B_DIMM_MAX = Set_DIMM_MAX(Ch_B_ADC);
//
	DIMM_C_Reg.all = 0x00;
	TWI_Master_Initialise();
	sei();
	if (DS3231_Init() & 0x01) DIMM_C_Reg.DIMM_ON =1;
	DIMM_C_Reg.call_Dimmer = 1;
	EICRA = (1 << ISC31);				// Set INT3 to detect the signal failing edge.
	EICRB = (1 << ISC61);				// Set INT6 to detect the signal failing edge.
	EIMSK = (1 << INT6)|(1 << INT3);	// Enable INT3 and INT6 interrupts.
}
//
int ADC_read(unsigned char dimm_ch_adc)
{
	ADMUX = (1<<REFS0)|(1<<ADLAR)|(1<<MUX2)|dimm_ch_adc;
		ADCSRA |= (1<<ADSC);
	while(ADCSRA &(1<<ADSC));				// Skip the first measurement, cause it can be not very accurate
		ADCSRA |= (1<<ADSC);
	while(ADCSRA &(1<<ADSC));
	return ADC;
}
//
int Set_DIMM_MAX (unsigned char dimm_ch_adc)
{
	int tmp;
	tmp = (ADC_read(dimm_ch_adc) >> 1) | 0x001f;
	tmp = tmp & 0x7fff;
	return tmp;
}