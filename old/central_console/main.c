#include "globals.h"

#include "messages.h"
#include "menu.h"
#include "menuitems.h"
#include "event.h"

#include "hardware/keyboard.h"
#include "hardware/ftdi.h"
#include "hardware/rfm12.h"
#include "hardware/lcd.h"
#include "hardware/ds1307.h"
#include "hardware/24lc512.h"

#define PAGER_WAIT_TIME 7000
//0.000125
/*
#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))

uint16_t    dummy                       EEPROM_SECTION  = 0;       // avoid using lowest addresses
uint8_t     channel_eep                 EEPROM_SECTION  = 0x01;
uint16_t    nbuttons_eep                EEPROM_SECTION  = 0;
uint8_t     snd_on_eep                  EEPROM_SECTION  = 0;
uint16_t    snd_duration_eep            EEPROM_SECTION  = 800;
uint8_t     connected_button_id_eep[32] EEPROM_SECTION  = {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
*/

// hardware/rfm12.c
extern volatile uint8_t error_counter, rfm_condition, pack_counter, pack_buffer[PACKET_SIZE], pack_size;
extern volatile uint16_t rfm_status;
// event.c
volatile uint8_t  active_call_id[N_ACTIVE_CALL_MAX], n_active_calls;
volatile uint16_t nbuttons;

volatile uint8_t status1, status2, channel, snd_on, second_call, active_on = 0, id_send = 0, send_channel = 1, id=1, id_any=0, menu_id=1, psw_on=0;
volatile uint16_t seep_pointer, snd_duration, pager_counter;
char lcd_buffer[18];

uint8_t clock(msg_par par);
uint8_t beep(msg_par par);
uint8_t beep_repeat(msg_par par);
uint8_t packet_handler(msg_par par);
uint8_t rx_check(msg_par par);



int main(void)
{
    _delay_ms(600);

    uint8_t init_buffer[9] = {0, 0x00, 0xa0, 0, 0, 1, 0, 0x08, 0};     // начальные настройки

/// Инициализация железа.
    lcd_init(LCD_DISP_ON);
	ds_init(init_buffer, 9);
	seep_init();
	uart_init();
	rfm12_init();

	DDRB |= (1<<PB0);       // пищалка

/// Считывание необходимых данных из памяти часов.

    ds_read(0x09, init_buffer, 9);

    snd_on = init_buffer[0];
    snd_duration = (init_buffer[1] << 8) + init_buffer[2];
    nbuttons = (init_buffer[3] << 8) + init_buffer[4];
    channel = init_buffer[5];
    seep_pointer = (init_buffer[6] << 8) + init_buffer[7];
    second_call = init_buffer[8];

    rfm12_channel(channel);
/*
    snd_on = eeprom_read_byte(&snd_on_eep);
    snd_duration = eeprom_read_word(&snd_duration_eep);
    nbuttons = eeprom_read_word(&nbuttons_eep);
    channel = eeprom_read_byte(&channel_eep);
    seep_wait();
    seep_pointer = (seep_read_byte(0x0000)) << 8;  //adrh
    seep_pointer |= seep_read_byte(0x0001);        //adrl
*/
    status1 = status2 = ' ';

/// Инициализация диспетчера задач.
	initMessages();
    KBD_init();

	setHandler(MSG_PACK_HANDLER, &packet_handler);
	setTimer(MSG_PACK_HANDLER, 0, 1);

	setHandler(MSG_CLOCK_REF, &clock);          // включаются часы
	setTimer(MSG_CLOCK_REF, 0, 800);
    sendMessage(MSG_CLOCK_REF, 0);

    setHandler(MSG_UART_REF, &uart_check);      // проверка данных по UART
	setTimer(MSG_UART_REF, 0, 20);

    setHandler(MSG_BEEP_REP, &beep_repeat);
	setHandler(MSG_BEEP, &beep);

	setHandler(MSG_MENU_SELECT, &selectMenu);
	startMenuNull();

    setHandler(MSG_RXCHK, &rx_check);
    setTimer(MSG_RXCHK, 0, 500);

    setHandler(MSG_KILL_PSW, &kill_password);

    if ( (seep_pointer + RECORD_SIZE) >= (SEEP_MAX_POINTER) ) {
        lcd_gotoxy(0,1);
        lcd_puts("\250a\274\307\277\304 \267a\276o\273\275e\275a");
        status1 = '!';
    }

    sei();

	for(;;) {
		dispatchMessage();
	}
}


