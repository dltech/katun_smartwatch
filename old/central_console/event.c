#include "globals.h"
#include "hardware/keyboard.h"
#include "hardware/24lc512.h"
#include "hardware/ds1307.h"
#include "hardware/rfm12.h"
#include "hardware/lcd.h"

#include "messages.h"
#include "menu.h"
#include "event.h"

volatile uint8_t active_call_id[N_ACTIVE_CALL_MAX], n_active_calls;
volatile uint16_t nbuttons;

extern char lcd_buffer[18];
extern volatile uint8_t  channel, send_channel, id, status1, pack_buffer[PACKET_SIZE];
extern volatile uint16_t seep_pointer;


uint8_t verify_and_set_bit(uint8_t byte)
{
    const uint8_t index = (byte >> 3) + BITMASK_POINTER;
    const uint8_t bit_mask = 1 << (byte & 0x07);
    byte = ds_read_byte(index);

    if ( byte & bit_mask )
        return 0;
    else
    {
        ds_write_byte(index, byte | bit_mask);
        return 1;
    }
}


/* Добавляет ID пульта в список активных вызовов, если его там нет */
uint8_t add_active_call(void)
{
    uint8_t find_index = 0, i;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
// Проверка наличия ID пульта в списке, если есть - функция возвращает 0.
        while ( (active_call_id[find_index] != pack_buffer[2]) && (find_index < n_active_calls) )
            ++find_index;

        if ( find_index != n_active_calls )
            return 0;

// Добавляет ID пульта в список, если список переполнен - записывает по кругу.
        if( n_active_calls < N_ACTIVE_CALL_MAX )
            active_call_id[n_active_calls++] = pack_buffer[2];
        else
        {
            for ( i=0 ; i<(N_ACTIVE_CALL_MAX-1) ; ++i )
                active_call_id[i] = active_call_id[i+1];

            active_call_id[N_ACTIVE_CALL_MAX-1] = pack_buffer[2];
        }
    }
    return 1;
}


/* Удалят ID из списка, сдвигая писок к началу */
uint8_t remove_active_call(void)
{
    uint8_t find_index = 0;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
// Проверяет наличие ID в списке, если нету - возвращает 0.
        while ( (active_call_id[find_index] != pack_buffer[2]) && (find_index < n_active_calls) )
            ++find_index;

        if (find_index == n_active_calls)
            return 0;

// Если есть - удаляет его из списка, сдвигая список к началу. Возвращает 1.
        --n_active_calls;

        for( ; find_index < n_active_calls ; ++find_index )
            active_call_id[find_index] = active_call_id[find_index+1];
    }
    return 1;
}


/* Записывает данные во внешнюю EEPROM. */
void seep_call_write(void)
{
    uint8_t i, event_buffer[RECORD_SIZE], time[7];

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        if ( (seep_pointer + RECORD_SIZE) >= (SEEP_MAX_POINTER) ) {
            status1 = '!';
            seep_pointer = 0x0008;
        }

        event_buffer[0] = pack_buffer[1];      // вызов/отмена
        event_buffer[1] = pack_buffer[2];      // id пакета

        ds_read_time(time);
        for ( i = 2 ; i < 5 ; ++i ) event_buffer[i] = time[8-i];
        for ( i = 5 ; i < 8 ; ++i ) event_buffer[i] = time[7-i];

        seep_pointer += RECORD_SIZE;

        ds_write_byte(SEEP_HPOINTER, (uint8_t)(seep_pointer>>8));
        ds_write_byte(SEEP_LPOINTER, (uint8_t)(seep_pointer&0x00ff));

        seep_write(seep_pointer-RECORD_SIZE, event_buffer, 10);
    }
}


/* Отображает событие на экран */
void display_call(void)
{
    char lcd_buffer[16];

    if( n_active_calls > 1 )
        sprintf(lcd_buffer, "A\272\277\270\263\275\303x \xb3\xc3\xb7. %d", n_active_calls);
    else if (n_active_calls == 1)
        sprintf(lcd_buffer, "ID = %03d \263\303\267o\263", active_call_id[0]);
    else
        lcd_buffer[0] = '\0';

    lcd_cleanstr(1);
    lcd_puts(lcd_buffer);
}



void send_ackn(void)
{
    pack_buffer[0+HEADER_NBYTES] = BUTTON;
    pack_buffer[1+HEADER_NBYTES] = ACKN;
    pack_buffer[2+HEADER_NBYTES] = pack_buffer[2];

    rfm12_transmit_pack(3);
}


void send_id(void)
{
    pack_buffer[0+HEADER_NBYTES] = BUTTON;
    pack_buffer[1+HEADER_NBYTES] = ID_GET;
    pack_buffer[2+HEADER_NBYTES] = id;
    pack_buffer[3+HEADER_NBYTES] = send_channel;

    if( verify_and_set_bit(id) )                     // если такой id раньше не добавлялся
    {
        ++nbuttons;                                     // инкремент счетчика добавленых пультов

        ds_write_byte(NBUTTONS_HPOINTER, (uint8_t)(nbuttons>>8));
        ds_write_byte(NBUTTONS_LPOINTER, (uint8_t)nbuttons);
    }

    rfm12_transmit_pack(4);
}


void send_active_call(void)
{
    uint8_t i = HEADER_NBYTES;

    pack_buffer[i++] = PAGER;
    pack_buffer[i++] = EVENT;

    for ( ; i<(n_active_calls+HEADER_NBYTES+2) ; ++i )    pack_buffer[i] = active_call_id[i-2-HEADER_NBYTES];

    rfm12_transmit_pack(n_active_calls+2);
}


void send_time(void)
{
    uint8_t time[7], i = HEADER_NBYTES;

    ds_read_time(time);

    pack_buffer[i++] = PAGER;
    pack_buffer[i++] = TIME;

    for ( ; i<(9+HEADER_NBYTES) ; ++i ) pack_buffer[i] = time[i-2-HEADER_NBYTES];

    rfm12_transmit_pack(9);
}
