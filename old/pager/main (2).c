#include "globals.h"
#include <avr/eeprom.h>
#include <util/atomic.h>

#include "messages.h"
#include "menu.h"
#include "menuitems.h"
#include "event.h"

#include "hardware/keyboard.h"
#include "hardware/rfm22.h"
#include "hardware/lcd.h"
#include "hardware/program_clock.h"

#define COMMAND_TX_TIME     TIMER_MSTOCYCLES(9)
#define RX_WAIT_TIME        TIMER_MSTOCYCLES(30)
#define SECOND_TIME         TIMER_MSTOCYCLES(1000)
#define EVENT_SYNCH_TIME    TIMER_MSTOCYCLES(5000)
#define REPEAT_TIME         TIMER_MSTOCYCLES(100)

#define BAT_THRES_45        BAT_VOLT(2.9)
#define BAT_THRES_35        BAT_VOLT(2.7)
#define BAT_THRES_25        BAT_VOLT(2.5)
#define BAT_THRES_15        BAT_VOLT(2.3)


#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))

 uint8_t     snd_on_eep          EEPROM_SECTION  = 0;
 uint16_t    snd_duration_eep    EEPROM_SECTION  = 160;
 uint8_t     light_on_eep        EEPROM_SECTION  = 1;
 uint16_t    light_duration_eep  EEPROM_SECTION  = 1600;
 uint8_t     channel_eep         EEPROM_SECTION  = 0x01;

char lcd_buffer[9];
volatile uint8_t channel, status1, status2, snd_on, light_on, req_rep_counter = 0, menu_x, nevent, active_on=0, rf_on=1, psw_on=0;
volatile uint16_t snd_duration, light_duration;
// hardware/rfm22.c
extern volatile uint8_t pack_buffer[BUFFER_SIZE_MAX], pack_size;
// hardware/program_clock.c
extern volatile uint8_t time[7];
// event.c
extern volatile uint8_t active_call_id[NCALLS_MAX], n_active_calls;


uint8_t clock(msg_par par);
uint8_t send_request(msg_par par);
uint8_t tx_to_rx(msg_par par);
uint8_t packet_handler(msg_par par);
void voltage_measure(void);
//uint8_t beep_repeat(msg_par par);
uint8_t beep(msg_par par);
uint8_t light(msg_par par);
uint8_t light_timer(msg_par par);


int main(void)
{
/// Инициализация железа.
    lcd_init(LCD_DISP_ON);
	rfm22_init();
	clock_init();

    DDRC |= (1<<PC4) | (1<<PC3); //speaker and light

/// Считывание необходимых данных из eeprom.
    snd_on = eeprom_read_byte(&snd_on_eep);
    snd_duration = eeprom_read_word(&snd_duration_eep);
    light_on = eeprom_read_byte(&light_on_eep);
    light_duration = eeprom_read_word(&light_duration_eep);
    channel = eeprom_read_byte(&channel_eep);

    rfm22_write(FREQ_HOP_CHSEL_P, channel-1);
    status1 = 'x';
    status2 = 0x9d;

/// Инициализация диспетчера задач.
	initMessages();
    KBD_init();

	setHandler(MSG_MENU_SELECT, &selectMenu);
	startMenuNull();

    setHandler(MSG_BEEP, &beep);
    setTimer(MSG_BEEP, 2*snd_on, snd_duration);

	setHandler(MSG_LIGHT, &light);
	setHandler(MSG_LIGHT_OFF, &light_timer);
    sendMessage(MSG_LIGHT, 0);

    setHandler(MSG_CLOCK_DISP_REF, &clock);
    setTimer(MSG_CLOCK_DISP_REF, 0, SECOND_TIME);

    setHandler(MSG_SEND_REQ, &send_request);
    setTimer(MSG_SEND_REQ, EVENT, EVENT_SYNCH_TIME);
    sendMessage(MSG_SEND_REQ, CLOCK);

    setHandler(MSG_PACK_HANDLER, &packet_handler);
    setHandler(MSG_TXRX, &tx_to_rx);

    display_call();

    sei();

	for(;;) {
		dispatchMessage();
	}
}



uint8_t clock(msg_par par)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        if(time[1] == 0)    sendMessage(MSG_SEND_REQ, CLOCK);       // синхронизация времени раз в час
        voltage_measure();

        sprintf(lcd_buffer, "%02d %02d ", time[2], time[1]);
        lcd_gotoxy(0,0);
        lcd_puts(lcd_buffer);
        lcd_putc(status1);
        lcd_putc(status2);

        lcd_gotoxy(2,0);
        if(time[0]&0x01)    lcd_putc(' ');
        else                lcd_putc(':');
    }
    return 1;
}


