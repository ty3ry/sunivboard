/*
 *  This extra small demo sends a random samples to your speakers.
 */
#include <stdbool.h>
#include <alsa/asoundlib.h>

#define LATENCY 500000

unsigned char buffer[16*1024];              /* some random data */

typedef struct
{
    bool display_help;
    char *playback_name;    /* pcm playback name */
    unsigned int channel;
    unsigned int rate;
    int soft_resample;  /* 0 = disallow alsa-lib resample stream, 1 = allow resampling */
	unsigned int latency;
} CmdLineOptions;

void print_usage(char *prog_name)
{
    printf(
        "Usage: %s [option] \n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
		"    -d, --device        Set device card, if none set to \"default\" \n "
		"    -c  channel    	 set channel \n"
        "    -s  sample rate   	 set sampling rate \n"
        "    -r  resample        0 = disallow alsa-lib resample stream, 1 = allow resampling \n"
        "    -l  latency   	     set latency (us) \n"
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
        .rate = 44100,
		.playback_name = "default",
		.channel = 2,
        .soft_resample = 1,
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
		else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--device") == 0)
		{
			if ( i + 1 == argc ) {
				fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
			}
			options.playback_name = argv[++i];
		}
		else if (strcmp(arg, "-s") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			arg = argv[++i];
			if ( !expect_int(arg, &options.rate) ) {
				fprintf(stderr, "error: Sampling rate not valid\n", arg);
                exit(1);
			}
		}
		else if (strcmp(arg, "-r") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			options.soft_resample = argv[++i];
		}
		else if (strcmp(arg, "-c") == 0 || strcmp(arg, "--channel") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			arg = argv[++i];
			if ( !expect_int(arg, &options.channel) || (options.channel != 2 && options.channel != 1) ) {
				fprintf(stderr, "error: channel not valid, set 1 (mono) or 2 (stereo) channel \n", arg);
                exit(1);
			}
		}
        else if (strcmp(arg, "-l") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			arg = argv[++i];
			if ( !expect_int(arg, &options.channel) ) {
				fprintf(stderr, "error: latency value not valid \n", arg);
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
    unsigned int i;
    snd_pcm_t *handle;
    snd_pcm_sframes_t frames;

    CmdLineOptions options = parse_command_line(argc, argv);

    if(options.display_help) {
        print_usage(argv[0]);
        exit(0);
    }
 
    for (i = 0; i < sizeof(buffer); i++)
        buffer[i] = random() & 0xff;
 
    if ((err = snd_pcm_open(&handle, options.playback_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
    if ((err = snd_pcm_set_params(handle,
                      SND_PCM_FORMAT_U8,
                      SND_PCM_ACCESS_RW_INTERLEAVED,
                      options.channel,
                      options.rate,
                      options.soft_resample,
                      options.latency)) < 0) { 
        printf("Playback open error: %s\n", snd_strerror(err));
        exit(EXIT_FAILURE);
    }
 
    for (i = 0; i < 16; i++) {
        frames = snd_pcm_writei(handle, buffer, sizeof(buffer));
        if (frames < 0)
            frames = snd_pcm_recover(handle, frames, 0);
        if (frames < 0) {
            printf("snd_pcm_writei failed: %s\n", snd_strerror(frames));
            break;
        }
        if (frames > 0 && frames < (long)sizeof(buffer))
            printf("Short write (expected %li, wrote %li)\n", (long)sizeof(buffer), frames);
    }
 
    /* pass the remaining samples, otherwise they're dropped in close */
    err = snd_pcm_drain(handle);
    if (err < 0)
        printf("snd_pcm_drain failed: %s\n", snd_strerror(err));
    snd_pcm_close(handle);
    return 0;
}
