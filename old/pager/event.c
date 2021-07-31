#include <avr/eeprom.h>
#include "globals.h"

#include "event.h"
#include "hardware/lcd.h"


extern char lcd_buffer[9];
volatile uint8_t active_call_id[NCALLS_MAX], n_active_calls;

void display_call(void)
{
        if(n_active_calls)
            if(n_active_calls == 1)     sprintf(lcd_buffer, "ID %03d", active_call_id[0]);
            else                        sprintf(lcd_buffer, "B\303\267. %2d", n_active_calls);
        else lcd_buffer[0] = '\0';
        lcd_cleanstr(1);
        lcd_puts(lcd_buffer);
}


/*
 Функция записи события в EEPROM.
void write_and_display_call(void)
{
    if (ncalls < NCALLS_MAX)            ++ncalls;                          // реализация записи по кругу
    else if( ncalls_over < NCALLS_MAX ) ++ncalls_over;
    else {
        ncalls_over = 0;
        call_pointer = 0;
    }

    if (pack_buffer[1] == CALL) pack_buffer[2] |= 0x80;                    // установка бита если вызов

    eeprom_write_byte(&eep_call_buffer[call_pointer++], pack_buffer[2]);   // секунды
    eeprom_write_byte(&eep_call_buffer[call_pointer++], pack_buffer[3]);   // минуты
    eeprom_write_byte(&eep_call_buffer[call_pointer++], pack_buffer[4]);   // час
    eeprom_write_byte(&eep_call_buffer[call_pointer++], pack_buffer[5]);   // ID

    sprintf(lcd_buffer, "%03d ", pack_buffer[5]);
    lcd_gotoxy(0,1);
    lcd_puts_b(lcd_buffer);

    if (pack_buffer[1] == CALL)     lcd_puts("\263\303\267.");
    else                            lcd_puts("o\277\274.");
}
*/

