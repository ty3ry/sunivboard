#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <mad.h>
#include <id3tag.h>
#include <alsa/asoundlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_BUFFER_SIZE	(5 * 8192)
#define OUTPUT_BUFFER_SIZE	(1152 * 8)	
//#define OUTPUT_BUFFER_SIZE	(1152 * 4)	

#define SAMPLE_RATE 48000
#define AMPLITUDE 10000

snd_pcm_t			*handle;
snd_pcm_uframes_t	frames;
pthread_mutex_t		lock;
pthread_cond_t		empty;
pthread_cond_t		full;
int					finished;
int					rate;
unsigned char		OutputBuffer[OUTPUT_BUFFER_SIZE];

void check(int ret)
{
    if (ret < 0)
    {
        fprintf(stderr, "error: %s (%d)\n", snd_strerror(ret), ret);
        exit(1);
    }
}

static unsigned short MadFixedToUshort(mad_fixed_t Fixed)
{
	Fixed = Fixed >> (MAD_F_FRACBITS - 15);
	return((unsigned short)Fixed);
}

int snd_init(void)
{
	int err;
	snd_pcm_hw_params_t *params;
	printf("mad mp3 player %s\n", __TIME__);

#if 1
	/* open the pcm device */
	if ((err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
		printf("failed to open pcm device \"default\" (%s)\n", snd_strerror(err));
		return -1;
	}

	/* alloc memory space for hardware parameter structure*/
	// if ((err = snd_pcm_hw_params_alloca(&params)) < 0) {
	// 	printf("cannot allocate hardware parameter structure (%s)\n", snd_strerror(err));
	// 	return -1;
	// }

	snd_pcm_hw_params_alloca(&params);

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
	if ((err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S32_LE)) < 0) {
	//if ((err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE)) < 0) {
	//if ((err = snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_U16_LE)) < 0) {
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}

	/* Set the sample rate of the sound card to 44100 */
	rate = 44100;
	if ((err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0)) < 0) {
		printf("cannot set sample format (%s)\n", snd_strerror(err));
		return -1;
	}
	printf("rate: %d\n", rate);

	/* Set the sound card channel to 2 */
	int channels = 2;
	if ((err = snd_pcm_hw_params_set_channels(handle, params, channels)) < 0) {
		printf("cannot set channel count (%s)\n", snd_strerror(err));
		return -1;
	}

	frames = 1152;
	//frames = 512;
	if ((err = snd_pcm_hw_params_set_periods(handle, params, 10, 0)) < 0) {
		printf("cannot set period size (%s)\n", snd_strerror(err));
		return -1;
	}
	printf("frames: %d\n", frames);
		
	if ((err = snd_pcm_hw_params(handle, params)) < 0) {
		printf("cannot set parameters (%s)\n", snd_strerror(err));
		return -1;
	}

	// snd_pcm_hw_params_free(params);
	if ((err = snd_pcm_prepare(handle)) < 0) {
		printf("cannont prepare audio interface for use (%s)\n", snd_strerror(err));
		return -1;
	}
#else
	snd_pcm_hw_params_alloca(&params);

    check(snd_pcm_hw_params_any(handle, params));
    check(snd_pcm_hw_params_set_access(handle, params, SND_PCM_ACCESS_RW_INTERLEAVED));
    check(snd_pcm_hw_params_set_format(handle, params, SND_PCM_FORMAT_S16_LE));
    check(snd_pcm_hw_params_set_channels(handle, params, 1));
    check(snd_pcm_hw_params_set_rate(handle, params, SAMPLE_RATE, 0));
    check(snd_pcm_hw_params_set_periods(handle, params, 10, 0));
    check(snd_pcm_hw_params_set_period_time(handle, params, 100000, 0)); // 0.1 seconds period time

    check(snd_pcm_hw_params(handle, params));
#endif
	return 0;
}

