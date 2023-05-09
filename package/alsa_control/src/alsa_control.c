/**
 * file: alsa_control.c
 * 
 * brief: c program to setting various alsa related control using input device like irc
*/
#include <alsa/asoundlib.h>
//#include <alsa/mixer.h> // already declared @asoundlib.h
#include <stdlib.h>
#include <stdbool.h>
#include <linux/input.h>
#include <sys/select.h>
#include <poll.h>


#define IR_DEV  "/dev/input/event0"

struct irc
{
    __u16 code;
    __u32 value; 
    __u8  event;
};

unsigned char *device;
struct input_event event;
struct irc ircode;
ssize_t bytesRead;
fd_set readfds;

/** device name */
const char* hw_name = "hw:1";   // card 1
const char* ir_dev_name = "/dev/input/event0";
const char* mixer_name = "Master";  // mixer name on card 1

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
    {0x9150,    KEY_VOLUMEUP,       KEY_VOLUMEUP},
    {0x9151,    KEY_VOLUMEDOWN,     KEY_VOLUMEDOWN},
    {0x911a,    KEY_NEXT,           0},
    {0x911b,    KEY_PREVIOUS,       0},
    {0x9113,    KEY_PLAYPAUSE,      0},
    {0x913c,    KEY_LIGHTS_TOGGLE,  0},
    {0x915d,    KEY_BASSBOOST,      0},
    {0x9131,    KEY_SOUND,          0},
    
};

#define KEY_MAP_LEN (sizeof(key_map) / sizeof(key_map[0]))

#define IR_EV_IDLE          0
#define IR_EV_SHORTPRESS    1
#define IR_EV_PRESSHOLD     2

struct irc scan_ir_code(int fd)
{
    static struct timeval timeout;
    static __u16 c_press = 0;
    static __u16 cur_code = 0;

    /* Wait on fd for input */
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 125000;

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
                    //printf("Value: 0x%x \n", cur_code);
                    for (int i = 0; i<KEY_MAP_LEN; i+=1) {
                        if (cur_code == key_map[i][0]) {
                            ircode.event = IR_EV_SHORTPRESS;
                            ircode.value = key_map[i][1];
                            return ircode;
                        }
                    } 
                }
                else 
                {
                    if (c_press++ > 2)
                    {
                        //printf("Repeat: 0x%x \n", cur_code);
                        /* repeat */
                        for (int i = 0; i<KEY_MAP_LEN; i+=1) {
                            if (cur_code == key_map[i][0]){
                                ircode.event = IR_EV_PRESSHOLD;
                                ircode.value = key_map[i][2];
                                return ircode;
                            }
                        } 
                    }
                }
            }
        }
    }

    ircode.event = IR_EV_IDLE;
    return ircode;
}


int main(int argc, char **argv)
{
    snd_mixer_t *mixer;
    snd_mixer_selem_id_t *ident;
    snd_mixer_elem_t *elem;
    long min, max;
    long old_volume, volume;

    int fd;
    struct irc code;

    fd = open(ir_dev_name, O_RDONLY | O_NONBLOCK);
    /* Let's open our input device */
    if (fd < 0)
    {
        fprintf(stderr, "Error opening %s for reading", ir_dev_name);
        exit(EXIT_FAILURE);
    }

    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, hw_name);
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&ident);
    snd_mixer_selem_id_set_index(ident, 0);
    snd_mixer_selem_id_set_name(ident, mixer_name);
    elem = snd_mixer_find_selem(mixer, ident);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, 0, &old_volume);

    printf("Min %ld max %ld current volume %ld\n", min, max, old_volume);

    volume = old_volume;

    while(1)
    {
        code = scan_ir_code(fd);

        if (code.event == IR_EV_IDLE){

        } else if (code.event == IR_EV_PRESSHOLD || code.event == IR_EV_SHORTPRESS) {
            
            switch(code.value){
                case KEY_VOLUMEUP:
                    if (volume < max) {
                        volume += 1;
                    }
                    printf("VOL UP : %d\r\n", volume);
                    snd_mixer_selem_set_playback_volume_all(elem, volume);
                break;

                case KEY_VOLUMEDOWN:
                    if (volume > min) {
                        volume -= 1;
                    }
                    printf("VOL DW : %d\r\n", volume);
                    snd_mixer_selem_set_playback_volume_all(elem, volume);
                break;

                case KEY_MUTE:
                     printf("Mute \n");
                break;

                default : break;
            }
        } else {

        }

#if 0
        switch (code.event) {
        case IR_EV_IDLE:
        break;

        case IR_EV_PRESSHOLD:
        case IR_EV_SHORTPRESS:
        /** volume */
        if(code.value == KEY_VOLUMEUP) {
            if (volume < max) {
                volume += 1;
            }
            printf("Volume up : %d\r\n", volume);
            snd_mixer_selem_set_playback_volume_all(elem, volume);
        }
        else if (code.value = KEY_VOLUMEDOWN) {
            if (volume > min) {
                volume -= 1;
            }
            printf("Volume down : %d\r\n", volume);
            snd_mixer_selem_set_playback_volume_all(elem, volume);
        }
        break;
        }
#endif

    }

    close(fd);
    exit(EXIT_SUCCESS);
}
