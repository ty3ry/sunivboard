#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/select.h>
#include <poll.h>

#include "ir_recv.h"


struct input_event event;
struct irc ircode;
ssize_t bytesRead;
fd_set readfds;

static const __u16 key_map[][3] = {
    {0x9117,    KEY_POWER,          KEY_POWER2}, // power
    {0x9115,    KEY_MUTE,           KEY_MICMUTE}, // mute
    {0x9158,    KEY_BLUETOOTH,      0},
    {0x910a,    KEY_MODE,           0},
    {0x9100,    KEY_NUMERIC_0,      0},
    {0x9101,    KEY_NUMERIC_1,      0},
    {0x9102,    KEY_NUMERIC_2,      0},
    {0x9103,    KEY_NUMERIC_3,      0},
    {0x9104,    KEY_NUMERIC_4,      0},
    {0x9105,    KEY_NUMERIC_5,      0},
    {0x9106,    KEY_NUMERIC_6,      0},
    {0x9107,    KEY_NUMERIC_7,      0},
    {0x9108,    KEY_NUMERIC_8,      0},
    {0x9109,    KEY_NUMERIC_9,      0},
    {0x9111,    KEY_MEMO,           0},
    {0x9150,    KEY_VOLUMEUP,       0},
    {0x9151,    KEY_VOLUMEDOWN,     0},
    {0x911a,    KEY_NEXT,           0},
    {0x911b,    KEY_PREVIOUS,       0},
    {0x9113,    KEY_PLAYPAUSE,      0},
    {0x913c,    KEY_LIGHTS_TOGGLE,  0},
    {0x915d,    KEY_BASSBOOST,      0},
    {0x9131,    KEY_SOUND,          0},
    
};



struct irc scan_ir_code(int fd)
{
    static struct timeval timeout;
    static __u16 c_press = 0;
    static __u16 cur_code = 0;

    /* Wait on fd for input */
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 120000;

    if ( !(select(fd + 1, &readfds, NULL, NULL, &timeout)) )
    {
        cur_code = 0;
        c_press = 0;
        ircode.event = IR_EV_IDLE;
        return ircode;
    }

    /* File descriptor is now ready */
    if (FD_ISSET(fd, &readfds))
    {
        bytesRead = read(fd, &event, sizeof(struct input_event));
        if (bytesRead > 0)
        {
            if (event.type == EV_KEY)
            {
                printf("[EV_KEY] code: %d value: 0x%X\n",event.code, event.value);
            } 
            else if (event.type == EV_MSC)
            {
                if (cur_code != event.value) {
                    cur_code = event.value;
                    printf("Value: 0x%x \n", cur_code);
                    for (int i = 0; i<KEY_MAP_LEN; i+=1) {
                        if (cur_code == key_map[i][0]) {
                            ircode.event = IR_EV_SHORTPRESS;
                            ircode.value = key_map[i][1];
                            //break;
                            return ircode;
                        }
                    } 
                }
                else 
                {
                    if (c_press++ > 2)
                    {
                        printf("Repeat: 0x%x \n", cur_code);
                        /* repeat */
                        for (int i = 0; i<KEY_MAP_LEN; i+=1) {
                            if (cur_code == key_map[i][0]){
                                ircode.event = IR_EV_PRESSHOLD;
                                ircode.value = key_map[i][2];
                                //break;
                                return ircode;
                            }
                        } 
                    }
                }
            }
            //return ircode;
        }
    }

    ircode.event = IR_EV_IDLE;
    return ircode;
}

