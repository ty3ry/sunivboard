/******
Demo for ssd1306 i2c driver for  Raspberry Pi 
******/

#include "ssd1306_i2c.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/** default i2c device */
#define	I2C_DEVICE	"/dev/i2c-0"

void print_usage(char *progname)
{
    printf(
        "Usage: %s [OPTIONS]...\n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
        "    -d, device          I2C device \n"
        "\n", progname);
}

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

typedef struct
{
    bool display_help;
    char *device;
} CmdLineOptions;

CmdLineOptions parse_command_line(int argc, char **argv)
{
    CmdLineOptions options = {
		.device = I2C_DEVICE,
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
            options.device = argv[++i];
        }
        else
        {
            fprintf(stderr, "error: Unrecognized option: '%s'\n", arg);
            exit(1);
        }
    }

    return options;
}

void main(int argc, char **argv) {
	const char* text = "This is demo for SSD1306 i2c driver for Embedded Linux";
	
	CmdLineOptions options = parse_command_line(argc, argv);

    if (options.display_help)
    {
        print_usage(argv[0]);
        exit(0);
    }

	ssd1306_begin(SSD1306_SWITCHCAPVCC, SSD1306_I2C_ADDRESS, options.device);
	/** clear buffer contain adafruit logo */
	
	ssd1306_display();
	sleep(1);

	ssd1306_clearDisplay();
	ssd1306_drawString(text);
	ssd1306_display();
	sleep(1);

	//ssd1306_dim(1);
	ssd1306_startscrollright(00,0x0F);
	//ssd1306_startscrolldiagright(0, 0xff);
	sleep(1);

	//ssd1306_clearDisplay();
	while(1);

}
