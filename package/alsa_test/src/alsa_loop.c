
/**
 * file : alsa_loop.c
 * used for embedded linux training (2022)
 * linux sound programming 
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <alsa/asoundlib.h>

#define BUF_BYTES (256 * 4 * 4)

#define LATENCY (250000)

/***
 * command line options for alsa_loop program 
*/
typedef struct
{
    bool display_help;
	char *playback_name;    /* used for pcm playback */
    char *capture_name;     /* used for pcm capture */
	int channel;
    int sample_rate;
    unsigned long latency;   /* adjust latency */
} CmdLineOptions;

void print_usage(char *prog_name)
{
    printf(
        "Usage: %s [options] \n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
		"    -i, --input         Set input (capture) device \n "
        "    -o, --output        Set output (playback) device \n "
		"    -c  channel    	 set channel (stereo or mono) \n"
        "    -s  sampling_rate   sampling rate\n"
        "    -l  --latency       adjust latency in ms \n"
        "\n", prog_name);
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

CmdLineOptions parse_command_line(int argc, char **argv)
{
    CmdLineOptions options = {
        .sample_rate = 44100,
		.capture_name = "default",
        .playback_name = "default",
		.channel = 2,
        .latency = LATENCY
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
		else if (strcmp(arg, "-i") == 0 || strcmp(arg, "--input") == 0)
		{
			if ( i + 1 == argc ) {
				fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
			}
			options.capture_name = argv[++i]; /* set input capture device */
		}
        else if (strcmp(arg, "-o") == 0 || strcmp(arg, "--output") == 0)
		{
			if ( i + 1 == argc ) {
				fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
			}
			options.playback_name = argv[++i]; /* set input playback device */
		}
		else if (strcmp(arg, "-s") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			arg = argv[++i];
			if ( !expect_int(arg, &options.sample_rate) ) {
				fprintf(stderr, "error: Sampling rate not valid\n", arg);
                exit(1);
			}
		}
		else if (strcmp(arg, "-l") == 0 || strcmp(arg, "--latency") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }

            arg = argv[++i];
			if ( !expect_int(arg, &options.latency) ) {
				fprintf(stderr, "error: Sampling rate not valid\n", arg);
                exit(1);
			}
		}
		else if (strcmp(arg, "-c") == 0 || strcmp(arg, "--channel") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			arg = argv[++i];
			if ( !expect_int(arg, &options.channel) ) {
				fprintf(stderr, "error: channel not valid, set 1 (mono) or 2 (stereo) channel \n", arg);
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

int main(int argc, char *argv[])
{
    int err;
    int buf[BUF_BYTES];
    CmdLineOptions options = parse_command_line(argc, argv);

    if (options.display_help) {
        print_usage(argv[0]);
        exit(0);
    }

    snd_pcm_t *playback_handle;
    snd_pcm_t *capture_handle;

    snd_pcm_format_t format = SND_PCM_FORMAT_S16_LE;

    unsigned int buf_frames = BUF_BYTES / options.channel / 2;

    if ((err = snd_pcm_open(&playback_handle, options.playback_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        fprintf(stderr, "Cannot open audio device %s (%s)\n", options.playback_name, snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_set_params(playback_handle, 
                                    format, 
                                    SND_PCM_ACCESS_RW_INTERLEAVED, 
                                    options.channel, 
                                    options.sample_rate, 
                                    1, 
                                    options.latency)) < 0)
    { 
        fprintf(stderr, "Playback open error: %s\n", snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_open(&capture_handle, options.capture_name, SND_PCM_STREAM_CAPTURE, 0)) < 0)
    {
        fprintf(stderr, "Cannot open audio device %s (%s)\n", options.capture_name, snd_strerror(err));
        exit(1);
    }

    if ((err = snd_pcm_set_params(capture_handle, 
                                    format, 
                                    SND_PCM_ACCESS_RW_INTERLEAVED, 
                                    options.channel, 
                                    options.sample_rate, 
                                    1, 
                                    options.latency)) < 0)
    { 
        fprintf(stderr, "Capture open error: %s\n", snd_strerror(err));
        exit(1);
    }

    while (1)
    {

        if ((err = snd_pcm_readi(capture_handle, buf, buf_frames)) != buf_frames)
        {
            fprintf(stderr, "read from audio interface failed (%s)\n", snd_strerror(err));
            exit(1);
        }
        if ((err = snd_pcm_writei(playback_handle, buf, buf_frames)) != buf_frames)
        {
            fprintf(stderr, "write to audio interface failed (%s)\n", snd_strerror(err));
            exit(1);
        }
        
    }

    fprintf(stderr, "close handles\n");
    snd_pcm_close(playback_handle);
    snd_pcm_close(capture_handle);

    return 0;
}
