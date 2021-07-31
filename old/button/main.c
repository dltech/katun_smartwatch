#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <avr/sleep.h>
#include <avr/eeprom.h>
#include "rfm12.h"


#define LED_ON PORTD |= (1<<6)
#define LED_OFF PORTD &= ~(1<<6)

#define NSENDS  10

#define BUTTON  0x50
#define ID_GET  0x49
#define CALL    0x01
#define CANCEL  0x02
#define ACKN    0x03

#define LED_TIME    0.5
#define LED_NFLASH_ERR 4
#define LED_NFLASH_OK  6
#define LED_NFLASH_ID  8

#define LED_COUNT   LED_TIME*1024.*65536./F_CPU

#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))

uint8_t             i;
volatile uint16_t   led_counter, wait_time, timeout;
volatile uint8_t    led_flash, id=0, S = 0;
extern volatile uint8_t pack_buffer[PACKET_SIZE], rfm_condition;

uint8_t     id_eep          EEPROM_SECTION  = 0x01;
uint8_t     channel_eep     EEPROM_SECTION  = 0x01;


ISR(INT0_vect)
{
	GIMSK &= ~(1<<INT0);
	_delay_ms(1);
	if ((PIND & (1 << PD4)) == 0)
	{
		S=1;
	}
	if ((PIND & (1 << PD5)) == 0)
	{
		S=2;
	}
	GIMSK |= (1<<INT0);
}

/*
ISR(TIMER1_OVF_vect)
{
    if(led_counter < LED_COUNT)
    {
        ++led_counter;
    }
    else
    {
        led_counter = 0;

        if( (--led_flash) == 0 )
        {
            TIMSK &= ~(1<<TOIE1);
            LED_OFF;
        }
        else
        {
            if(led_flash & 0x01)    LED_OFF;
            else                    LED_ON;
        }
    }
}
*/


void transmit_command(uint8_t command)
{
    pack_buffer[HEADER_NBYTES] = BUTTON;
    pack_buffer[HEADER_NBYTES+1] = command;
    pack_buffer[HEADER_NBYTES+2] = id;
    rfm12_transmit_pack(3);
    while(rfm_condition == TRANSMIT_PACK);
}



int main(void)
{
	DDRD = 0b01000000;
	PORTD = 0b00110100;

	TCCR0B = 0x02;          // вкл. таймер-счетчик 0, делитель 8
//    TCCR1B = 0x02;          // вкл. таймер-счетчик 1, делитель 8

	rfm12_init();					// инициализация RFM
	rfm12_deep_sleep();

    id = eeprom_read_byte(&id_eep);
    rfm12_channel(eeprom_read_byte(&channel_eep));

	sei();

/// Основной цикл, отслеживает состояния кнопок, если нажата какая либо, отправляет вызов или отмену.
    while(1)
    {
        GIMSK |= (1<<INT0);
        sleep_enable();
        set_sleep_mode(SLEEP_MODE_PWR_DOWN);
        sleep_mode();
        _delay_ms(10);                           // хрен знает зачем

        GIMSK &= ~(1<<INT0);
        wait_time = TCNT0;
        wait_time = (wait_time | 0x03) & 0x0f;

        for( i=0 ; i<NSENDS ; ++i )
        {
            if(S==1)        transmit_command(CALL);
            else if(S==2)   transmit_command(CANCEL);

            rfm12_wait_pack();

            timeout = 0;
            TCNT1=0;
            while( (rfm_condition == RECEIVE_PACK) && (timeout < wait_time) )
            {
//                while( (TIFR & (1<<TOV1)) == 0 );
//                TIFR |= (1<<TOV1);               // обнуляется флаг
                _delay_ms(20);
                ++timeout;
            }

            if( (rfm_condition == NEW_PACK) && (pack_buffer[0] == BUTTON) )             // если какойто пакет принят
            {
                if(pack_buffer[1] == ID_GET)
                {
                    id = pack_buffer[2];
                    rfm12_channel(pack_buffer[3]);
                    i = 0xee;

                    eeprom_write_byte(&id_eep, pack_buffer[2]);
                    eeprom_write_byte(&channel_eep, pack_buffer[3]);
                    break;
                }
                else if( (pack_buffer[1] == ACKN) | (pack_buffer[2] == id) )
                    break;
            }
        }

        rfm12_deep_sleep();
        S = 0;

        led_counter = 0;
        if (i < (NSENDS-1) )    led_flash = LED_NFLASH_OK;
        else if (i == 0xee)     led_flash = LED_NFLASH_ID;
        else                    led_flash = LED_NFLASH_ERR;

        for( ; led_flash>0 ; --led_flash)
        {
            if(led_flash & 0x01)    LED_OFF;
            else                    LED_ON;
            _delay_ms(200);
        }

        LED_OFF;

        _delay_ms(50);                          // защита от дребезга
    }
}

