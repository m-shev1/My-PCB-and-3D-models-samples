/*
 * main.c
 *
 * Created: 6/20/2015 12:07:01 AM
 *  Author: mike
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_Master.h"
#include "Dimmer.h"
union DS3231_Reg DS3231_Reg;
unsigned char tmp_time_m, tmp_time_h, tmp_al1_m, tmp_al1_h;

int main( void )
{
	Setup();

	while(1)
	{
		if(DIMM_C_Reg.call_Dimmer) Dimmer();
	}
}