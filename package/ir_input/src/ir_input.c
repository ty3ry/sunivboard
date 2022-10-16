#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <sys/select.h>


int main(int argc, char **argv)
{
    int fd;
    struct input_event event;
    ssize_t bytesRead;
    unsigned char *device;

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
                /**
                 * do parsing ...
                */
                printf("[EV_KEY] code: %d value: %d\n",event.code, event.value);
            } 
            else if (event.type == EV_MSC)
            {
                /*  
                * do parsing ...
                */
                printf("[EV_MSC] code: %d value: %d \n", event.code, event.value);
            }
            else {
                /** exception **/
            }
        }
    }
    close(fd);
    return EXIT_SUCCESS;
}
