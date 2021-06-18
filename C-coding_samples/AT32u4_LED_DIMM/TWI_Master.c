/*
 * TWI_Master.c
 *
 * Created: 6/20/2015 12:47:32 AM
 *  Author: mike
 */ 


#include <avr/io.h>
#include <avr/interrupt.h>
#include "TWI_Master.h"

static unsigned char TWI_buf[ TWI_BUFFER_SIZE ];    // Transceiver buffer
static unsigned char TWI_msgSize;                   // Number of bytes to be transmitted.
static unsigned char TWI_state = TWI_NO_STATE;      // State byte. Default set to TWI_NO_STATE.

union TWI_statusReg TWI_statusReg = {0};            // TWI_statusReg is defined in TWI_Master.h
union DS3231_Reg DS3231_Reg;
//
/****************************************************************************
Call this function to set up the TWI master to its initial standby state.
Remember to enable interrupts from the main application after initializing the TWI.
****************************************************************************/
void TWI_Master_Initialise(void)
{
  TWBR = TWI_TWBR;                                  // Set bit rate register (Baudrate). Defined in header file.
// TWSR = TWI_TWPS;                                  // Not used. Driver presumes prescaler to be 00.
  TWDR = 0xFF;                                      // Default content = SDA released.
  TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins.
         (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt.
         (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests.
         (0<<TWWC);                                 //
}    
//    
/****************************************************************************
Call this function to test if the TWI_ISR is busy transmitting.
****************************************************************************/
unsigned char TWI_Transceiver_Busy( void )
{
  return ( TWCR & (1<<TWIE) );                  // IF TWI Interrupt is enabled then the Transceiver is busy
}
//
/****************************************************************************
Call this function to fetch the state information of the previous operation. The function will hold execution (loop)
until the TWI_ISR has completed with the previous operation. If there was an error, then the function 
will return the TWI State code. 
****************************************************************************/
unsigned char TWI_Get_State_Info( void )
{
  while ( TWI_Transceiver_Busy() );             // Wait until TWI has completed the transmission.
  return ( TWI_state );                         // Return error state.
}
//
/****************************************************************************
Call this function to send a prepared message. The first byte must contain the slave address and the
read/write bit. Consecutive bytes contain the data to be sent, or empty locations for data to be read
from the slave. Also include how many bytes that should be sent/read including the address byte.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
//
void TWI_Start_Transceiver_With_Data( unsigned char *msg, unsigned char msgSize )
{
  unsigned char temp;

  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.

  TWI_msgSize = msgSize;                        // Number of data to transmit.
  TWI_buf[0]  = msg[0];                         // Store slave address with R/W setting.
  if (!( msg[0] & (TRUE<<TWI_READ_BIT) ))       // If it is a write operation, then also copy data.
  {
    for ( temp = 1; temp < msgSize; temp++ )
      TWI_buf[ temp ] = msg[ temp ];
  }
  TWI_statusReg.all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
         (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
         (0<<TWWC);                             //
}
//
/****************************************************************************
Call this function to resend the last message. The driver will reuse the data previously put in the transceiver buffers.
The function will hold execution (loop) until the TWI_ISR has completed with the previous operation,
then initialize the next operation and return.
****************************************************************************/
void TWI_Start_Transceiver( void )
{
  while ( TWI_Transceiver_Busy() );             // Wait until TWI is ready for next transmission.
  TWI_statusReg.all = 0;      
  TWI_state         = TWI_NO_STATE ;
  TWCR = (1<<TWEN)|                             // TWI Interface enabled.
         (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
         (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
         (0<<TWWC);                             //
}
//
/**********************************************************************************************
This function reads data from the DS3231. The mode of reading; one register or whole reg. set 
at once, depends on the reg_n parameter value. If the value is over the last DS3231 reg. number
(0x12) then the whole reg. set reads to the "DS3231_reg" buffer, otherwise the only reg. value
 returns by the function.
**********************************************************************************************/
unsigned char DS3231_Reg_Read( unsigned char reg_n)
{
	unsigned char i, j, point, msg_size;
	unsigned char DS3231_address = 0x68;
	  
	  while ( TWI_Transceiver_Busy() );       // Wait until TWI is ready for next transmission.
	  
	  if (reg_n > 0x12)
	  {
		  point = 0x00;
		  msg_size = 20;
	  }
	  else
	  {
		  point = reg_n;
		  msg_size = 2;
	  }
	
	TWI_buf[0] = (DS3231_address<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // Set DS3231 reg.start address to read from
	TWI_buf[1] = point;
	TWI_msgSize = 0x02;                      // Number of data to transmit.
	TWI_statusReg.all = 0;
	TWI_state         = TWI_NO_STATE ;
	TWCR = (1<<TWEN)|                        // TWI Interface enabled.
	  (1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
	  (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
	  (0<<TWWC);                             //
 
		while ( TWI_Transceiver_Busy() );      // Wait until TWI is ready for next transmission.

	TWI_buf[0] = (DS3231_address<<TWI_ADR_BITS) | (TRUE<<TWI_READ_BIT); // Read DS3231 regs.
	TWI_msgSize = msg_size;                      // Number of data to transmit.
	TWI_statusReg.all = 0;
	TWI_state         = TWI_NO_STATE ;
	TWCR = (1<<TWEN)|                        // TWI Interface enabled.
	(1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
	(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
	(0<<TWWC);                             //

		while ( TWI_Transceiver_Busy() );      // Wait until TWI is ready for next transmission.

	if( TWI_statusReg.lastTransOK )               // Last transmission competed successfully.
	{
		if (reg_n > 0x12)
		{
			for ( i=0, j=1 ; j < TWI_msgSize; i++, j++ )                 // Copy data from Transceiver buffer to DS3231_Reg.
			{
				DS3231_Reg.all[i] = TWI_buf[j];
			}
			return( TWI_statusReg.lastTransOK );
//			return TWI_buf[1];
		}
		else return TWI_buf[1];
	}
	else return 0;
//	return TWI_buf[1];
  }
//
/**********************************************************************************
This function is called when to initialize/check the DS3231 status after the system
reset. It compares the DS3231 current time with AL1 and AL2 settings and returns 
0x01 if the light shall be switched ON, and 0x02 if OFF.
**********************************************************************************/
unsigned char DS3231_Init( void )
{
	unsigned char init_status = 0x00;
	
	if( !(DS3231_Reg_Read(0x3F) ))			// Escape with error code, if no access to DS3231
	{
		init_status = 0x10;
	}
	
	if (DS3231_Reg.status & 0x80)	// Check, if DS3231 has not been shutdown
	{
		init_status = 0x20;
		DS3231_Reg_Reco();
	} 
//
//
	if ((DS3231_Reg.AL1_hour_min < DS3231_Reg.hour_min) && (DS3231_Reg.hour_min < DS3231_Reg.AL2_hour_min))
	{	
		init_status = 0x01;	
	}	// The light shall be switched ON
	else
	{	
		init_status = 0x00;
	}	// The light shall be switched OFF
	
	DS3231_Reg_Write(0x0F, 0x00);	// Clean AL1, AL2 interrupt flags, if any.
	DS3231_Reg_Write(0x0E, 0x07);	// Make sure that AL1 and AL2 interrupts are enabled.
	return init_status;
}
//
/*****************************************************************************
Just placeholder for DS3231 recovery function yet, does nothing at the moment.
*****************************************************************************/
unsigned char DS3231_Reg_Reco( void)
{
	unsigned char reco_status = 0x00;
	return reco_status;
}
//
/********************************************************************************************
   This function is called to figure out what the timer event (AL1 or AL2)has occurred
when the interrupt from DS3231 is triggered, Then the function returns alarms flags status.
********************************************************************************************/
unsigned char DS3231_Check_IntF( void)
{
	unsigned char int_f;
	int_f = (DS3231_Reg_Read(0xf));
	DS3231_Reg_Write(0xf,0x00);
	int_f &= 0x03;
	return int_f;
}
//
/********************************************************************************************
   This function writes a byte to one DS3231 register at once.
********************************************************************************************/
unsigned char DS3231_Reg_Write(unsigned char reg_n, unsigned char reg_v)
{
		unsigned char DS3231_address = 0x68;
		
		while ( TWI_Transceiver_Busy() );       // Wait until TWI is ready for next transmission.
			
		TWI_buf[0] = (DS3231_address<<TWI_ADR_BITS) | (FALSE<<TWI_READ_BIT); // Set DS3231 reg.start address to read from
		TWI_buf[1] = reg_n;
		TWI_buf[2] = reg_v;
		TWI_msgSize = 0x03;                      // Number of data to transmit.
		TWI_statusReg.all = 0;
		TWI_state         = TWI_NO_STATE ;
		TWCR = (1<<TWEN)|                        // TWI Interface enabled.
		(1<<TWIE)|(1<<TWINT)|                  // Enable TWI Interupt and clear the flag.
		(0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|       // Initiate a START condition.
		(0<<TWWC);                             //
		
		while ( TWI_Transceiver_Busy() );       // Wait until TWI is ready for next transmission.
		
		return( TWI_statusReg.lastTransOK );
}
//
// ********** Interrupt Handlers ********** //
/****************************************************************************
This function is the Interrupt Service Routine (ISR), and called when the TWI interrupt is triggered;
that is whenever a TWI event has occurred. This function should not be called directly from the main
application.
****************************************************************************/
ISR(TWI_vect)
{
  static unsigned char TWI_bufPtr;
  
  switch (TWSR)
  {
    case TWI_START:             // START has been transmitted  
    case TWI_REP_START:         // Repeated START has been transmitted
      TWI_bufPtr = 0;                                     // Set buffer pointer to the TWI Address location
    case TWI_MTX_ADR_ACK:       // SLA+W has been tramsmitted and ACK received
    case TWI_MTX_DATA_ACK:      // Data byte has been tramsmitted and ACK received
      if (TWI_bufPtr < TWI_msgSize)
      {
        TWDR = TWI_buf[TWI_bufPtr++];
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to send byte
               (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           //
               (0<<TWWC);                                 //  
      }else                    // Send STOP after last byte
      {
        TWI_statusReg.lastTransOK = TRUE;                 // Set status bits to completed successfully. 
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
               (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
               (0<<TWWC);                                 //
      }
      break;
    case TWI_MRX_DATA_ACK:      // Data byte has been received and ACK tramsmitted
      TWI_buf[TWI_bufPtr++] = TWDR;
    case TWI_MRX_ADR_ACK:       // SLA+R has been tramsmitted and ACK received
      if (TWI_bufPtr < (TWI_msgSize-1) )                  // Detect the last byte to NACK it.
      {
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
               (1<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send ACK after reception
               (0<<TWWC);                                 //  
      }else                    // Send NACK after next reception
      {
        TWCR = (1<<TWEN)|                                 // TWI Interface enabled
               (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag to read next byte
               (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // Send NACK after reception
               (0<<TWWC);                                 // 
      }    
      break; 
    case TWI_MRX_DATA_NACK:     // Data byte has been received and NACK tramsmitted
      TWI_buf[TWI_bufPtr] = TWDR;
      TWI_statusReg.lastTransOK = TRUE;                 // Set status bits to completed successfully. 
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (0<<TWIE)|(1<<TWINT)|                      // Disable TWI Interrupt and clear the flag
             (0<<TWEA)|(0<<TWSTA)|(1<<TWSTO)|           // Initiate a STOP condition.
             (0<<TWWC);                                 //
      break;      
    case TWI_ARB_LOST:          // Arbitration lost
      TWCR = (1<<TWEN)|                                 // TWI Interface enabled
             (1<<TWIE)|(1<<TWINT)|                      // Enable TWI Interupt and clear the flag
             (0<<TWEA)|(1<<TWSTA)|(0<<TWSTO)|           // Initiate a (RE)START condition.
             (0<<TWWC);                                 //
      break;
    case TWI_MTX_ADR_NACK:      // SLA+W has been tramsmitted and NACK received
    case TWI_MRX_ADR_NACK:      // SLA+R has been tramsmitted and NACK received    
    case TWI_MTX_DATA_NACK:     // Data byte has been tramsmitted and NACK received
//    case TWI_NO_STATE              // No relevant state information available; TWINT = �0�
    case TWI_BUS_ERROR:         // Bus error due to an illegal START or STOP condition
    default:     
      TWI_state = TWSR;                                 // Store TWSR and automatically sets clears noErrors bit.
                                                        // Reset TWI Interface
      TWCR = (1<<TWEN)|                                 // Enable TWI-interface and release TWI pins
             (0<<TWIE)|(0<<TWINT)|                      // Disable Interupt
             (0<<TWEA)|(0<<TWSTA)|(0<<TWSTO)|           // No Signal requests
             (0<<TWWC);                                 //
  }
}