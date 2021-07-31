#ifndef MENUITEMS_H_INCLUDED
#define MENUITEMS_H_INCLUDED

#define PASSWORD    9745


/** Тут хранятся алгоритмы пунктов меню **/

uint8_t kill_password(msg_par par);
uint8_t Key_password(msg_par par);
uint8_t Key_time(msg_par par);
uint8_t Key_snd(msg_par par);
uint8_t Key_channel(msg_par par);
uint8_t Key_scall(msg_par par);

uint8_t Key_ok(msg_par par);
uint8_t Key_event(msg_par par);
uint8_t Key_active(msg_par par);
uint8_t Key_id(msg_par par);
uint8_t Key_button(msg_par par);


#endif // MENUITEMS_H_INCLUDED
