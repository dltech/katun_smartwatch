#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED


#define N_ACTIVE_CALL_MAX   24
#define RECORD_SIZE 10
#define MAX_RECORDS SEEP_MAX_POINTER / RECORD_SIZE


#define ID_GET  0x49
#define CALL    0x01
#define CANCEL  0x02

#define BUTTON  0x50
#define PAGER   0x73
#define CLOCK   0x63
#define EVENT   0x4d

#define NCALLS_MAX 30
#define CALL_SIZE  4
#define CALL_BUFFER_SIZE    NCALLS_MAX*CALL_SIZE

/*
#define EEPROM_SECTION  __attribute__ ((section (".eeprom")))

extern uint16_t    dummy                                EEPROM_SECTION  ;       // avoid using lowest addresses
extern uint8_t     eep_call_pointer                     EEPROM_SECTION  ;
extern uint8_t     eep_ncalls                           EEPROM_SECTION  ;
extern uint8_t     eep_ncalls_over                      EEPROM_SECTION  ;
extern uint8_t     eep_call_buffer[CALL_BUFFER_SIZE]    EEPROM_SECTION  ;
*/

void display_call(void);


#endif // EVENT_H_INCLUDED
