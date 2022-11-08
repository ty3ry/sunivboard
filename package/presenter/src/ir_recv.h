#ifndef IR_RECV_H
#define IR_RECV_H

#include <stdint.h>

struct irc
{
    __u16 code;
    __u32 value; 
    __u8  event;
};

#define KEY_MAP_LEN (sizeof(key_map) / sizeof(key_map[0]))

#define IR_EV_IDLE          0
#define IR_EV_SHORTPRESS    1
#define IR_EV_PRESSHOLD     2


struct irc scan_ir_code(int fd);

#endif /*IR_RECV_H*/