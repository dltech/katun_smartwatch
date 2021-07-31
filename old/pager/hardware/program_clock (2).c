#include <avr/io.h>
#include <avr/interrupt.h>

#include "program_clock.h"


volatile uint8_t time[7];               // тут хранится время

void clock_init(void)
{
    CLOCK_TCCRA = CLOCK_TCCRA_DEF;      // инициализация таймера
    CLOCK_TCCRB = CLOCK_TCCRB_DEF;
    CLOCK_COMP_REG = CLOCK_COMP_DEF;

    TIMSK |= (1<<OCIE1A);               // включить прерывание по переполнению таймера

    time[0] = time[1] = time[2] = 0;    // внести начальное значение времени (1 января 2014, 00:00:00)
    time[3] = time[4] = time[5] = 1;
    time[6] = 14;
}


ISR(TIMER1_COMPA_vect)
{
    cli();

    if( (++time[0]) >= 60 )
    {
        time[0] = 0;
        if( (++time[1]) >= 60 )
        {
            time[1] = 0;
            if( (++time[2]) >= 24 )
            {
                time[2] = 0;
            }
        }
    }
    sei();
}
