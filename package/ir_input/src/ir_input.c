#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/select.h>

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




int main(int argc, char **argv)
{
    int fd;
    struct input_event event;
    ssize_t bytesRead;
    unsigned char *device;
    struct irc ircode;

    int ret;
    fd_set readfds;

    if (argc < 2) {
        return -1;
    }

    device = argv[1];

    fd = open(device, O_RDONLY);
    /* Let's open our input device */
    if (fd < 0)
    {
        fprintf(stderr, "Error opening %s for reading", device);
        exit(EXIT_FAILURE);
    }

    ircode.keyState = KEY_PRESSED_ONCE;

    while (1)
    {
        /* Wait on fd for input */
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        ret = select(fd + 1, &readfds, NULL, NULL, NULL);

        if (ret == -1)
        {
            fprintf(stderr, "select call on %s: an error occurred",
                    device);
            break;
        }
        else if (!ret)
        { /* If we have decided to use timeout */
            fprintf(stderr, "select on %s: TIMEOUT", device);
            break;
        }

        /* File descriptor is now ready */
        if (FD_ISSET(fd, &readfds))
        {
            bytesRead = read(fd, &event,
                             sizeof(struct input_event));
            if (bytesRead == -1)
            {
                /* Process read input error*/
                fprintf(stderr, "read input error \n");
            }
            
            if (bytesRead != sizeof(struct input_event))
            {
                /* Read value is not an input even */
                printf("Read value is not an input event \n");
            }
            
            if (event.type == EV_KEY)
            {
                printf("[EV_KEY] code: %d value: 0x%X\n",event.code, event.value);
            } 
            else if (event.type == EV_MSC)
            {
                //printf("[EV_MSC] code: %d value: 0x%x \n", event.code, event.value);

#if 1
                switch(ircode.keyState) {
                default: break;
                case KEY_PRESSED_JITTER:
                    //ircode.keyState = (ircode.value != event.value) ? KEY_PRESSED_ONCE : KEY_PRESSED_JITTER;
                //     ircode.keyState = KEY_PRESSED_ONCE;
                // break;

                case KEY_PRESSED_ONCE:
                    printf("Key press : 0x%x \n", event.value);
                    ircode.keyState = KEY_PRESSED_REPEAT;
                break;

                case KEY_PRESSED_REPEAT:
                    printf("Repeat : 0x%x \n", event.value);
                    ircode.keyState = KEY_PRESSED_JITTER;
                break;

                }
#endif
            }
        }

    }
    close(fd);
    return EXIT_SUCCESS;
}
