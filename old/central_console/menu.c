#include "globals.h"

#include "messages.h"
#include "menu.h"
#include "menuitems.h"
#include "event.h"

#include "hardware/keyboard.h"
#include "hardware/ds1307.h"
#include "hardware/lcd.h"
#include "hardware/rfm12.h"


menuItem *selectedMenuItem, *menuStack[10];                     // выбраный пункт меню
volatile uint8_t menuStackTop;

const char strNULL[] PROGMEM = "";

menuItem    Null_Menu = {(struct menustruct*)0, (struct menustruct*)0, (struct menustruct*)0, (struct menustruct*)0, 0, {0x00}};    //объявление пустого пункта меню

extern volatile uint8_t second_call;
extern volatile uint16_t seep_pointer, nbuttons, n_active_calls;
extern uint8_t menu_x, channel, nevent;
uint8_t time[7];

//        NAME,    NEXT,        PREVIOUS,    PARENT,     CHILD
MAKE_MENU(m_s1i1,  m_s1i2,    m_s1i2,      NULL_ENTRY, m_s1i3,      0,          "");
MAKE_MENU(m_s1i2,  m_s1i3,    m_s1i8,      NULL_ENTRY, m_s1i1,      0,          "B\303xo\343");                                             // Выход
MAKE_MENU(m_s1i3,  m_s1i4,    m_s1i2,      NULL_ENTRY, m_s2i1,      0,          "Hac\277po\271\272\270");                                   // Настройка
MAKE_MENU(m_s1i4,  m_s1i5,    m_s1i3,      NULL_ENTRY, NULL_ENTRY,  MENU_ROM,   "Coc\277. \276a\274\307\277\270");                   // Состояние памяти
MAKE_MENU(m_s1i5,  m_s1i6,    m_s1i4,      NULL_ENTRY, NULL_ENTRY,  MENU_ST,    "C\277a\277yc");                                      // Статус
MAKE_MENU(m_s1i6,  m_s1i7,    m_s1i5,      NULL_ENTRY, NULL_ENTRY,  MENU_EV,    "Co\262\303\277\270\307");                            // События
MAKE_MENU(m_s1i7,  m_s1i8,    m_s1i6,      NULL_ENTRY, NULL_ENTRY,  MENU_ID,    "B\303\343a\300a ID");                                // Выдача ID
MAKE_MENU(m_s1i8,  m_s1i2,    m_s1i7,      NULL_ENTRY, NULL_ENTRY,  MENU_ACT,   "A\272\277\270\263\275\303e \263\303\267o\263\303"); // Активные вызовы

// подменю настройка
MAKE_MENU(m_s2i1,  m_s2i2,    m_s2i5,      m_s1i2,     NULL_ENTRY,   MENU_TIME, "\251c\277. \263pe\274e\275\270");                  // Установка времени
MAKE_MENU(m_s2i2,  m_s2i3,    m_s2i1,      m_s1i2,     NULL_ENTRY,   MENU_SND,  "Hac\277po\271\272a \267\263y\272a");                // Настройка звука
MAKE_MENU(m_s2i3,  m_s2i4,    m_s2i2,      m_s1i2,     NULL_ENTRY,   MENU_CH,   "B\303\262op \272a\275a\273a");                       // Выбор канала
MAKE_MENU(m_s2i4,  m_s2i5,    m_s2i3,      m_s1i2,     NULL_ENTRY,   MENU_SCALL,"\250o\263\277op\275\303e \263\303\267o\263\303"); // Повторные вызовы
MAKE_MENU(m_s2i5,  m_s2i1,    m_s2i4,      m_s1i2,     m_s1i2,       0,         "Ha\267a\343");                                             // Выход


