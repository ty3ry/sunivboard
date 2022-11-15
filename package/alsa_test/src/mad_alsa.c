/**
 * file: alsa_mad.c
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <mad.h>
#include <id3tag.h>
#include <alsa/asoundlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE (5 * 8192)
#define OUTPUT_BUFFER_SIZE (1152 * 8)

#define	SOUND_PCM_FORMAT	(SND_PCM_FORMAT_S16_LE)

typedef struct
{
    bool display_help;
    char *path;
	char *playback_name;
    unsigned int sample;
	int channel;
} CmdLineOptions;

snd_pcm_t *handle;
snd_pcm_uframes_t frames;
pthread_mutex_t lock;
pthread_cond_t empty;
pthread_cond_t full;
int finished;
int rate;
unsigned char OutputBuffer[OUTPUT_BUFFER_SIZE];


void print_usage(char *prog_name)
{
    printf(
        "Usage: %s [mp3_file] \n\n"
        "Options:\n"
        "    -h, --help          Display this message\n"
		"    -d, --device        Set device card, if none set to \"default\" \n "
        "    -p  --path PATH     mp3 file location path \n"
		"    -c  channel    	 set channel \n"
        "    -s  sampling_rate   sampling rate\n"
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
        .sample = 44100,
		.playback_name = "default",
		.channel = 2
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
			if ( !expect_int(arg, &options.sample) ) {
				fprintf(stderr, "error: Sampling rate not valid\n", arg);
                exit(1);
			}
		}
		else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--path") == 0)
		{
			if (i + 1 == argc)
            {
                fprintf(stderr, "error: Argument to option '%s' missing\n", arg);
                exit(1);
            }
			options.path = argv[++i];
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


int sound_init(CmdLineOptions *cmdOpt)
{
	int err;
	snd_pcm_hw_params_t *params;

	/* open the pcm device */
	if ((err = snd_pcm_open(&handle, cmdOpt->playback_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
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
	if ((err = snd_pcm_hw_params_any(handle, params)) < 0) {
		printf("failed to initialize hardware parameter structure (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the access parameters of the sound card to interleaved access */
	if ((err = snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
		printf("cannot set access type (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the data format of the sound card to signed 32-bit little endian */
	if ((err = snd_pcm_hw_params_set_format(handle, params, SOUND_PCM_FORMAT)) < 0) {
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sample rate of the sound card to 44100 */
	if ((err = snd_pcm_hw_params_set_rate_near(handle, params, &cmdOpt->sample, 0)) < 0)
	{
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sound card channel */
	if ((err = snd_pcm_hw_params_set_channels(handle, params, cmdOpt->channel)) < 0) {
		printf("cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}

	frames = 1152;
	if ((err = snd_pcm_hw_params_set_period_size_near(handle, params, &frames, 0)) < 0) {
		printf("cannot set period size (%s)\n", snd_strerror(err));
		return -1;
	}

	if ((err = snd_pcm_hw_params(handle, params)) < 0) {
		printf("cannot set parameters (%s)\n", snd_strerror(err));
		return -1;
	}

	snd_pcm_hw_params_free(params);
	if ((err = snd_pcm_prepare(handle)) < 0) {
		printf("cannont prepare audio interface for use (%s)\n", snd_strerror(err));
		return -1;
	}

	return 0;
}

/*
 * The following utility routine performs simple rounding, clipping, and
 * scaling of MAD's high-resolution samples down to 16 bits. It does not
 * perform any dithering or noise shaping, which would be recommended to
 * obtain any exceptional audio quality. It is therefore not recommended to
 * use this routine if high-quality output is desired.
 */
static inline signed int scale(mad_fixed_t sample)
{
    /* round */
    sample += (1L << (MAD_F_FRACBITS - 16));

    /* clip */
    if (sample >= MAD_F_ONE)
        sample = MAD_F_ONE - 1;
    else if (sample < -MAD_F_ONE)
        sample = -MAD_F_ONE;

    /* quantize */
    return sample >> (MAD_F_FRACBITS + 1 - 16);
}

/* Decoding function */
void *decode(void *pthread_arg)
{
	struct mad_stream Stream;
	struct mad_frame Frame;
	struct mad_synth Synth;
	// mad_timer_t				Timer;
	unsigned char Mp3_InputBuffer[INPUT_BUFFER_SIZE];
	unsigned char *OutputPtr = OutputBuffer;
	unsigned char *const OutputBufferEnd = OutputBuffer + OUTPUT_BUFFER_SIZE;
	int i, err;
	int fd = (int)pthread_arg;

	/* libmad initialization */
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);

	/* start decoding */
	do
	{
		/* If the buffer is empty or less than one frame of data, fill the buffer with data */
		if (Stream.buffer == NULL || Stream.error == MAD_ERROR_BUFLEN)
		{
			size_t BufferSize;			/* buffer size*/
			size_t Remaining;			/* frame remaining data */
			unsigned char *BufferStart; /* head pointer */

			if (Stream.next_frame != NULL)
			{
				printf("still in stock\n");

				/* Add the remaining undecoded data to the buffer this time */
				Remaining = Stream.bufend - Stream.next_frame;
				memmove(Mp3_InputBuffer, Stream.next_frame, Remaining);
				BufferStart = Mp3_InputBuffer + Remaining;
				BufferSize = INPUT_BUFFER_SIZE - Remaining;
			}
			else
			{
				printf("exhausted\n");

				/* The buffer address is set, but not yet filled with data */
				BufferSize = INPUT_BUFFER_SIZE;
				BufferStart = Mp3_InputBuffer;
				Remaining = 0;
			}

			/* read data from file and fill buffer */
			BufferSize = read(fd, BufferStart, BufferSize);
			if (BufferSize <= 0)
			{
				printf("file read failed\n");
				exit(-1);
			}

			mad_stream_buffer(&Stream, Mp3_InputBuffer, BufferSize + Remaining);
			Stream.error = 0;
		}

		if (err = mad_frame_decode(&Frame, &Stream))
		{
			//printf("decoding error: %x\n", Stream.error);

			if (MAD_RECOVERABLE(Stream.error))
			{
				printf("recoverable\n");
				continue;
			}
			else
			{
				if (Stream.error == MAD_ERROR_BUFLEN)
				{
					printf("buffer The data is less than one frame and needs to be filled\n");

					continue; /* buffer Decoded light, need to continue filling */
				}
				else if (Stream.error == MAD_ERROR_LOSTSYNC)
				{
					printf("out of sync\n");

					int tagsize;
					tagsize = id3_tag_query(Stream.this_frame, Stream.bufend - Stream.this_frame);
					if (tagsize > 0)
					{
						mad_stream_skip(&Stream, tagsize);
					}
					continue;
				}
				else
				{
					printf("critical error, stop decoding\n");
					exit(-1);
				}
			}
		}
		/* Set the playback time of each frame */
		// mad_timer_add(&Timer, Frame.header.duration);
		/* decode into audio data */
		mad_synth_frame(&Synth, &Frame);

		pthread_mutex_lock(&lock);
		if (finished)
		{
			pthread_cond_wait(&empty, &lock);
		}

		unsigned int nchannels, nsamples;
    	mad_fixed_t const *left_ch, *right_ch;

		nchannels = Synth.pcm.channels;
		nsamples = Synth.pcm.length;
		left_ch = Synth.pcm.samples[0];
		right_ch = Synth.pcm.samples[1];

		while(nsamples--)
		{
			signed int sample;

			/* output sample(s) in 16-bit signed little-endian PCM */

			sample = scale(*left_ch++);

			*(OutputPtr++) = sample >> 0;
			*(OutputPtr++) = sample >> 8;
			if (nchannels == 2)
			{
				sample = scale(*right_ch++);
				*(OutputPtr++) = sample >> 0;
				*(OutputPtr++) = sample >> 8;
			}
		}

		OutputPtr = OutputBuffer;
		finished = 1;
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&full);

	} while (1);

	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);

	return;
}


int main(int argc, char *argv[])
{
	CmdLineOptions options = parse_command_line(argc, argv);
	int mp3_fd;
	int err = -1;
	pthread_t decode_thread;

	if (options.display_help)
	{
		print_usage(argv[0]);
		exit(0);
	}

	mp3_fd = open(options.path, O_RDONLY);
	if (!mp3_fd)
	{
		perror("failed to open file ");
		return -1;
	}

	if (sound_init(&options) < 0)
	{
		printf("Failed to init sound card\n");
		return -1;
	}

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&empty);
	pthread_cond_init(&full);
	pthread_create(&decode_thread, NULL, decode, (void *)mp3_fd);

	while (1)
	{
		pthread_mutex_lock(&lock);
		if (!finished)
		{
			pthread_cond_wait(&full, &lock);
		}

		if ((err = snd_pcm_writei(handle, OutputBuffer, 1151)) < 0) {
			printf("write error: %s, errno: %d\n", snd_strerror(err), err);
			if (err == -EPIPE)
			{
				int errb;
				errb = snd_pcm_recover(handle, err, 0);
				if (errb < 0)
				{
					printf("failed to recover from underrun\n");
					return -1;
				}
				else
				{
					printf("recover\n");
				}
				snd_pcm_prepare(handle);
			}
		}

		finished = 0;
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&empty);
	}

	return 0;
}