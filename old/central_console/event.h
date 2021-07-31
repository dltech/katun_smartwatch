#ifndef EVENT_H_INCLUDED
#define EVENT_H_INCLUDED

#define N_ACTIVE_CALL_MAX   24
#define RECORD_SIZE 8
#define MAX_RECORDS SEEP_MAX_POINTER / RECORD_SIZE

#define BUTTON  0x50
#define ID_GET  0x49
#define CALL    0x01
#define CANCEL  0x02
#define ACKN    0x03

/** Формат пакета кнопки.
pack_buffer[0]  тип устройства (конопка или пейджер)
pack_buffer[1]  команда (получить ID вызов или отмена)
pack_buffer[2]  ID пульта
pack_buffer[3]  канал
**/

#define PAGER       0x73
#define TIME        0x63
#define EVENT       0x4d

/** Формат пакета пейджера.
pack_buffer[0]  тип устройства (конопка или пейджер)
pack_buffer[1]  команда (запрос времени либо сообщений)
pack_buffer[2]  пусто (ибо ID пейджеру нафиг не нужен)
pack_buffer[3]  канал
pack_buffer[4]  часы
pack_buffer[5]  минуты
pack_buffer[6]  количество активных вызовов
pack_buffer[7]  ID пульта 1
...
pack_buffer[31] ID пульта n
**/


/* Определение адресов переменных, хранящихся в памяти часов */

#define SND_ON_POINTER          0x09
#define SND_DURATION_HPOINTER   0x0a
#define SND_DURATION_LPOINTER   0x0b
#define NBUTTONS_HPOINTER       0x0c
#define NBUTTONS_LPOINTER       0x0d
#define CHANNEL_POINTER         0x0e
#define SEEP_HPOINTER           0x0f
#define SEEP_LPOINTER           0x10
#define SECOND_CALL_POINTER     0x11
#define BITMASK_POINTER         0x12


uint8_t verify_and_set_bit(uint8_t byte);
uint8_t add_active_call(void);
uint8_t remove_active_call(void);
void seep_call_write(void);
void display_call(void);

// функции, формирующие и отравляющие нужные пакеты
void send_ackn(void);
void send_id(void);
void send_active_call(void);
void send_time(void);



#endif // MESS_H_INCLUDED