/* В этой функции описывается алгоритм выбора пунктов меню */
uint8_t selectMenu(msg_par par)
{
    char lcd_buffer[16];

    lcd_clrscr();

	switch (par)
    {
	case MENU_TIME :
	{
		lcd_init(LCD_DISP_ON_CURSOR_BLINK);
		lcd_puts("\xe3\xe3 \xbc\xbc \xb4\xb4 \xc0\xc0 \xbc\xb8\xbd");
        ds_read_time(time);
        menu_x = 0;

		setHandler(MSG_KEY_PRESS, &Key_time);
        sendMessage(MSG_KEY_PRESS, 0);
		break;
	}
    case MENU_SND :
	{
		lcd_init(LCD_DISP_ON_CURSOR_BLINK);
		lcd_puts(" \267\263y\272: ");
		lcd_gotoxy(0,1);
		lcd_puts(" \343\273\270\277.     ce\272.");
        menu_x = 0;
		setHandler(MSG_KEY_PRESS, &Key_snd);
        sendMessage(MSG_KEY_PRESS, 0);
		break;
	}
	case MENU_CH :
	{
 		lcd_init(LCD_DISP_ON_CURSOR_BLINK);
        lcd_puts("B\263e\343\270\277e");

        setHandler(MSG_KEY_PRESS, &Key_channel);
        sendMessage(MSG_KEY_PRESS, 0);
        break;
	}
	case MENU_SCALL :
    {
        nevent = second_call;
        lcd_init(LCD_DISP_ON_CURSOR_BLINK);
        lcd_puts("\250o\263\277op\275\303e \263\303\267.:");

        setHandler(MSG_KEY_PRESS, &Key_scall);
        sendMessage(MSG_KEY_PRESS, 0);
        break;
    }

	case MENU_ROM :
	{
		sprintf(lcd_buffer,"\244a\276o\273\275e\275o:%3d%%", seep_pointer/640);
		lcd_puts(lcd_buffer);
        sprintf(lcd_buffer, "%4d \307\300. \270\267 %4d", seep_pointer>>3, 8000);
        lcd_gotoxy(0,1);
        lcd_puts(lcd_buffer);
		setHandler(MSG_KEY_PRESS, &Key_ok);
		break;
	}
	case MENU_ST :
	{
        lcd_puts("Ko\273\270\300ec\277\263o");
        sprintf(lcd_buffer, "\276y\273\304\277o\263: %3d", nbuttons);
        lcd_gotoxy(0,1);
        lcd_puts(lcd_buffer);

        setHandler(MSG_KEY_PRESS, &Key_ok);
        break;
	}
	case MENU_EV :
	{
        if (seep_pointer > RECORD_SIZE)
        {
            setHandler(MSG_KEY_PRESS, &Key_event);
            nevent = 0;
            sendMessage(MSG_KEY_PRESS, 0);
        }
        else
        {
            lcd_gotoxy(2,1);
            lcd_puts("He\277 \263\303\267o\263o\263");
            setHandler(MSG_KEY_PRESS, &Key_ok);
        }
        break;
	}
	case MENU_ID :
	{
        rfm12_channel(1);
        lcd_puts("    Bc\277a\263\304\277e");
        lcd_gotoxy(0,1);
        lcd_puts("   \262a\277ape\271\272\270");
        setHandler(MSG_KEY_PRESS, &Key_id);
        break;
	}
	case MENU_ACT :
	{
        setHandler(MSG_KEY_PRESS, &Key_active);
        nevent = 0;
        sendMessage(MSG_KEY_PRESS, 0);
        break;
	}
	}
	return 1;
}


/* Функция перехода к другому пункту меню, если пункт есть - то переходит, иначе - остается */
void menuChange(menuItem* NewMenu)
{
	if ((menuItem*)NewMenu == (menuItem*)&NULL_ENTRY)
	  return;

	selectedMenuItem = NewMenu;
	return;
}


/* Вывод пункта меню на экран */
uint8_t dispMenu(msg_par par) {

	lcd_cleanstr(1);

	if(selectedMenuItem == (menuItem*)&m_s1i1)  display_call();
    else                                        lcd_puts_p((char *)&selectedMenuItem->Text);

	return (1);
}


/* Переход к другому пункту меню по нажатой кнопке */
uint8_t menuKey(msg_par par) {
	switch (par) {
	case 0: {
		return 1;
        }
	case KEY_LEFT: {
		menuChange(PREVIOUS);
		break;
        }
	case KEY_RIGHT: {
		menuChange(NEXT);
		break;
        }
	case KEY_DOWN : {
            stopMenu();
            setHandler(MSG_KEY_PRESS, &Key_active);
            nevent = 0;
            sendMessage(MSG_KEY_PRESS, 0);
            break;
        }
	case KEY_OK:
		{ // выбор пункта
			uint8_t sel;
			sel = SELECT;
			if (sel != 0)
			{
                stopMenu();
				sendMessage(MSG_MENU_SELECT, sel);
				return (1);
			}
			else
			{
				menuChange(CHILD);
			}
			break;
		}
	case KEY_UP: { // отмена выбора (возврат)
		menuChange(PARENT);
        }
	}
    dispMenu(0);

	return (1);
}


/* выход из меню */
uint8_t stopMenu(void) {
	killHandlersNum(MSG_KEY_PRESS);
	killTimer(MSG_CLOCK_REF);
	killHandler(MSG_DISP_REFRESH, &dispMenu);
	return (0);
}


/* вход в меню */
uint8_t startMenu(void) {
	lcd_init(LCD_DISP_ON);
	dispMenu(0);

	setHandler(MSG_KEY_PRESS, &menuKey);
	setHandler(MSG_DISP_REFRESH, &dispMenu);
    setTimer(MSG_CLOCK_REF, 0, 800);
    sendMessage(MSG_CLOCK_REF, 0);

    if(n_active_calls) display_call();

    return (0);
}


/* вход в меню с нулевого пункта*/
uint8_t startMenuNull(void) {
	selectedMenuItem = (menuItem*)&m_s1i1;
    startMenu();
    return (0);
}


uint8_t checkMenu(void) {
    if(	selectedMenuItem == (menuItem*)&m_s1i1 ) return 1;
    else                                         return 0;
}
