#include "globals.h"

#include "hardware/keyboard.h"
#include "hardware/ds1307.h"
#include "hardware/24lc512.h"
#include "hardware/rfm12.h"
#include "hardware/lcd.h"

#include "messages.h"
#include "menu.h"
#include "menuitems.h"
#include "event.h"

uint8_t menu_x = 0, nevent = 0, menu_channel = 1, event_buffer[RECORD_SIZE], menu_psw_digit[4];
uint16_t menu_password;

extern char lcd_buffer[18];
extern uint8_t time[7];
extern volatile uint8_t active_call_id[N_ACTIVE_CALL_MAX], n_active_calls, channel, snd_on, second_call, pack_buffer[PACKET_SIZE], rfm_condition, active_on, status1, status2, send_channel, id_send, id, id_any, menu_id, psw_on;
extern volatile uint16_t seep_pointer, nbuttons, snd_duration;


uint8_t kill_password(msg_par par)
{
    static uint8_t counter = 0;

    if (++counter > 29)
    {
        psw_on = 0;
        killTimer(MSG_KILL_PSW);
    }
    return 0;
}



uint8_t Key_password(msg_par par)
{
	switch (par)
	{
        case KEY_LEFT :
        {
            if(menu_x > 0)  --menu_x;
            else            menu_x = 3;
            break;
        }
        case KEY_RIGHT :
        {
            if(menu_x < 3)  ++menu_x;
            else            menu_x = 0;
            break;
        }

        case KEY_DOWN:
        {
            if (menu_psw_digit[menu_x] > 0)   --menu_psw_digit[menu_x];
            else            menu_psw_digit[menu_x] = 9;

            break;
        }

        case KEY_UP:
        {
            if (menu_psw_digit[menu_x] < 9)   ++menu_psw_digit[menu_x];
            else            menu_psw_digit[menu_x] = 0;

            break;
        }

        case KEY_OK:
        {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                menu_password = menu_psw_digit[0]*1000 + menu_psw_digit[1]*100 + menu_psw_digit[2]*10 + menu_psw_digit[3];
                killHandler(MSG_KEY_PRESS, &Key_password);
                startMenu();
                if (menu_password == PASSWORD)
                {
//                    goto_m_s2i1();
//                    dispMenu(0);
                    menu_psw_digit[3] = menu_psw_digit[2] = menu_psw_digit[1] = menu_psw_digit[0] = 0;
                    psw_on = 1;
                    setTimer(MSG_KILL_PSW, 0, 48000);
                }
                else
                {
                    lcd_gotoxy(0,1);
                    lcd_puts("\275e\263ep\275\303\271 \276apo\273\304");
                }
                break;
            }
        }
	}

	if ( (par == KEY_LEFT) || (par == KEY_RIGHT) || (par == KEY_UP) || (par == KEY_DOWN) || (par == 0) )
    {
        lcd_gotoxy(0,1);
        sprintf(lcd_buffer, "      %d%d%d%d      ", menu_psw_digit[0], menu_psw_digit[1], menu_psw_digit[2], menu_psw_digit[3]);
        lcd_puts(lcd_buffer);

        lcd_gotoxy(menu_x+6,1);
	}

	return 1;
}



uint8_t Key_time(msg_par par)
{
	switch (par)
	{
        case KEY_LEFT : {
            if (menu_x>0)    menu_x -= 3;
            lcd_gotoxy(menu_x,1);
            break;
        }

        case KEY_RIGHT : {
            if (menu_x<12)    menu_x += 3;
            lcd_gotoxy(menu_x,1);
            break;
        }

        case KEY_DOWN : {
            switch (menu_x)
            {
                case 0:
                    if (time[4]>0)  --time[4];
                    else            time[4] = 31;
                    break;
                case 3:
                    if (time[5]>0)  --time[5];
                    else            time[5] = 12;
                    break;
                case 6:
                    if (time[6]>0)  --time[6];
                    else            time[6] = 99;
                    break;
                case 9:
                    if (time[2]>0)    --time[2];
                    else              time[2] = 23;
                    break;
                case 12:
                    if (time[1]>0)    --time[1];
                    else              time[1] = 59;
                    break;
            }
            break;
        }

        case KEY_UP : {
            switch (menu_x)
            {
                case 0:
                    if (time[4]<31)  ++time[4];
                    else             time[4] = 0;
                    break;
                case 3:
                    if (time[5]<12)  ++time[5];
                    else             time[5] = 0;
                    break;
                case 6:
                    if (time[6]<99)  ++time[6];
                    else             time[6] = 0;
                    break;
                case 9:
                    if (time[2]<23)  ++time[2];
                    else             time[2] = 0;
                    break;
                case 12:
                    if (time[1]<59)  ++time[1];
                    else             time[1] = 0;
                    break;
            }
            break;
        }

        case KEY_OK:
        {
            time[0] = time[3] = 0x00;
            ds_set_time(time);

            lcd_init(LCD_DISP_ON);
            killHandler(MSG_KEY_PRESS, &Key_time);
            startMenu();
            break;
        }
	}

    if ( (par == KEY_UP) | (par == KEY_DOWN) | (par == 0) )
    {
        sprintf(lcd_buffer, "%02d.%02d.%02d %02d:%02d", time[4], time[5], time[6], time[2], time[1]);
        lcd_gotoxy(0,1);
        lcd_puts(lcd_buffer);
        lcd_gotoxy(menu_x,1);
    }

	return (1);
}



