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
#define	SOUND_PCM_FORMAT	(SND_PCM_FORMAT_S16_LE)

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


void print_usage(char *progname)
{
    printf(
        "Usage: %s [OPTIONS]...\n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
        "    -d, device          Device / Card used for pcm playback\n"
        "    -l, --loop          Play sound in an infinite loop\n"
        "    -s  sampling_rate   sampling rate\n"
        "    -r  --raw PATH      Play a raw pcm file\n"
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
    char *playback_name;
    char *raw_path;
    unsigned int channel;
    unsigned int rate;
    int fade_ms;
} CmdLineOptions;

CmdLineOptions parse_command_line(int argc, char **argv)
{
    CmdLineOptions options = {
        .channel = 2,
        .playback_name = "default",
        .rate = SAMPLE_RATE
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
            options.playback_name = argv[++i];
        }
        else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--raw") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            options.raw_path = argv[++i];
        }
        else if (strcmp(arg, "-c") == 0)
        {
            if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
            arg = argv[++i];
            if (!expect_int(arg, &options.channel) || (options.channel != 2 && options.channel != 1))
            {
                fprintf(stderr, "error: Channel value not valid , set 2 for stereo and 1 for mono output\n", arg);
                exit(1);
            }
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
	if ((err = snd_pcm_hw_params_set_channels(playback_handle, params, cmdOpt->channel)) < 0) {
		printf("cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}


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


char *read_entire_file(char *path, size_t *size)
{
    FILE *f = fopen(path, "rb");
    if (!f)
    {
        fprintf(stderr, "error: Unable to open file `%s`, please provide a valid file\n", path);
        exit(1);
    }
    if (fseek(f, 0, SEEK_END) != 0)
    {
        fprintf(stderr, "error: Unable to perform seek operation on file `%s`\n", path);
        fprintf(stderr, "1\n");
        fclose(f);
        exit(1);
    }
    long file_size = ftell(f);
    if (file_size < 0)
    {
        fprintf(stderr, "error: Unable to get file size for `%s`\n", path);
        fclose(f);
        exit(1);
    }
    if (fseek(f, 0, SEEK_SET) != 0)
    {
        fprintf(stderr, "error: Unable to perform seek operation on file `%s`\n", path);
        fclose(f);
        exit(1);
    }
    char *contents = (char *)malloc(file_size);
    if (fread(contents, 1, file_size, f) != file_size)
    {
        fprintf(stderr, "error: Unable to read file `%s`\n", path);
        fclose(f);
        free(contents);
        exit(1);
    }
    if (size)
    {
        *size = file_size;
    }
    return contents;
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
		printf("Failed to init sound card (%s)\n", options.playback_name);
		return -1;
	}
    
    int fade_in_samples = options.fade_ms * (SAMPLE_RATE / 1000);
    s16 *samples;
    size_t file_size = 0;
    int sample_count = 0;
    if (options.raw_path)
    {
        samples = (s16 *)read_entire_file(options.raw_path, &file_size);
        sample_count = file_size / sizeof(s16);
    }

    //printf("file size : %d \n", file_size);

    do
    {
        int sample_index = 0;

        for (int offset = 0; offset < file_size; offset += SAMPLE_RATE * sizeof(s16))
        {
            int chunk_sample_count = SAMPLE_RATE;
            int chunk_size = chunk_sample_count * sizeof(s16);
            if (offset + chunk_size > file_size)
            {
                chunk_sample_count = (file_size - offset) / sizeof(s16);
            }

            printf("chunk sample count : %d \n", chunk_sample_count);
            snd_pcm_writei(playback_handle, (u8 *)samples + offset, chunk_sample_count);
        }
    } while (1);

    snd_pcm_drain(playback_handle);
    snd_pcm_close(playback_handle);

    return 0;
}