void voltage_measure(void)
{
    uint8_t voltage = 0x1f & rfm22_read(BATTERY_VOLTAGE_P);

    if(voltage>3)
    {
        if(voltage > BAT_THRES_45)
            status2 = 0x9b;
        else if(voltage > BAT_THRES_35)
            status2 = 0x9c;
        else if(voltage > BAT_THRES_25)
            status2 = 0x9d;
        else if(voltage > BAT_THRES_15)
            status2 = 0x9e;
        else
            status2 = 0x9f;
    }
}


uint8_t send_request(msg_par par)
{
    static uint8_t req_rep_counter_local;

    if(!rf_on) {
        status1 = '!';
        return 0;
    }

    if( (req_rep_counter > 0) && (status1 != 'x') ) {
        setTimer(MSG_SEND_REQ, par, REPEAT_TIME);
        status1 = 'x';
    }
    if( (req_rep_counter == 7) && (snd_on > 0) )    setTimer(MSG_BEEP, 2, snd_duration);

    if( (req_rep_counter == 0) && (status1 != 0x00) ) {
        setTimer(MSG_SEND_REQ, EVENT, EVENT_SYNCH_TIME);
        status1 = 0x00;
        if( (snd_on>0) && (req_rep_counter_local >= 7) )  setTimer(MSG_BEEP, 2, snd_duration);
    }
    req_rep_counter_local = req_rep_counter;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        pack_buffer[0] = PAGER;
        pack_buffer[1] = par;

        rfm22_transmit_start(2);
    }
    setTimer(MSG_TXRX, par, COMMAND_TX_TIME);

    return 0;
}


uint8_t tx_to_rx(msg_par par)
{
    killTimer(MSG_TXRX);
    setTimer(MSG_PACK_HANDLER, par, RX_WAIT_TIME);

    rfm22_transmit_complete();
    rfm22_receive_start();

    return 0;
}


uint8_t packet_handler(msg_par par)
{
    uint8_t i;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        killTimer(MSG_PACK_HANDLER);

        if( rfm22_receive_complete() ) {                                // если пакет не пакет
            if(req_rep_counter <= 30)   ++req_rep_counter;
            return 1;                                                   // разворачиваемся и уходим
        }
        if(pack_buffer[0] != PAGER)     return 2;                       // если пакет не наш, опять уходим

        req_rep_counter = 0;

/// Собственно, обработчик пакета
        switch(pack_buffer[1])
        {
            case CLOCK :
            {
                for(i=0 ; i<7 ; ++i)   time[i] = pack_buffer[i+2];
                break;
            }
            case EVENT :
            {
                for(i=0 ; ( (i<N_ACTIVE_CALL_MAX) && (i<(pack_size-2)) ) ; ++i)   active_call_id[i] = pack_buffer[i+2];    // копирует информацию об активных вызовах в буфер пейджера

                if(n_active_calls != (pack_size-2))                                         // если количество вызовов изменилось
                {
                    if(snd_on)                                                              // звуковая игнализация
                    {
                        if(n_active_calls > (pack_size-2))
                            setTimer(MSG_BEEP, 2, snd_duration);
                        else
                            setTimer(MSG_BEEP, 6, snd_duration);
                    }
                    if(light_on)   sendMessage(MSG_LIGHT, 0);                               // подсветка

                    n_active_calls = pack_size - 2;
                    if(!active_on)
                    {
                        if(n_active_calls)
                        {
                            stopMenu();
                            startMenuNull();
                            display_call();
                        }
                        else if( checkMenu() )   display_call();
                    }
                }
                if(active_on)   sendMessage(MSG_KEY_PRESS, 0);
                break;
            }
        }
    }

    return 0;
}

/*
uint8_t beep_repeat(msg_par par)
{
    killTimer(MSG_BEEP);
    setTimer(MSG_BEEP, par, snd_duration);
    return 0;
}
*/

uint8_t beep(msg_par par)
{
	static uint8_t i;

	PORTC ^= (1<<PC4);
	i++;
	if(i>=par)
	{
		i=0;
        PORTC &= ~(1<<PC4);
        killTimer(MSG_BEEP);
	}
	return 1;
}


uint8_t light(msg_par par)
{
    PORTC |= (1<<PC3);
    setTimer(MSG_LIGHT_OFF, 0, light_duration);
    return 0;
}

uint8_t light_timer(msg_par par)
{
    PORTC &= ~(1<<PC3);
    killTimer(MSG_LIGHT_OFF);
    return 0;
}
