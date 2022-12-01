/**
 * file: alsa_play_tone.c
*/

#include <alloca.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#include <alsa/asoundlib.h>

typedef unsigned char u8;
typedef short s16;
typedef float f32;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define SAMPLE_RATE 48000
#define AMPLITUDE 10000

snd_pcm_t *playback_handle;
snd_pcm_uframes_t frames;


f32 clamp(f32 value, f32 min, f32 max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

s16 *square_wave(s16 *buffer, size_t sample_count, int freq)
{
    int samples_full_cycle = floorf((f32)SAMPLE_RATE / (f32)freq);
    int samples_half_cycle = floorf((f32)samples_full_cycle / 2.0f);
    int cycle_index = 0;
    for (int i = 0; i < sample_count; i++)
    {
        s16 sample = 0;
        if (cycle_index < samples_half_cycle)
        {
            sample = +AMPLITUDE;
        }
        else
        {
            sample = -AMPLITUDE;
        }
        cycle_index = (cycle_index + 1) % samples_full_cycle;
        buffer[i] = sample;
    }
    return buffer;
}

s16 *sine_wave(s16 *buffer, size_t sample_count, int freq)
{
    for (int i = 0; i < sample_count; i++)
    {
        buffer[i] = AMPLITUDE * sinf(((f32)i / (f32)SAMPLE_RATE) * 2 * M_PI * freq);
    }
    return buffer;
}

void print_usage(char *progname)
{
    printf(
        "Usage: %s [OPTIONS]...\n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
        "    -d, DEVICE          Card Identifier \n"
        "    -l, --loop          Play sound in an infinite loop\n"
        "    -t, --type [sine|square]\n"
        "                        Generate and play a sound wave of the specified type\n"
        "    -f, --freq FREQ     Specify the frequency of the generated sound wave in Hz\n"
        "    --fade MS           Fade (in and out) in milliseconds, ignored when not playing a raw pcm\n"
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
    bool should_loop;
    char *playback_name;
    char *wave_type;
    unsigned int rate;  /* sample rate */
    int freq;
    int fade_ms;
} CmdLineOptions;

CmdLineOptions parse_command_line(int argc, char **argv)
{
    CmdLineOptions options = {
        .playback_name = "default",
        .rate = SAMPLE_RATE,
        .wave_type = "sine",
        .freq = 440
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
        else if (strcmp(arg, "-l") == 0 || strcmp(arg, "--loop") == 0)
        {
            options.should_loop = true;
        }
        else if (strcmp(arg, "-d") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.playback_name = argv[++i];
        }
        else if (strcmp(arg, "-t") == 0 || strcmp(arg, "--type") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.wave_type = argv[++i];
            if (strcmp(options.wave_type, "sine") != 0 && strcmp(options.wave_type, "square") != 0)
            {
                fprintf(stderr, "error: Unknown wave type: `%s`\n", options.wave_type);
                exit(1);
            }
        }
        else if (strcmp(arg, "-f") == 0 || strcmp(arg, "--freq") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            arg = argv[++i];
            if (!expect_int(arg, &options.freq) || options.freq < 20 || options.freq > 20000)
            {
                fprintf(stderr, "error: Frequency needs to be an integer between 20-20000 (instead was `%s`)\n", arg);
                exit(1);
            }
        }
        else if (strcmp(arg, "--fade") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            arg = argv[++i];
            if (!expect_int(arg, &options.fade_ms) || options.fade_ms < 0 || options.fade_ms > 5000)
            {
                fprintf(stderr, "error: Fade needs to be an integer between 0-5000 (instead was `%s`)\n", arg);
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
	if ((err = snd_pcm_open(&playback_handle, cmdOpt->playback_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
	{
		printf("failed to open pcm device \"%s\" (%s)\n", cmdOpt->playback_name, snd_strerror(err));
		return -1;
	}

	/* alloc memory space for hardware parameter structure*/
	if ((err = snd_pcm_hw_params_malloc(&params)) < 0){
		printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Initialize the sound card parameter structure with default data */
	if ((err = snd_pcm_hw_params_any(playback_handle, params)) < 0) {
		printf("failed to initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the access parameters */
	if ((err = snd_pcm_hw_params_set_access(playback_handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf("cannot set access type (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the data format */
	if ((err = snd_pcm_hw_params_set_format(playback_handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sample rate of the sound card */
	if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, params, &cmdOpt->rate, 0)) < 0)
	{
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sound card channel */
	if ((err = snd_pcm_hw_params_set_channels(playback_handle, params, 1)) < 0) {
		printf("cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}

	// frames = 1152;
	// if ((err = snd_pcm_hw_params_set_period_size_near(playback_handle, params, &frames, 0)) < 0) {
	// 	printf("cannot set period size (%s)\n", snd_strerror(err));
	// 	return -1;
	// }

	if ((err = snd_pcm_hw_params(playback_handle, params)) < 0) {
		printf("cannot set parameters (%s)\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_hw_params_free(params);
	if ((err = snd_pcm_prepare(playback_handle)) < 0) {
		printf("cannont prepare audio interface for use (%s)\n", snd_strerror(err));
		return -1;
	}

	return 0;
}


int main(int argc, char **argv)
{
    CmdLineOptions options = parse_command_line(argc, argv);

    if (options.display_help)
    {
        print_usage(argv[0]);
        exit(0);
    }

    if (sound_init(&options) < 0)
	{
		printf("Failed to init sound card\n");
		return -1;
	}

    do
    {
        s16 buffer[SAMPLE_RATE] = {0};
        if (strcmp(options.wave_type, "sine") == 0)
        {
            snd_pcm_writei(playback_handle, sine_wave(buffer, SAMPLE_RATE, options.freq), SAMPLE_RATE);
        }
        else if (strcmp(options.wave_type, "square") == 0)
        {
            snd_pcm_writei(playback_handle, square_wave(buffer, SAMPLE_RATE, options.freq), SAMPLE_RATE);
        }
    } while (options.should_loop);

    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);

    return 0;
}