uint8_t Key_snd(msg_par par)
{
	switch (par)
	{
        case KEY_LEFT : {
            if (menu_x)    menu_x = 0;
            else           menu_x = 1;
            break;
        }

        case KEY_RIGHT : {
            if (menu_x)    menu_x = 0;
            else           menu_x = 1;
            break;
        }

        case KEY_DOWN : {
            switch (menu_x)
            {
                case 0:
                    if (snd_on)    snd_on = 0;
                    else           snd_on = 1;
                    break;
                case 1:
                    if (snd_duration>160)  snd_duration -= 160;
                    else                 snd_duration = 1600;
                    break;
            }
            break;
        }

        case KEY_UP : {
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
            ds_write_byte(SND_ON_POINTER, snd_on);
            ds_write_byte(SND_DURATION_HPOINTER, (uint8_t)(snd_duration>>8));
            ds_write_byte(SND_DURATION_LPOINTER, (uint8_t)snd_duration);

            if(!snd_on) killTimer(MSG_BEEP_REP);

            lcd_init(LCD_DISP_ON);
            killHandler(MSG_KEY_PRESS, &Key_snd);
            startMenu();
            break;
        }
	}

    if ( (par == KEY_UP) | (par == KEY_DOWN) | (par == 0) )
    {
        lcd_gotoxy(7,0);
        if (snd_on) lcd_puts("\263\272\273\306\300e\275 ");
        else        lcd_puts("\263\303\272\273\306\300e\275");
        lcd_gotoxy(7,1);
        if (snd_duration == 1600)   sprintf(lcd_buffer, "1.0");
        else                        sprintf(lcd_buffer, "0.%01d", snd_duration/160);
        lcd_puts(lcd_buffer);

        lcd_gotoxy(8,menu_x);
    }

	return (1);
}


uint8_t Key_channel(msg_par par)
{
	switch (par)
	{
        case KEY_DOWN:
        {
            if(channel > 1)  --channel;
            else             channel = 14;
            break;
        }

        case KEY_UP:
        {
            if(channel < 14) ++channel;
            else             channel = 1;
            break;
        }

        case KEY_OK:
        {
            ds_write_byte(CHANNEL_POINTER, channel);

            rfm12_channel(channel);

            killHandler(MSG_KEY_PRESS, &Key_channel);
            startMenu();
            break;
        }
	}

	if ( (par == KEY_UP) || (par == KEY_DOWN) || (par == 0) )
    {
        lcd_gotoxy(0,1);
        sprintf(lcd_buffer, "Ka\275a\273: %2d", channel);    // Канал: channel
        lcd_puts(lcd_buffer);
        lcd_gotoxy(8,1);
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



uint8_t Key_event(msg_par par)
{
	switch (par)
	{
        case KEY_DOWN :
        {
            if( nevent < ((seep_pointer>>3)-2) )
                ++nevent;
            break;
        }

        case KEY_UP :
        {
            if (nevent > 0)
                --nevent;
            break;
        }

        case KEY_OK:
        {
            killHandler(MSG_KEY_PRESS, &Key_event);
            startMenu();
            break;
        }
	}

	if ( (par == KEY_UP) || (par == KEY_DOWN) || (par == 0) )
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            seep_read(seep_pointer - ((nevent+1)<<3), event_buffer, RECORD_SIZE);
            lcd_gotoxy(0,0);
            sprintf(lcd_buffer, "\xcc%04d #%03d ", (seep_pointer>>3) - nevent-1, event_buffer[1]);    // №xxxx #xxx
            lcd_puts(lcd_buffer);
            if(event_buffer[0] == CALL)    lcd_puts("\263\303\267.");
            else                           lcd_puts("o\277\274.");
            lcd_gotoxy(0,1);
            sprintf(lcd_buffer, "%02d.%02d %02d:%02d:%02d ", event_buffer[4], event_buffer[3], event_buffer[5], event_buffer[6], event_buffer[7]);
            lcd_puts(lcd_buffer);
            killTimer(MSG_KEY_REPEAT);
        }
    }
	return 1;
}


