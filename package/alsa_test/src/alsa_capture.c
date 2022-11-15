/**
 * file: alsa capture
 * 
 * capture sound to file
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>
#include <signal.h>

#define BUFSIZE 128
#define RATE 44100
#define	SOUND_PCM_FORMAT	(SND_PCM_FORMAT_S16_LE)

FILE *fout = NULL;
snd_pcm_t *capture_handle;
snd_pcm_uframes_t frames;

void print_usage(char *progname)
{
    printf(
        "Usage: %s [OPTIONS]...\n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
        "    -d, device          Device/card name used for pcm capture\n"
        "    -s  sampling_rate   sampling rate\n"
        "    -f  file            File name\n"
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
    char *file;
    char *capture_name;
    unsigned int rate;
} CmdLineOptions;

CmdLineOptions parse_command_line(int argc, char **argv)
{
    CmdLineOptions options = {
        .file = ".capture.wav",
        .rate = RATE
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
            options.capture_name = argv[++i];
        }
        else if (strcmp(arg, "-f") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.file = argv[++i];
        }
        else if (strcmp(arg, "-s") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            arg = argv[++i];
            if (!expect_int(arg, &options.rate))
            {
                fprintf(stderr, "error: Sampling rate value not valid\n", arg);
                exit(1);
            }
        }
        else
        {
            fprintf(stderr, "error: Unrecognized option: '%s'\n", arg);
            exit(1);
        }
    }

    return options;
}

int sound_init(CmdLineOptions *cmdOpt)
{
	int err;
	snd_pcm_hw_params_t *params;
   

	/* open the pcm device */
	if ((err = snd_pcm_open(&capture_handle, cmdOpt->capture_name, SND_PCM_STREAM_CAPTURE, 0)) < 0)
	{
		printf("failed to open pcm device \"%s\" (%s)\n", cmdOpt->capture_name, snd_strerror(err));
		return -1;
	}

	/* alloc memory space for hardware parameter structure*/
	if ((err = snd_pcm_hw_params_malloc(&params)) < 0){
		printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Initialize the sound card parameter structure with default data */
	if ((err = snd_pcm_hw_params_any(capture_handle, params)) < 0) {
		printf("failed to initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the access parameters */
	if ((err = snd_pcm_hw_params_set_access(capture_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf("cannot set access type (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the data format */
	if ((err = snd_pcm_hw_params_set_format(capture_handle, params, SOUND_PCM_FORMAT)) < 0) {
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sample rate of the sound card */
	if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, params, &cmdOpt->rate, 0)) < 0)
	{
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sound card channel */
	if ((err = snd_pcm_hw_params_set_channels(capture_handle, params, 2)) < 0) {
		printf("cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params(capture_handle, params)) < 0) {
		printf("cannot set parameters (%s)\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_hw_params_free(params);

	return 0;
}

/*
 * quit on ctrl-c
 */
void sigint(int sig)
{
	if (fout != NULL)
	{
		fclose(fout);
	}
	exit(1);
}

int main(int argc, char *argv[])
{
	int i;
	int err;
	short buf[BUFSIZE];
	int nread;

    CmdLineOptions options = parse_command_line(argc, argv);
	
    if(options.display_help) {
        print_usage(argv[0]);
        exit(0);
    }
    
    if ((fout = fopen(options.file, "w")) == NULL)
	{
		fprintf(stderr, "Can't open %s for writing\n", options.file);
		exit(1);
	}

    /* set signal handler */
	signal(SIGINT, sigint);

    if (sound_init(&options) < 0) {
        printf("Failed to init sound card (%s)\n", options.capture_name);
        exit(EXIT_FAILURE);
    }

	while (1)
	{
		if ((nread = snd_pcm_readi(capture_handle, buf, BUFSIZE)) < 0)
		{
			fprintf(stderr, "read from audio interface failed (%s)\n",
					snd_strerror(err));
			/* recover */
			snd_pcm_prepare(capture_handle);
		}
		else
		{
			fwrite(buf, sizeof(short), nread, fout);
		}
	}

	snd_pcm_close(capture_handle);
	exit(0);
}