uint8_t clock(msg_par par)
{
	uint8_t time[7];
    char lcd_buffer[16];

	ds_read_time(time);

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        sprintf(lcd_buffer, "%02d.%02d %02d:%02d:%02d", time[4], time[5], time[2], time[1], time[0]);
        lcd_gotoxy(0,0);
        lcd_puts(lcd_buffer);
        lcd_putc(status1);
        lcd_putc(status2);
    }
    return 1;
}


uint8_t beep_repeat(msg_par par)
{
    setTimer(MSG_BEEP, par, snd_duration);
    return 0;
}


uint8_t beep(msg_par par)
{
	static uint8_t i=0;

	PORTB ^= (1<<PB0);
	i++;
	if(i>=par)
	{
		i=0;
        PORTB &= ~(1<<PB0);
        killTimer(MSG_BEEP);
	}
	return 1;
}


uint8_t rx_check(msg_par par)               // чистой воды костыль, кривой и с занозами
{
    rfm_status = rfm12_write(STATUS_READ);

    if( (rfm_status & ((1<<RGITFFIT) | (1<<RGURFFOV)) ) > 0 )
    {
        ++error_counter;
        rfm12_write(CONFIGURATION_SETTING);                                                // чтобы обнулить регистр отправки
        rfm12_write(FIFO_AND_RESET);                                                       // перестать заполнять FIFO

        pack_counter = pack_size = 0;
        rfm12_write(FIFO_AND_RESET | FIFO_EN_MASK);
        rfm12_write(POWER_RX);
    }

    return 0;
}


uint8_t packet_handler(msg_par par)
{
    if(pager_counter < PAGER_WAIT_TIME)    ++pager_counter;
    else    if(status2 == 'P')             status2 = ' ';

	if(rfm_condition != NEW_PACK)  return 2;                        // если нового пакета нет - возвращает 1

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        killTimer(MSG_PACK_HANDLER);

        switch (pack_buffer[0])
        {
            case BUTTON :
            {
                switch (pack_buffer[1])
                {
                case CALL :
                {
                    if (id_send)
                    {
                        if ((id_any == 1) || (pack_buffer[2] == menu_id))
                            send_id();
                    }
                    else
                    {
                        if ( (add_active_call() > 0) || (second_call > 0) )
                        {
                            seep_call_write();
                            if(snd_on) {
                                setTimer(MSG_BEEP, 6, snd_duration);
                                setTimer(MSG_BEEP_REP, 6, snd_duration*10);
                            }

                            if(active_on)   sendMessage(MSG_KEY_PRESS, 0);
                            else
                            {
                                stopMenu();
                                startMenuNull();
                                display_call();
                            }
                        }
                        send_ackn();
                    }
                    break;
                }
                case CANCEL :
                {
                    if (id_send)
                    {
                        if ((id_any == 1) || (pack_buffer[2] == menu_id))
                            send_id();
                    }
                    else
                    {
                        if ( remove_active_call() )
                        {
                            seep_call_write();
                            if(snd_on) {
                                killTimer(MSG_BEEP_REP);
                                setTimer(MSG_BEEP, 2, snd_duration);
                            }

                            if(active_on)   sendMessage(MSG_KEY_PRESS, 0);
                            else
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
                        send_ackn();
                    }
                    break;
                }
                }
                break;
            }

            case PAGER :
            {
                pager_counter = 0;
                if(status2 == ' ')  status2 = 'P';

                switch(pack_buffer[1])
                {
                    case EVENT :
                        send_active_call();
                        break;
                    case TIME :
                        send_time();
                        break;
                }
                break;
            }
        }

        setTimer(MSG_PACK_HANDLER, 0, 1);
    }
	return 1;
}

