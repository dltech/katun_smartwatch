#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "rf12.h"



char buff[16];

volatile unsigned char RF12_Index = 0;
unsigned char RF12_Data[RF12_DataLength+10];	// +10 == paket overhead


uint16_t crc16(volatile uint8_t *pcBlock, uint8_t len)
{
    uint16_t crc = 0xFFFF;
    uint8_t i;

    while (len--)
    {
        crc ^= *pcBlock++ << 8;

        for (i = 0; i < 8; i++)
            crc = crc & 0x8000 ? (crc << 1) ^ 0x1021 : crc << 1;
    }

    return crc;
}


ISR(INT2_vect)
{
	if(RF12_status.Rx)
	{
		if(RF12_Index < RF12_DataLength)
			RF12_Data[RF12_Index++] = rf12_trans(0xB000) & 0x00FF;
		else
		{
			rf12_trans(0x8208);
			RF12_status.Rx = 0;
		}
		if(RF12_Index >= RF12_Data[0] + 3)		//EOT
		{
			rf12_trans(0x8208);
			RF12_status.Rx = 0;
			RF12_status.New = 1;
		}
	}
	else if(RF12_status.Tx)
	{
		rf12_trans(0xB800 | RF12_Data[RF12_Index]);
		if(!RF12_Index)
		{
			RF12_status.Tx = 0;
			rf12_trans(0x8208);		// TX off
		}
		else
		{
			RF12_Index--;
		}
	}
	else
	{
		rf12_trans(0x0000);			//dummy read
							//TODO: what happend
	}
}

void rf12_sleep(void)
{
rf12_trans(0x8201);
}

/* програмный SPI с ассемблерными вставками - это нечто */
unsigned short rf12_trans(unsigned short wert)
{
	unsigned short werti = 0;
	unsigned char i;

	cbi(RF_PORT, CS);			// обнулить бит в регистре
	for (i=0; i<16; i++)
	{
		if (wert&0x8000)
			sbi(RF_PORT, SDI);	// установить бит в регистре
		else
			cbi(RF_PORT, SDI);
		werti<<=1;
		if (RF_PIN&(1<<SDO))
			werti|=1;
		sbi(RF_PORT, SCK);
		wert<<=1;
		asm("nop");
		cbi(RF_PORT, SCK);
	}
	sbi(RF_PORT, CS);
	return werti;
}

void rf12_init(void)
{
	unsigned char i;

	RF_DDR |= (1<<SDI)|(1<<SCK)|(1<<CS);
	RF_DDR &= ~(1<<SDO);
	RF_PORT |= (1<<CS);

	for (i=0; i<10; i++)
		_delay_ms(10);			// wait until POR done

	rf12_trans(0xC040);			// AVR CLK: 10MHz
	rf12_trans(0x80D7);			// Enable FIFO
	rf12_trans(0xC2AB);			// Data Filter: internal
	rf12_trans(0xCA81);			// Set FIFO mode
	rf12_trans(0xE000);			// disable wakeuptimer
	rf12_trans(0xC800);			// disable low duty cycle
	rf12_trans(0xC4F7);			// AFC settings: autotuning: -10kHz...+7,5kHz

	rf12_trans(0x0000);

	RF12_status.Rx = 0;
	RF12_status.Tx = 0;
	RF12_status.New = 0;

	RF_IRQDDR &= ~(1<<IRQ);
	GICR |= (1<<INT2);
}

void rf12_setbandwidth(unsigned char bandwidth, unsigned char gain, unsigned char drssi)
{
	rf12_trans(0x9400|((bandwidth&7)<<5)|((gain&3)<<3)|(drssi&7));
}

void rf12_setfreq(unsigned short freq)
{	if (freq<96)				// 430,2400MHz
		freq=96;
	else if (freq>3903)			// 439,7575MHz
		freq=3903;
	rf12_trans(0xA000|freq);
}

void rf12_setbaud(unsigned short baud)
{
	if (baud<663)
		return;
	if (baud<5400)					// Baudrate= 344827,58621/(R+1)/(1+CS*7)
		rf12_trans(0xC680|((43104/baud)-1));
	else
		rf12_trans(0xC600|((344828UL/baud)-1));
}

void rf12_setpower(unsigned char power, unsigned char mod)
{
	rf12_trans(0x9800|(power&0x07)|((mod&0x0f)<<4));
}


/* none blocking methods */
unsigned char rf12_rxstart(void)
{
	if(RF12_status.New)
		return(1);			//buffer not yet empty
	if(RF12_status.Tx)
		return(2);			//tx in action
	if(RF12_status.Rx)
		return(3);			//rx already in action

	rf12_trans(0x82C8);			// RX on
	rf12_trans(0xCA81);			// set FIFO mode
	rf12_trans(0xCA83);			// enable FIFO

	RF12_Index = 0;
	RF12_status.Rx = 1;

	return(0);				//all went fine
}


/* Функция проверки окончания приема пакета. Возвращает 255 если передача не окончена, 254 если передача окончена, но пакет не принят, 0 если ошибка crc, и размер пакет если он успешно принят */

unsigned char rf12_rxfinish(volatile unsigned char *data)
{
	uint16_t crc, crc_chk = 0;
	unsigned char i;
	if(RF12_status.Rx)
        return(255);				//not finished yet
	if(!RF12_status.New)
		return(254);				//old buffer
	for(i=0; i<RF12_Data[0]-2; i++)
			data[i] = RF12_Data[i+1];
	crc_chk = crc16(data, RF12_Data[0]-2);
	crc = RF12_Data[RF12_Data[0]-1];
	crc |= RF12_Data[RF12_Data[0]] << 8;
	RF12_status.New = 0;
	if(crc != crc_chk)
		return(0);
	else
	{
		return(RF12_Data[0]-2);			//strsize
	}
}

unsigned char rf12_txstart(volatile unsigned char *data, unsigned char size)
{
	unsigned char i, l;
	uint16_t crc;
	if(RF12_status.Tx)
		return(2);			//tx in action
	if(RF12_status.Rx)
		return(3);			//rx already in action
	if(size > RF12_DataLength)
		return(4);			//str to big to transmit

	RF12_status.Tx = 1;
	RF12_Index = size + 9;			//act -10

	i = RF12_Index;
	RF12_Data[i--] = 0xAA;
	RF12_Data[i--] = 0xAA;
	RF12_Data[i--] = 0xAA;
	RF12_Data[i--] = 0x2D;
	RF12_Data[i--] = 0xD4;
	RF12_Data[i--] = size+2;
	for(l=0; l<size; l++)
	{
		RF12_Data[i--] = data[l];
	}
	crc = crc16(data, size);
	RF12_Data[i--] = (crc & 0x00FF);
	RF12_Data[i--] = (crc >> 8);
	RF12_Data[i--] = 0xAA;
	RF12_Data[i--] = 0xAA;

	rf12_trans(0x8238);			// TX on

	return(0);				//all went fine
}

unsigned char rf12_txfinished(void)
{
	if(RF12_status.Tx)
		return(255);			//not yet finished
	return(0);
}

void rf12_allstop(void)
{
	RF12_status.Rx = 0;
	RF12_status.Tx = 0;
	RF12_status.New = 0;
	rf12_trans(0x8208);		//shutdown all
	rf12_trans(0x0000);		//dummy read
}
/* ---------------------- */
