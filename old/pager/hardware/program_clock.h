#ifndef PROGRAM_CLOCK_H_INCLUDED
#define PROGRAM_CLOCK_H_INCLUDED

#define CLOCK_TCCRA      TCCR1A
#define CLOCK_TCCRA_DEF  0x00
#define CLOCK_TCCRB      TCCR1B
#define CLOCK_TCCRB_DEF  ( (1<<WGM12) | (1<<CS12) | (1<<CS10) )
#define CLOCK_COMP_REG   OCR1A
#define CLOCK_COMP_DEF   F_CPU/1024/2


void clock_init(void);


#endif // PROGRAM_CLOCK_H_INCLUDED
