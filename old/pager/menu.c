#include "globals.h"

#include "messages.h"
#include "menu.h"
#include "menuitems.h"
#include "event.h"

#include "hardware/keyboard.h"
#include "hardware/lcd.h"


menuItem *selectedMenuItem, *menuStack[10];                     // выбраный пункт меню
volatile uint8_t menuStackTop;

const char strNULL[] PROGMEM = "";

menuItem    Null_Menu = {(struct menustruct*)0, (struct menustruct*)0, (struct menustruct*)0, (struct menustruct*)0, 0, {0x00}};    //объявление пустого пункта меню

extern char lcd_buffer[8];
extern volatile uint8_t time[7], snd_on, light_on, menu_x, nevent, n_active_calls, active_call_id[N_ACTIVE_CALL_MAX];

//        NAME,    NEXT,        PREVIOUS,    PARENT,     CHILD
MAKE_MENU(m_s1i1,  m_s1i2,    m_s1i2,      NULL_ENTRY, m_s1i2,       MENU_ACT,  "");
MAKE_MENU(m_s1i2,  m_s1i3,    m_s1i5,      NULL_ENTRY, m_s1i1,       0, " B\303xo\343");                           // Выход
MAKE_MENU(m_s1i3,  m_s1i4,    m_s1i2,      NULL_ENTRY, m_s2i1,       0, " Hac\277p.");                             // Настройка
MAKE_MENU(m_s1i4,  m_s1i5,    m_s1i3,      NULL_ENTRY, NULL_ENTRY,   MENU_DATE, "  \340a\277a");                   // Дата
MAKE_MENU(m_s1i5,  m_s1i6,    m_s1i4,      NULL_ENTRY, NULL_ENTRY,   MENU_RF, " C\263\307\267\304");               // Связь
MAKE_MENU(m_s1i6,  m_s1i2,    m_s1i5,      NULL_ENTRY, NULL_ENTRY,   MENU_ACT, "A\272\277. \263\303\267");         // Активные вызовы

// подменю настройка
MAKE_MENU(m_s2i1,  m_s2i2,    m_s2i4,      m_s1i2,     NULL_ENTRY,   MENU_SND, "  \244\263y\272");                  // Звук
MAKE_MENU(m_s2i2,  m_s2i3,    m_s2i1,      m_s1i2,     NULL_ENTRY,   MENU_CH, " Ka\275a\273");                   // Канал
MAKE_MENU(m_s2i3,  m_s2i4,    m_s2i2,      m_s1i2,     NULL_ENTRY,   MENU_LIGHT, " \250o\343c\263. ");                 // Подсветка
MAKE_MENU(m_s2i4,  m_s2i1,    m_s2i3,      m_s1i2,     m_s1i2,       0, " Ha\267a\343");                            // Назад


/* В этой функции описывается алгоритм выбора пунктов меню */
uint8_t selectMenu(msg_par par)
{
    lcd_clrscr();

	switch (par)
	{
    case MENU_SND :
	{
		lcd_init(LCD_DISP_ON_CURSOR_BLINK);
		lcd_gotoxy(0,1);
		lcd_puts("\343\273\270\277   c");
        menu_x = 0;
		setHandler(MSG_KEY_PRESS, &Key_snd);
        sendMessage(MSG_KEY_PRESS, 0);
		break;
	}
    case MENU_LIGHT :
	{
		lcd_init(LCD_DISP_ON_CURSOR_BLINK);
		lcd_gotoxy(0,1);
		lcd_puts("\343\273\270\277   c");
        menu_x = 0;
		setHandler(MSG_KEY_PRESS, &Key_light);
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
	case MENU_DATE :
	{
        sprintf(lcd_buffer,"%02d ", time[4]);
        lcd_puts(lcd_buffer);
        switch(time[5])
        {
            case 1 :
                lcd_puts("\307\275\263.");
                break;
            case 2 :
                lcd_puts("\344e\263p.");
                break;
            case 3 :
                lcd_puts("\274ap\277");
                break;
            case 4 :
                lcd_puts("a\276p.");
                break;
            case 5 :
                lcd_puts("\274a\307");
                break;
            case 6 :
                lcd_puts("\270\306\275\307");
                break;
            case 7 :
                lcd_puts("\270\306\273\307");
                break;
            case 8 :
                lcd_puts("a\263\264.");
                break;
            case 9 :
                lcd_puts("ce\275\277.");
                break;
            case 10 :
                lcd_puts("o\272\277.");
                break;
            case 11 :
                lcd_puts("\275o\307\262p.");
                break;
            case 12 :
                lcd_puts("\343e\272.");
                break;
        }

        lcd_gotoxy(0,1);
        sprintf(lcd_buffer, "  20%02d", time[6]);
        lcd_puts(lcd_buffer);
/*        switch(time[3])
        {
            case 1 :
                lcd_puts("\276\275");
                break;
            case 2 :
                lcd_puts("\263\277");
                break;
            case 3 :
                lcd_puts("cp");
                break;
            case 4 :
                lcd_puts("\300\277");
                break;
            case 5 :
                lcd_puts("\276\277");
                break;
            case 6 :
                lcd_puts("c\262");
                break;
            case 7 :
                lcd_puts("\263c");
                break;
        } */
        setHandler(MSG_KEY_PRESS, &Key_ok);
        break;
	}
    case MENU_RF :
	{
        lcd_init(LCD_DISP_ON_CURSOR_BLINK);
        setHandler(MSG_KEY_PRESS, &Key_rf);
        sendMessage(MSG_KEY_PRESS, 0);
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
unsigned char dispMenu(msg_par par) {

	lcd_cleanstr(1);
	if(selectedMenuItem == (menuItem*)&m_s1i1)  display_call();
	else                                        lcd_puts_p((char *)&selectedMenuItem->Text);

	return (1);
}


/* Переход к другому пункту меню по нажатой кнопке */
uint8_t menuKey(msg_par par) {
	if(light_on)    sendMessage(MSG_LIGHT, 0);

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
	case KEY_OK:
		{ // выбор пункта
			uint8_t sel;
			sel = SELECT;
			if (sel != 0) {

                stopMenu();
				sendMessage(MSG_MENU_SELECT, sel);

				return (1);
			} else {
				menuChange(CHILD);
			}
			break;
		}
	}
    dispMenu(0);

	return (1);
}


/* выход из меню */
uint8_t stopMenu(void) {
	killHandlersNum(MSG_KEY_PRESS);
	killTimer(MSG_CLOCK_DISP_REF);
	killHandler(MSG_DISP_REFRESH, &dispMenu);
	return (0);
}


/* вход в меню */
uint8_t startMenu(void) {
	lcd_init(LCD_DISP_ON);
	dispMenu(0);

	setHandler(MSG_KEY_PRESS, &menuKey);
	setHandler(MSG_DISP_REFRESH, &dispMenu);
    setTimer(MSG_CLOCK_DISP_REF, 0, 800);
    sendMessage(MSG_CLOCK_DISP_REF, 0);
    return (0);
}



/* вход в меню */
uint8_t startMenuNull(void) {
	selectedMenuItem = (menuItem*)&m_s1i1;
    startMenu();
    if(n_active_calls) display_call();
    return (0);
}


uint8_t checkMenu(void)
{
    if(	selectedMenuItem == (menuItem*)&m_s1i1 ) return 1;
    else                                         return 0;
}

