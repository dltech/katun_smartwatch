/*
 * Part of Katun PB-20 alternative and working firmware.
 * EEPROM 24LC512 library with programm I2C
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

#include <inttypes.h>
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#include "24lc512.h"


void seep_i2c_init(void)
{
    SEEP_I2C_DDR |= (1<<SEEP_SCL) | (1<<SEEP_SDA);
    SEEP_I2C_PORT |= (1<<SEEP_SCL) | (1<<SEEP_SDA);
}


void seep_i2c_start(void)
{
    _delay_us(SEEP_I2C_TIME4);
    SEEP_I2C_PORT &= ~(1<<SEEP_SDA);
    _delay_us(SEEP_I2C_TIME4);
    SEEP_I2C_PORT &= ~(1<<SEEP_SCL);
    _delay_us(SEEP_I2C_TIME2);
}


void seep_i2c_stop(void)
{
    SEEP_I2C_PORT &= ~(1<<SEEP_SDA);
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT |= (1<<SEEP_SCL);
    _delay_us(SEEP_I2C_TIME4);
    SEEP_I2C_PORT |= (1<<SEEP_SDA);
    _delay_us(SEEP_I2C_TIME4);
}


void seep_i2c_write_ack(void)
{
    SEEP_I2C_DDR |= (1<<SEEP_SDA);
    SEEP_I2C_PORT &= ~(1<<SEEP_SDA);

    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT |= (1<<SEEP_SCL);
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT &= ~(1<<SEEP_SCL);
    SEEP_I2C_PORT |= (1<<SEEP_SDA);
}


uint8_t seep_i2c_read_ack(void)
{
    uint8_t ack;

    SEEP_I2C_DDR &= ~(1<<SEEP_SDA);
    SEEP_I2C_PORT |= (1<<SEEP_SDA);
    _delay_us(SEEP_I2C_TIME2);

    SEEP_I2C_PORT |= (1<<SEEP_SCL);

    _delay_us(SEEP_I2C_TIME4);
    ack = SEEP_I2C_PIN & (1<<SEEP_SDA);
    _delay_us(SEEP_I2C_TIME4);

    SEEP_I2C_PORT &= ~(1<<SEEP_SCL);
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_DDR |= (1<<SEEP_SDA);

    if (ack)
        return 1;
    else
        return 0;
}



uint8_t seep_init(void)
{
    uint8_t ack = 0;

    seep_i2c_init();

    if ( seep_read_byte(0x0002) != 0xed )
    {
        seep_wait();
        ack += seep_write_byte(0x0000, 0x00);
        seep_wait();
        ack += seep_write_byte(0x0001, 0x08);
        seep_wait();
        ack += seep_write_byte(0x0002, 0xed);
        seep_wait();
    }
    return ack;
}


/* ???????????????????? ???????? ???????? ???? I2C. */

void seep_i2c_write_byte( uint8_t byte )
{
    uint8_t mask = 0x80;

    while ( mask )
    {
        if (mask & byte)    SEEP_I2C_PORT |= (1<<SEEP_SDA);
        else                SEEP_I2C_PORT &= ~(1<<SEEP_SDA);
        mask >>= 1;

        _delay_us(SEEP_I2C_TIME2);
        SEEP_I2C_PORT |= (1<<SEEP_SCL);
        _delay_us(SEEP_I2C_TIME2);
        SEEP_I2C_PORT &= ~(1<<SEEP_SCL);
    }
}


/* ?????????????? ???????????? ?????????? ???? I2C, ???????????????????? ???????????????????? ???????? */

uint8_t seep_i2c_read_byte(void)
{
    uint8_t byte = 0, mask = 0x80;

    SEEP_I2C_DDR &= ~(1<<SEEP_SDA);

    while ( mask )
    {
        _delay_us(SEEP_I2C_TIME2);
        SEEP_I2C_PORT |= (1<<SEEP_SCL);
        _delay_us(SEEP_I2C_TIME4);

        if (SEEP_I2C_PIN & (1<<SEEP_SDA))
            byte |= mask;
        mask >>= 1;

        _delay_us(SEEP_I2C_TIME4);
        SEEP_I2C_PORT &= ~(1<<SEEP_SCL);
    }
    SEEP_I2C_DDR |= (1<<SEEP_SDA);

    return byte;
}


