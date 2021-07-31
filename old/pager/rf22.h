/*----------------------------------------------------------------------------
 Copyright:
 Author:
 Remarks:
 known Problems: none
 Version:        16.12.2009
 Description:    RF22 Funkmodule
------------------------------------------------------------------------------*/


//#define BAND_868

#define RF22_PORT PORTB
#define RF22_DDR  DDRB
#define RF22_PIN  PINB

#define CS		2
#define SDI		3
#define SDO		4
#define SCK		5

#define nIRQ	(PINB&(1<<0))

void rf22_init(void);											// initialize module
void rf22_setfreq(unsigned short freq);

void rf22_write(unsigned char addr, unsigned char data);
unsigned char rf22_read(unsigned char addr);

void rf22_rxmode(void);

unsigned char rf22_getpacket(unsigned char *data);
void rf22_sendpacket(unsigned char *data, unsigned char size);


#ifdef BAND_868
 #define RF22FREQ(freq)	((freq-860.0)*3200)
#else
 #define RF22FREQ(freq)	((freq-430.0)*6400)
#endif


