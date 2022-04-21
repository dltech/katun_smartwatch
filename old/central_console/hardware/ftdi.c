/*
 * Part of Katun PB-20 alternative and working firmware.
 * pc connecting functions using USB-COM HW FTDI interface
 *
 * Copyright 2010 Mikhail Belkin <dltech174@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "../globals.h"
#include "../messages.h"
#include "../event.h"

#include "24lc512.h"
#include "ftdi.h"
#include "rfm12.h"

extern volatile uint8_t status2;
extern volatile uint16_t seep_pointer;
volatile uint16_t uart_pointer;
volatile uint8_t ft_event_buffer[RECORD_SIZE*2], i;


void uart_init(void)
{
    UCSRA = UCSRA_DEF;
    UCSRB = UCSRB_DEF;
    UCSRC = UCSRC_DEF;
    UBRRH = UBRRH_DEF;
    UBRRL = UBRRL_DEF;

    FT_DDR &= ~((1<<CTS) | (1<<RTS));
    FT_PORT |= ((1<<CTS) | (1<<RTS));
}



ISR(USART_UDRE_vect)
{
    cli();
    UDR = ft_event_buffer[i++];

    if( i == (RECORD_SIZE*2) )
    {
        if (uart_pointer > RECORD_SIZE)     uart_pointer -= RECORD_SIZE;
        else {
            UCSRB &= ~(1<<UDRIE);
            setTimer(MSG_PACK_HANDLER, 0, 1);
            status2 = ' ';
        }
        ft_data_converter();
    }
    sei();
}



uint8_t uart_check(msg_par par)
{
    uint8_t data;

    if( UCSRA & (1<<RXC) )
    {
        data = UDR;
        switch (data)
        {
            case SEND_DATA :
            {
                if (seep_pointer <= RECORD_SIZE) return 2;
                killTimer(MSG_PACK_HANDLER);
                rfm12_sleep();
                status2 = 'S';

                uart_pointer = seep_pointer - RECORD_SIZE;
                ft_data_converter();

                UCSRB |= (1<<UDRIE);

                break;
            }

            case CLEAN_DATA :
            {
                seep_wait();
                seep_write_byte(0x0000, 0x00);
                seep_wait();
                seep_write_byte(0x0001, 0x08);

                seep_pointer = 0x0008;

                while ( !(UCSRA & (1<<UDRE)) );
                UDR = CLEAN_DATA_OK1;
                while ( !(UCSRA & (1<<UDRE)) );
                UDR = CLEAN_DATA_OK2;

                break;
            }
        }
    }
    return 1;
}



void ft_data_converter(void)
{
    seep_wait();
    seep_read(uart_pointer, (uint8_t *)ft_event_buffer+RECORD_SIZE, RECORD_SIZE);

    ft_event_buffer[0] = 0x00;
    ft_event_buffer[1] = 0x00;
    ft_event_buffer[2] = ft_event_buffer[9];
    ft_event_buffer[3] = 2 - ft_event_buffer[8];

    for (i = 5 ; i < 255 ; --i)  {
        ft_event_buffer[15-i-i-1] = ft_event_buffer[15-i] / 10;
        ft_event_buffer[15-i-i] = ft_event_buffer[15-i] % 10;
    }
    i = 0;
}
