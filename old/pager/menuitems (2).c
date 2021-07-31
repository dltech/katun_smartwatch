#include "globals.h"
#include <avr/eeprom.h>

#include "messages.h"
#include "menuitems.h"
#include "menu.h"
#include "event.h"

#include "hardware/keyboard.h"
#include "hardware/lcd.h"
#include "hardware/rfm22.h"


#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))
extern uint8_t     snd_on_eep          EEPROM_SECTION ;
extern uint16_t    snd_duration_eep    EEPROM_SECTION ;
extern uint8_t     light_on_eep        EEPROM_SECTION ;
extern uint16_t    light_duration_eep  EEPROM_SECTION ;
extern uint8_t     channel_eep         EEPROM_SECTION ;


extern char lcd_buffer[9];
extern volatile uint8_t channel, snd_on, light_on, menu_x, nevent, active_on, rf_on, n_active_calls, active_call_id[N_ACTIVE_CALL_MAX];
extern volatile uint16_t snd_duration, light_duration;




uint8_t Key_snd(msg_par par)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        switch (par)
        {
            case KEY_LEFT : {
                switch (menu_x)
                {
                    case 0:
                        if (snd_on)    snd_on = 0;
                        else           snd_on = 1;
                        break;
                    case 1:
                        if (snd_duration>160)  snd_duration -= 160;
                        else                   snd_duration = 1600;
                        break;
                }
                break;
            }

            case KEY_RIGHT : {
                switch (menu_x)
                {
                    case 0:
                        if (snd_on)    snd_on = 0;
                        else           snd_on = 1;
                        break;
                    case 1:
                        if (snd_duration<1600)  snd_duration += 160;
                        else                     snd_duration = 160;
                        break;
                }
                break;
            }

            case KEY_OK:
            {
                if(menu_x==0)
                {
                    menu_x = 1;
                    break;
                }

                eeprom_write_byte(&snd_on_eep, snd_on);
                eeprom_write_word(&snd_duration_eep, snd_duration);

                lcd_init(LCD_DISP_ON);
                killHandler(MSG_KEY_PRESS, &Key_snd);
                startMenu();
                break;
            }
        }

        if ( (par == KEY_LEFT) | (par == KEY_RIGHT) | (par == 0) )
        {
            lcd_gotoxy(0,0);
            if (snd_on) lcd_puts("\263\272\273\306\300e\275 ");
            else        lcd_puts("\263\303\272\273\306\300e\275");
            lcd_gotoxy(4,1);
            if (snd_duration == 1600)   sprintf(lcd_buffer, "1.0");
            else                        sprintf(lcd_buffer, "0.%d", snd_duration/160);
            lcd_puts(lcd_buffer);

            lcd_gotoxy(4,menu_x);
        }
    }
	return (1);
}


uint8_t Key_light(msg_par par)
{
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        switch (par)
        {
            case KEY_LEFT : {
                switch (menu_x)
                {
                    case 0:
                        if (light_on)    light_on = 0;
                        else             light_on = 1;
                        break;
                    case 1:
                        if (light_duration>800)  light_duration -= 800;
                        else                     light_duration = 14400;
                        break;
                }
                break;
            }

            case KEY_RIGHT : {
                switch (menu_x)
                {
                    case 0:
                        if (light_on)    light_on = 0;
                        else             light_on = 1;
                        break;
                    case 1:
                        if (light_duration<14400)  light_duration += 800;
                        else                       light_duration = 800;
                        break;
                }
                break;
            }

            case KEY_OK:
            {
                if(menu_x==0)
                {
                    menu_x = 1;
                    break;
                }

                eeprom_write_byte(&light_on_eep, light_on);
                eeprom_write_word(&light_duration_eep, light_duration);

                lcd_init(LCD_DISP_ON);
                killHandler(MSG_KEY_PRESS, &Key_light);
                startMenu();
                break;
            }
        }

        if ( (par == KEY_LEFT) | (par == KEY_RIGHT) | (par == 0) )
        {
            lcd_gotoxy(0,0);
            if (light_on) lcd_puts("  \263\272\273. ");
            else          lcd_puts("  \263\303\272\273. ");
            lcd_gotoxy(4,1);
            if( (light_duration%1600) > 0 ) sprintf(lcd_buffer, "%d.5", light_duration/1600);
            else                            sprintf(lcd_buffer, "%d.0", light_duration/1600);
            lcd_puts(lcd_buffer);

            lcd_gotoxy(4,menu_x);
        }
    }
	return (1);
}