/* Decoding function */
void *decode(void *pthread_arg)
{
	struct mad_stream 		Stream;
	struct mad_frame 		Frame;
	struct mad_synth 		Synth;
	//mad_timer_t				Timer;
	unsigned char 			Mp3_InputBuffer[INPUT_BUFFER_SIZE];
	unsigned char			*OutputPtr = OutputBuffer;
	unsigned char *const	OutputBufferEnd = OutputBuffer + OUTPUT_BUFFER_SIZE;
	int						i, err;
	int						fd = (int)pthread_arg;

	printf("Init mad \n");
	/* libmad initialization */
	mad_stream_init(&Stream);
	mad_frame_init(&Frame);
	mad_synth_init(&Synth);

	printf("start decoding\n");

	/* start decoding */
	do {
		/* If the buffer is empty or less than one frame of data, fill the buffer with data */
		if(Stream.buffer == NULL || Stream.error == MAD_ERROR_BUFLEN) {
			size_t 			BufferSize;		/* buffer size*/
			size_t			Remaining;		/* frame remaining data */
			unsigned char	*BufferStart;	/* head pointer */

			if (Stream.next_frame != NULL) {
				printf("still in stock\n");

				/* Add the remaining undecoded data to the buffer this time */
				Remaining = Stream.bufend - Stream.next_frame;
				memmove(Mp3_InputBuffer, Stream.next_frame, Remaining);
				BufferStart = Mp3_InputBuffer + Remaining;
				BufferSize = INPUT_BUFFER_SIZE - Remaining;
			} else {
				printf("exhausted\n");

				/* The buffer address is set, but not yet filled with data */
				BufferSize = INPUT_BUFFER_SIZE;
				BufferStart = Mp3_InputBuffer;
				Remaining = 0;
			}

			/* read data from file and fill buffer */
			BufferSize = read(fd, BufferStart, BufferSize);
			if (BufferSize <= 0) {
				printf("file read failed\n");
				exit(-1);
			}

			mad_stream_buffer(&Stream, Mp3_InputBuffer, BufferSize + Remaining);
			Stream.error = 0;
		}

		if (err = mad_frame_decode(&Frame, &Stream)) {
			printf("decoding error: %x\n", Stream.error);

			if (MAD_RECOVERABLE(Stream.error)) {
				printf("recoverable\n");
				continue;
			} else {
				if (Stream.error == MAD_ERROR_BUFLEN) {
					printf("buffer The data is less than one frame and needs to be filled\n");

					continue; /* buffer Decoded light, need to continue filling */
				} else if (Stream.error == MAD_ERROR_LOSTSYNC) {
					printf("out of sync\n");

					int tagsize;
					tagsize = id3_tag_query(Stream.this_frame, Stream.bufend - Stream.this_frame);
					if (tagsize > 0) {
						mad_stream_skip(&Stream, tagsize);
					}
					continue;
				} else {
					printf("critical error, stop decoding\n");
					exit(-1);
				}
			}
		}
		/* Set the playback time of each frame */
		//mad_timer_add(&Timer, Frame.header.duration);
		/* decode into audio data */
		mad_synth_frame(&Synth, &Frame);


		pthread_mutex_lock(&lock);
		if (finished) {
			pthread_cond_wait(&empty, &lock);
		}
		/*printf("Synth.pcm.length: %d\n", Synth.pcm.length);
		printf("samplerate: %d\n", Synth.pcm.samplerate);
		 if (rate != Synth.pcm.samplerate) {
			if ((err = snd_pcm_hw_params_set_rate_near(handle, params, &rate, 0)) < 0) {
				printf("cannot set sample format (%s)\n", snd_strerror(err));
				return -1;
			}
			rate = Synth.pcm.samplerate;
			if ((err = snd_pcm_hw_params(handle, params)) < 0) {
				printf("cannot set parameters (%s)\n", snd_strerror(err));
				return -1;
			}
			if ((err = snd_pcm_prepare(handle)) < 0) {
				printf("cannont prepare audio interface for use (%s)\n", snd_strerror(err));
				return -1;
			}
		}*/
		
		/* Convert the decoded audio data into 16-bit data */
		for (i = 0; i < Synth.pcm.length; i++) {
			//unsigned short Sample;
			signed int Sample;
			Sample = Synth.pcm.samples[0][i];
			//Sample = MadFixedToUshort(Synth.pcm.samples[0][i]);
			*(OutputPtr++) = (Sample & 0xff);
			*(OutputPtr++) = (Sample >> 8);
			*(OutputPtr++) = (Sample >> 16);
			*(OutputPtr++) = (Sample >> 24);

			if (MAD_NCHANNELS(&Frame.header) == 2) {
				Sample = Synth.pcm.samples[1][i];
				//Sample = MadFixedToUshort(Synth.pcm.samples[1][i]);
				*(OutputPtr++) = (Sample & 0xff);
				*(OutputPtr++) = (Sample >> 8);
				*(OutputPtr++) = (Sample >> 16);
				*(OutputPtr++) = (Sample >> 24);
			}

			/* The output buffer is full */
			if (OutputPtr >= OutputBufferEnd) {
				OutputPtr = OutputBuffer;
				finished = 1;
				pthread_mutex_unlock(&lock);
				pthread_cond_signal(&full);
			}
		}
	} while(1);

	mad_synth_finish(&Synth);
	mad_frame_finish(&Frame);
	mad_stream_finish(&Stream);
	
	return;
}

long writebuf(snd_pcm_t *handle, char *buf, long len, size_t *frames)
{
	long r;
	while (len > 0) {
		r = snd_pcm_writei(handle, buf, 4);
		if (r == -EAGAIN)
			continue;
		if (r < 4) {
			printf("write = %li\n", r);
		}
		//printf("write = %li\n", r);
		if (r < 0)
			return r;
		// showstat(handle, 0);
		//buf += r * 4;
		buf += r * 8;
		len -= r;
		*frames += r;
	}
	return 0;
}

int main(int argc, char **argv)
{
	int			mp3_fd;
	int			err = -1;
	pthread_t	decode_thread;

	if (!argv[1]) {
		printf("plz input a mp3 file\n");
		return -1;
	}

	mp3_fd = open(argv[1], O_RDONLY);
	if (!mp3_fd) {
		perror("failed to open file ");
		return -1;
	}

	if (snd_init() < 0) {
		printf("faile to init snd card\n");
		return -1;
	}

	printf("Init thread and mutex \n");

	pthread_mutex_init(&lock, NULL);
	//printf("mutex init done\n");
	pthread_cond_init(&empty);
	//printf("cond init empty done\n");
	pthread_cond_init(&full);
	//printf("Going to create thread decode\n");
	//sleep(1);
	pthread_create(&decode_thread, NULL, decode, (void*)mp3_fd);

	printf("init thread and mutex done!\n");
	
	while (1) {
		pthread_mutex_lock(&lock);
		if (!finished) {
			pthread_cond_wait(&full, &lock);
		}
		
		//if ((err = snd_pcm_writei(handle, OutputBuffer, frames)) < 0) {
		size_t count;
		if ((err = writebuf(handle, OutputBuffer, 1152, &count)) < 0) {
			printf("write error: %s, errno: %d\n", snd_strerror(err), err);
			if (err == -EPIPE) {
				int errb;
				errb = snd_pcm_recover(handle, err, 0);
				if (errb < 0) {
					printf("failed to recover from underrun\n");
					return -1;
				} else {
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