/* ???????????????? ???????? eeprom ???? ?????????? ???????????? ?????????????? ????????, ?????? ???????? ?????????????? ???? ?????????????? */
void seep_wait(void)
{
    uint16_t timeout = 0;

    while( (seep_read_byte(0x0002) == 0) && (timeout < SEEP_TIMEOUT) )
        ++timeout;
}


/* ???????????? ???????????????????? ???????????? ?????????? ???? I2C. ???????????????????? ???????????????? > 0 ???????? ?????? ??????????. */

uint8_t seep_write_byte( uint16_t address, uint8_t data )
{
    uint8_t ack = 0;

    seep_i2c_start();

    seep_i2c_write_byte(SEEP_ADDRESS_W);
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)(address>>8));
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)address);
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte(data);
    ack += seep_i2c_read_ack();

    seep_i2c_stop();

    return ack;
}


/* ???????????? ???????????????????? ???????? ???? I2C. ???????????????????? ???????????????? > 0 ???????? ?????? ??????????. */

uint8_t seep_write( uint16_t address, uint8_t *data, uint8_t ndata )
{
    uint8_t i, ack = 0;

    seep_i2c_start();

    seep_i2c_write_byte(SEEP_ADDRESS_W);
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)(address>>8));
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)address);
    ack += seep_i2c_read_ack();

    for (i=0 ; i<ndata ; ++i)
    {
        seep_i2c_write_byte(data[i]);
        ack += seep_i2c_read_ack();
    }

    seep_i2c_stop();

    return ack;
}


/* ???????????? ???????????? ?????????? ???? I2C. ???????????????????? ?????????????????? ??????????, ???????? ??????????????, ?????????? ???????????????????? 0 */

uint8_t seep_read_byte( uint16_t address )
{
    uint8_t ack = 0, data;

    seep_i2c_start();

    seep_i2c_write_byte(SEEP_ADDRESS_W);
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)(address>>8));
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)address);
    ack += seep_i2c_read_ack();
    SEEP_I2C_PORT |= (1<<SEEP_SCL);
    seep_i2c_start();

    seep_i2c_write_byte(SEEP_ADDRESS_R);
    seep_i2c_write_ack();

    data = seep_i2c_read_byte();
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT |= (1<<SEEP_SDA) | (1<<SEEP_SCL);
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT &= ~(1<<SEEP_SCL);

    seep_i2c_stop();

    if ( ack > 0 )
        return 0;
    else
        return data;
}


/* ???????????? ???????????????????? ???????? ???????????? ???? I2C */

uint8_t seep_read( uint16_t address, uint8_t *data, uint8_t ndata )
{
    uint8_t i, ack = 0;

    seep_i2c_start();

    seep_i2c_write_byte(SEEP_ADDRESS_W);
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)(address>>8));
    ack += seep_i2c_read_ack();

    seep_i2c_write_byte((uint8_t)address);
    ack += seep_i2c_read_ack();
    SEEP_I2C_PORT |= (1<<SEEP_SCL);
    seep_i2c_start();

    seep_i2c_write_byte(SEEP_ADDRESS_R);
    seep_i2c_write_ack();

    --ndata;
    for (i=0 ; i<ndata ; ++i)
    {
        data[i] = seep_i2c_read_byte();
        seep_i2c_write_ack();
    }

    data[ndata] = seep_i2c_read_byte();
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT |= (1<<SEEP_SDA) | (1<<SEEP_SCL);
    _delay_us(SEEP_I2C_TIME2);
    SEEP_I2C_PORT &= ~(1<<SEEP_SCL);

    seep_i2c_stop();

    return ack;
}
