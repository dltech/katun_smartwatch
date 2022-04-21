#ifndef FTDI_H_INCLUDED
#define FTDI_H_INCLUDED
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

// USART initialization
// Communication Parameters: 8 Data, 1 Stop, No Parity
// USART Receiver: On
// USART Transmitter: On
// USART Mode: Asynchronous
// USART Baud Rate: 9600

#define UBRRH_DEF   0x00
#define UBRRL_DEF   0x67
#define UCSRA_DEF   0x00
#define UCSRB_DEF   0x18
#define UCSRC_DEF   0x86

// сигналы RTS и CTS  для FT232BM
#define FT_PORT PORTD
#define FT_DDR  DDRD
#define FT_PIN  PIND

#define CTS 2
#define RTS 3

//команды обмена данными с пк
#define SEND_DATA       240
#define CLEAN_DATA      249
#define CLEAN_DATA_OK1  250
#define CLEAN_DATA_OK2  251



void uart_init(void);
uint8_t uart_check(msg_par par);

void ft_send_data(void);
void ft_clean_data(void);
void ft_data_converter(void);





#endif // FTDI_H_INCLUDED
