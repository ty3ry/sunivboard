#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/select.h>
#include <poll.h>


unsigned char *device;

struct irc
{
    __u16 code;
    __u32 value; 
    __u8  keyState;
};

#define KEY_PRESSED_JITTER  0
#define KEY_PRESSED_ONCE    1
#define KEY_PRESSED_REPEAT  2

/**
 * struct rc_map_table - represents a scancode/keycode pair
 *
 * @scancode: scan code (u32)
 * @keycode: Linux input keycode
 */
struct rc_map_table {
	__u32	scancode;
	__u32	keycode;
};

static const __u16 key_map[][3] = {
    {0x9117,    KEY_POWER,  KEY_POWER2},
    {0x9115,    KEY_MUTE,   KEY_MICMUTE},
    {}
};

__u16 scan_ir_code(int fd)
{
    struct irc ircode;
    ssize_t bytesRead;
    int ret;
    fd_set readfds;
    struct input_event event;
    struct timeval timeout;

    /* Wait on fd for input */
    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);

    timeout.tv_sec = 0;
    timeout.tv_usec = 250000;

    if ( !(select(fd + 1, &readfds, NULL, NULL, &timeout)) )
    {
        ircode.value = 0;
        return 0;
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
                //printf("[EV_MSC] code: %d value: 0x%x \n", event.code, event.value);
                if (ircode.value != event.value) {
                    ircode.value = event.value;
                    printf("Value: 0x%x \n", ircode.value);
                    // for (int i = 0; i<3; i+=1) {
                    //     if (ircode.value == key_map[i][0])
                    //         return key_map[i][1];
                    // } 
                    return ircode.value;
                }
                // else 
                // {
                //     printf("Repeat: 0x%x \n", ircode.value);
                //     /* repeat */
                //     for (int i = 0; i<3; i+=1) {
                //         if (ircode.value == key_map[i][0])
                //             return key_map[i][2];
                //     } 
                // }
            }
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    int fd;
    __u16 code;

    if (argc < 2) {
        return -1;
    }

    device = argv[1];

    fd = open(device, O_RDONLY | O_NONBLOCK);
    /* Let's open our input device */
    if (fd < 0)
    {
        fprintf(stderr, "Error opening %s for reading", device);
        exit(EXIT_FAILURE);
    }


    while (1)
    {
        if ((code = scan_ir_code(fd)) != 0) {
            printf("Scan value: 0x%x\n", code);
        }
    }
    close(fd);
    return EXIT_SUCCESS;
}
