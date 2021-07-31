#ifndef MENU_H_INCLUDED
#define MENU_H_INCLUDED



typedef const struct PROGMEM menustruct {
	struct menustruct *Next;
	struct menustruct *Previous;
	struct menustruct *Parent;
	struct menustruct *Child;
	uint8_t     Select;
	const char  Text[];
} menuItem;


#define MAKE_MENU(Name, Next, Previous, Parent, Child, Select, Text) \
    extern menuItem Next;     \
	extern menuItem Previous; \
	extern menuItem Parent;   \
	extern menuItem Child;  \
	menuItem Name = {(struct menustruct*)&Next, (struct menustruct*)&Previous, (struct menustruct*)&Parent, (struct menustruct*)&Child, (uint8_t)Select,  {Text}}


#define PREVIOUS   ((menuItem*)pgm_read_word(&selectedMenuItem->Previous))
#define NEXT       ((menuItem*)pgm_read_word(&selectedMenuItem->Next))
#define PARENT     ((menuItem*)pgm_read_word(&selectedMenuItem->Parent))
#define CHILD      ((menuItem*)pgm_read_word(&selectedMenuItem->Child))
#define SELECT		(pgm_read_byte(&selectedMenuItem->Select))

#define NULL_ENTRY Null_Menu


enum {
    MENU_CANCEL=1,
    MENU_TIME,
    MENU_SND,
    MENU_CH,
	MENU_ROM,
	MENU_ST,
	MENU_EV,
	MENU_ID,
	MENU_ACT,
	MENU_SCALL
};


uint8_t startMenu(void);
uint8_t startMenuNull(void);
void initMenu(void);
uint8_t stopMenu(void);
uint8_t menuKey(msg_par par);
uint8_t selectMenu(msg_par par);
uint8_t checkMenu(void);


#endif // MENU_H_INCLUDED
