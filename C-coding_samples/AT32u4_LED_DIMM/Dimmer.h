/*
 * IncFile1.h
 *
 * Created: 6/20/2015 12:31:25 AM
 *  Author: mike
 */ 


#ifndef DIMMER_H_
#define DIMMER_H_
/****************************************************************************
 Variables definitions
****************************************************************************/
volatile int A_DIMM_MAX;
volatile int B_DIMM_MAX; 
volatile int D_TIME;
//
/****************************************************************************
  Global definitions
****************************************************************************/
union DIMM_C_Reg                       // Status byte holding flags.
{
	unsigned char all;
	struct
	{
		unsigned char DIMM_ON:1;
		unsigned char AUTO_DIMM:1;
		unsigned char A_C_DONE:1;
		unsigned char B_C_DONE:1;
		unsigned char timer_en:1;
		unsigned char timer_flag:1;
		unsigned char is_running:1;
		unsigned char call_Dimmer:1;
	};
};
//
extern volatile union DIMM_C_Reg DIMM_C_Reg;
//
/****************************************************************************
  Bit and byte definitions
****************************************************************************/
#define MAX_PWM 0x7FFF			// 15bit PWM
#define EX_DIMM  4096           // The DIMM timing change PWM-point
#define Ch_A_ADC 0x04
#define Ch_B_ADC 0x05
#define DIMM_ADC_MASK 0x001F
#define D_TIME_MAN 1			// Time Delay parameter for manual dimming
#define D_TIME_A01 5000			// The initial Time Delay parameter for auto dimming on->off
#define D_TIME_A10 25			// The initial Time Delay parameter for auto dimming off->on
#define Short_TD 0xff			// An argument for "nop-loop" based SW delay.
#define SETUP_D 4095			// Timeout for enter dimm-max setup
//
/****************************************************************************
  Function definitions
****************************************************************************/
void Dimmer(void);
void T_Delay(int);
unsigned char P_time(int);
void Setup(void);
int ADC_read(unsigned char);
int Set_DIMM_MAX (unsigned char);
//
#endif /* DIMMER_H_ */