uint8_t Key_channel(msg_par par)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        switch (par)
        {
            case KEY_LEFT:
            {
                if(channel > 1)  --channel;
                else             channel = 14;
                break;
            }

            case KEY_RIGHT:
            {
                if(channel < 14) ++channel;
                else             channel = 1;
                break;
            }

            case KEY_OK:
            {
                eeprom_write_byte(&channel_eep, channel);
                rfm22_write(FREQ_HOP_CHSEL_P, channel-1);

                killHandler(MSG_KEY_PRESS, &Key_channel);
                startMenu();
                break;
            }
        }

        if ( (par == KEY_LEFT) || (par == KEY_RIGHT) || (par == 0) )
        {
            lcd_gotoxy(0,1);
            sprintf(lcd_buffer, "Ka\275a\273 %2d", channel);
            lcd_puts(lcd_buffer);
            lcd_gotoxy(7,1);
        }
    }
    return 1;
}



uint8_t Key_ok(msg_par par)
{
	if( par == KEY_OK )
	{
        killHandler(MSG_KEY_PRESS, &Key_ok);
        startMenu();
	}
	return 1;
}


uint8_t Key_rf(msg_par par)
{
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        switch (par)
        {
            case KEY_LEFT : {
                if(rf_on)  rf_on = 0;
                else       rf_on = 1;
                break;
            }

            case KEY_RIGHT : {
                if(rf_on)  rf_on = 0;
                else       rf_on = 1;
                break;
            }

            case KEY_OK:
            {
                lcd_init(LCD_DISP_ON);
                killHandler(MSG_KEY_PRESS, &Key_rf);
                startMenu();
                break;
            }
        }

        if ( (par == KEY_LEFT) | (par == KEY_RIGHT) | (par == 0) )
        {
            lcd_gotoxy(0,1);
            if (rf_on) lcd_puts("  \263\272\273. ");
            else       lcd_puts("  \263\303\272\273.");
            lcd_gotoxy(4,1);
        }
    }
	return (1);
}


uint8_t Key_active(msg_par par)
{
    uint8_t i;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
        active_on = 1;

        switch (par)
        {
            case KEY_LEFT:
            {
                if ( nevent > 3 )
                    nevent -= 4;
                break;
            }

            case KEY_RIGHT:
            {
                if( (nevent+4) < (n_active_calls) )
                    nevent += 4;
                break;
            }

            case KEY_OK:
            {
                killHandler(MSG_KEY_PRESS, &Key_active);
                active_on = 0;
                startMenu();
                break;
            }
        }

        if ( (par == KEY_LEFT) || (par == KEY_RIGHT) || (par == 0) )
        {
            lcd_clrscr();

            if(n_active_calls)
            {
                for ( i=nevent ; (i < (nevent + 2)) && (i < n_active_calls) ; ++i )
                {
                    sprintf(lcd_buffer, "%03d ", active_call_id[i]);
                    lcd_puts(lcd_buffer);
                }
                lcd_gotoxy(0,1);
                for ( i=(nevent+2) ; (i < (nevent + 4)) && (i < n_active_calls) ; ++i )
                {
                    sprintf(lcd_buffer, "%03d ", active_call_id[i]);
                    lcd_puts(lcd_buffer);
                }

                if(nevent > 1) {
                    lcd_gotoxy(7,0);
                    lcd_putc(0xd9);
                }
                if(n_active_calls > (nevent+4)) {
                    lcd_gotoxy(7,1);
                    lcd_putc(0xda);
                }
            }
            else
            {
                lcd_gotoxy(0,1);
                lcd_puts("He\277 \263\303\267.");
            }
        }
    }
	return 1;
}
