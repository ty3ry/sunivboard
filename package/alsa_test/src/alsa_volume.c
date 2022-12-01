/**
 * file: alsa_volume.c
*/
#include <alsa/asoundlib.h>
#include <alsa/mixer.h>
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

void print_usage(char *progname)
{
    printf(
        "Usage: %s [OPTIONS]...\n\n"
        "Options:\n"
        "    -h, --help             Display this message\n"
        "    -d, device_name        Device/card number used for pcm playback hw:x format\n"
        "    -n  mixer_control_name Mixer control name based on card\n"
        "    -i  device input (IR)  Input for /dev/input/eventX\n"
        "\n", progname);
}

typedef struct
{
    bool display_help;
    int device_num;
    char *device_name;
    char *mix_ctrl_name;
    char *ir_device;
} CmdLineOptions;

bool expect_int(char *str, int *out)
{
    int num = 0;
    for (char *p = str; *p != 0; p++)
    {
        if (*p < '0' || *p > '9')
        {
            return false;
        }
        int new_val = num * 10 + (*p - '0');
        if (new_val < num)
        {
            return false;
        }
        num = new_val;
    }
    if (out)
    {
        *out = num;
    }
    return true;
}

CmdLineOptions parse_command_line(int argc, char **argv)
{
    CmdLineOptions options = {
        .device_num = 0,
        .mix_ctrl_name = "Headphone",
        .ir_device = "/dev/input/event0"
    };

    if (argc == 1)
    {
        print_usage(argv[0]);
        exit(0);
    }
    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0)
        {
            options.display_help = true;
        }
        else if (strcmp(arg, "-d") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.device_name = argv[++i];
            // if (!expect_int(arg, &options.device_num)) {
            //     fprintf(stderr, "error: Sampling rate value not valid\n", arg);
            //     exit(1);
            // }
        }
        else if (strcmp(arg, "-n") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.mix_ctrl_name = argv[++i];
        }
        else if (strcmp(arg, "-i") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.ir_device = argv[++i];
        }
        else
        {
            fprintf(stderr, "error: Unrecognized option: '%s'\n", arg);
            exit(1);
        }
    }

    return options;
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

    CmdLineOptions options = parse_command_line(argc, argv);

    if (options.display_help) {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }


    fd = open(options.ir_device, O_RDONLY | O_NONBLOCK);
    /* Let's open our input device */
    if (fd < 0)
    {
        fprintf(stderr, "Error opening %s for reading", options.ir_device);
        exit(EXIT_FAILURE);
    }

    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, options.device_name);
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&ident);
    snd_mixer_selem_id_set_index(ident, 0);
    snd_mixer_selem_id_set_name(ident, options.mix_ctrl_name);
    elem = snd_mixer_find_selem(mixer, ident);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, 0, &old_volume);
    printf("Min %ld max %ld current volume %ld\n", min, max, old_volume);
    volume = old_volume;

    while(1)
    {
        code = scan_ir_code(fd);

        switch (code.event) {
        case IR_EV_IDLE:
        break;

        case IR_EV_PRESSHOLD:
        case IR_EV_SHORTPRESS:
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
        
    }

    close(fd);
    exit(EXIT_SUCCESS);
}
