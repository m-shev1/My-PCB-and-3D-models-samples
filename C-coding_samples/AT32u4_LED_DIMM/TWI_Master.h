/*
 * IncFile1.h
 *
 * Created: 6/20/2015 12:50:55 AM
 *  Author: mike
 */ 


/****************************************************************************
  TWI Status/Control register definitions
****************************************************************************/
#define TWI_BUFFER_SIZE 21  // Set this to the largest message size that will be sent including address byte.

#define TWI_TWBR            0x0C        // TWI Bit rate Register setting.
                                        // Se Application note for detailed 
                                        // information on setting this value.
// Not used defines!
//#define TWI_TWPS          0x00        // This driver presumes prescaler = 00

/****************************************************************************
  Global definitions
****************************************************************************/

union TWI_statusReg                       // Status byte holding flags.
{
    unsigned char all;
    struct
    {
        unsigned char lastTransOK:1;      
        unsigned char unusedBits:7;
    };
};

extern union TWI_statusReg TWI_statusReg;
//
union DS3231_Reg
{
	unsigned char all[19];
	struct
	{
		unsigned char sec;			// 0 - BCD
		int hour_min;				// 1,2 - BCD
		unsigned char day;			// 3 - BCD
		unsigned char date;			// 4 - BCD
		unsigned char month;		// 5-0 BCD(5:7-century)
		unsigned char year;			// 6 - BCD
		unsigned char AL1_sec;		// 7 - BCD(7:7-A1M1)
		int AL1_hour_min;			// 8,9 - BCD(8:7-A1M2,9:7-A1M3)
		unsigned char AL1_day_date;	// 10 - BCD(10:7-A1M4)
		int AL2_hour_min;			// 11,12 - BCD(8:7-A2M2,9:7-A2M3)
		unsigned char AL2_day_date;	// 13 - BCD(13:7-A2M4)
		unsigned char control;		// 14 - EOSC,BBSQW,CONV,RS2,RS1,INTCN,A2IE,A1IE
		unsigned char status;		// 15 - OSF,0,0,0,EN32kHz,BSY,A2F,A1F
		unsigned char t_align_offset; // 16
		unsigned char temp_H;		// 17 - Temp MSB
		unsigned char temp_L;		// 18 - Temp LSB
		
	};
	
};
extern union DS3231_Reg DS3231_Reg;

/****************************************************************************
  Function definitions
****************************************************************************/
void TWI_Master_Initialise( void );
unsigned char TWI_Transceiver_Busy( void );
unsigned char TWI_Get_State_Info( void );
void TWI_Start_Transceiver_With_Data( unsigned char * , unsigned char );
void TWI_Start_Transceiver( void );
unsigned char TWI_Get_Data_From_Transceiver( unsigned char *, unsigned char );
unsigned char DS3231_Reg_Read( unsigned char); // if the function's argument is > 0x12, then whole registers read to DS3231_Reg[]
unsigned char DS3231_Init( void );
unsigned char DS3231_Reg_Reco( void);
unsigned char DS3231_Check_IntF( void);
unsigned char DS3231_Reg_Write(unsigned char, unsigned char);
/****************************************************************************
  Bit and byte definitions
****************************************************************************/
#define TWI_READ_BIT  0       // Bit position for R/W bit in "address byte".
#define TWI_ADR_BITS  1       // Bit position for LSB of the slave address bits in the init byte.

#define TRUE          1
#define FALSE         0

#define ALARM_1		1
#define ALARM_2		2
#define BOTH_AL		3

/****************************************************************************
  TWI State codes
****************************************************************************/
// General TWI Master staus codes                      
#define TWI_START                  0x08  // START has been transmitted  
#define TWI_REP_START              0x10  // Repeated START has been transmitted
#define TWI_ARB_LOST               0x38  // Arbitration lost

// TWI Master Transmitter staus codes                      
#define TWI_MTX_ADR_ACK            0x18  // SLA+W has been tramsmitted and ACK received
#define TWI_MTX_ADR_NACK           0x20  // SLA+W has been tramsmitted and NACK received 
#define TWI_MTX_DATA_ACK           0x28  // Data byte has been tramsmitted and ACK received
#define TWI_MTX_DATA_NACK          0x30  // Data byte has been tramsmitted and NACK received 

// TWI Master Receiver staus codes  
#define TWI_MRX_ADR_ACK            0x40  // SLA+R has been tramsmitted and ACK received
#define TWI_MRX_ADR_NACK           0x48  // SLA+R has been tramsmitted and NACK received
#define TWI_MRX_DATA_ACK           0x50  // Data byte has been received and ACK tramsmitted
#define TWI_MRX_DATA_NACK          0x58  // Data byte has been received and NACK tramsmitted

// TWI Slave Transmitter staus codes
#define TWI_STX_ADR_ACK            0xA8  // Own SLA+R has been received; ACK has been returned
#define TWI_STX_ADR_ACK_M_ARB_LOST 0xB0  // Arbitration lost in SLA+R/W as Master; own SLA+R has been received; ACK has been returned
#define TWI_STX_DATA_ACK           0xB8  // Data byte in TWDR has been transmitted; ACK has been received
#define TWI_STX_DATA_NACK          0xC0  // Data byte in TWDR has been transmitted; NOT ACK has been received
#define TWI_STX_DATA_ACK_LAST_BYTE 0xC8  // Last data byte in TWDR has been transmitted (TWEA = “0”); ACK has been received

// TWI Slave Receiver staus codes
#define TWI_SRX_ADR_ACK            0x60  // Own SLA+W has been received ACK has been returned
#define TWI_SRX_ADR_ACK_M_ARB_LOST 0x68  // Arbitration lost in SLA+R/W as Master; own SLA+W has been received; ACK has been returned
#define TWI_SRX_GEN_ACK            0x70  // General call address has been received; ACK has been returned
#define TWI_SRX_GEN_ACK_M_ARB_LOST 0x78  // Arbitration lost in SLA+R/W as Master; General call address has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_ACK       0x80  // Previously addressed with own SLA+W; data has been received; ACK has been returned
#define TWI_SRX_ADR_DATA_NACK      0x88  // Previously addressed with own SLA+W; data has been received; NOT ACK has been returned
#define TWI_SRX_GEN_DATA_ACK       0x90  // Previously addressed with general call; data has been received; ACK has been returned
#define TWI_SRX_GEN_DATA_NACK      0x98  // Previously addressed with general call; data has been received; NOT ACK has been returned
#define TWI_SRX_STOP_RESTART       0xA0  // A STOP condition or repeated START condition has been received while still addressed as Slave

// TWI Miscellaneous status codes
#define TWI_NO_STATE               0xF8  // No relevant state information available; TWINT = “0”
#define TWI_BUS_ERROR              0x00  // Bus error due to an illegal START or STOP condition