uint8_t Key_id(msg_par par)
{
	switch (par)
	{
        case KEY_LEFT :
        {
            if(menu_x > 0)  --menu_x;
            else            menu_x = 3;
            break;
        }
        case KEY_RIGHT :
        {
            if(menu_x < 3)  ++menu_x;
            else            menu_x = 0;
            break;
        }

        case KEY_DOWN:
        {
            switch (menu_x)
            {
                case 0 :
                {
                    if(menu_channel > 1)    --menu_channel;
                    else                    menu_channel = 14;
                    break;
                }
                case 1 :
                {
                    if( (menu_id == 0) && (id_any == 0) )  id_any = 1;
                    else {
                        --menu_id;
                        id_any = 0;
                    }
                    break;
                }
                case 2 :
                {
                    if(send_channel > 1)    --send_channel;
                    else                    send_channel = 14;
                    break;
                }
                case 3 :
                {
                    --id;
                    break;
                }
            }
            break;
        }

        case KEY_UP:
        {
            switch (menu_x)
            {
                case 0 :
                {
                    if(menu_channel < 14)    ++menu_channel;
                    else                    menu_channel = 1;
                    break;
                }
                case 1 :
                {
                    if ( (menu_id == 255) && (id_any == 0) )    id_any = 1;
                    else {
                        ++menu_id;
                        id_any = 0;
                    }
                    break;
                }
                case 2 :
                {
                    if(send_channel < 14)    ++send_channel;
                    else                    send_channel = 1;
                    break;
                }
                case 3 :
                {
                    ++id;
                    break;
                }
            }
            break;
        }

        case KEY_OK:
        {
            ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
            {
                rfm12_channel(channel);
                id_send = 0;
                killHandler(MSG_KEY_PRESS, &Key_id);
                startMenuNull();
                break;
            }
        }
	}

	if ( (par == KEY_LEFT) || (par == KEY_RIGHT) || (par == KEY_UP) || (par == KEY_DOWN) || (par == 0) )
    {
        ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
        {
            lcd_gotoxy(0,1);
            if (id_any) sprintf(lcd_buffer, "%2d --- \377 %2d %3d ", menu_channel, send_channel, id);
            else        sprintf(lcd_buffer, "%2d %3d \377 %2d %3d ", menu_channel, menu_id, send_channel, id);
            lcd_puts(lcd_buffer);
            switch (menu_x)
            {
                case 0 :
                    lcd_gotoxy(1,1);
                    break;
                case 1 :
                    lcd_gotoxy(4,1);
                    break;
                case 2 :
                    lcd_gotoxy(10,1);
                    break;
                case 3 :
                    lcd_gotoxy(13,1);
                    break;
            }
            rfm12_channel(menu_channel);
        }
	}

	return 1;
}


uint8_t Key_active(msg_par par)
{
    uint8_t i;

    active_on = 1;

	switch (par)
	{
        case KEY_DOWN:
        {
            if ( (nevent+8) < (n_active_calls) )
                nevent += 8;
            break;
        }

        case KEY_UP:
        {
            if( nevent > 7 )
                nevent -= 8;
            break;
        }

        case KEY_OK:
        {
            active_on = 0;
            killHandler(MSG_KEY_PRESS, &Key_active);
            startMenu();
            break;
        }
	}

	if ( (par == KEY_UP) || (par == KEY_DOWN) || (par == 0) )
    {
        lcd_clrscr();

        if(n_active_calls)
        {
            for ( i=nevent ; (i < (nevent + 8)) && (i < n_active_calls) ; ++i )
            {
                sprintf(lcd_buffer, "%03d ", active_call_id[i]);
                lcd_puts(lcd_buffer);
            }
            if(nevent > 1) {
                lcd_gotoxy(15,0);
                lcd_putc(0xd9);
            }
            if(n_active_calls > (nevent+8)) {
                lcd_gotoxy(15,1);
                lcd_putc(0xda);
            }
        }
        else
        {
            lcd_gotoxy(2,1);
            lcd_puts("He\277 \263\303\267o\263o\263");
        }
    }

	return 1;
}


uint8_t Key_scall(msg_par par)
{

	switch (par)
	{
        case KEY_DOWN:
        {
            if(nevent) nevent = 0;
            else       nevent = 1;
            break;
        }

        case KEY_UP:
        {
            if(nevent) nevent = 0;
            else       nevent = 1;
            break;
        }

        case KEY_OK:
        {
            second_call = nevent;
            ds_write_byte(SECOND_CALL_POINTER, second_call);

            killHandler(MSG_KEY_PRESS, &Key_scall);
            startMenu();
            break;
        }
	}

	if ( (par == KEY_UP) || (par == KEY_DOWN) || (par == 0) )
    {
        lcd_gotoxy(0,1);
        if(nevent) lcd_puts("   \276p\270\275\270\274a\277\304    ");
        else       lcd_puts("  \270\264\275op\270po\263a\277\304  ");
        lcd_gotoxy(8,1);
	}

	return 1;
}



