/**
 * https://lauri.xn--vsandi-pxa.com/2013/12/implementing-mp3-player.en.html
 * 
 * compile : 
 * $ gcc -o player player.c -lpulse -lpulse-simple -lmad -g
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <mad.h>
// #include <pulse/simple.h>
// #include <pulse/error.h>
#include <alsa/asoundlib.h>


#define SAMPLE_RATE 44100
#define AMPLITUDE 10000

//pa_simple *device = NULL;

int ret = 1;
int error;

snd_pcm_t *pcm_handle;
struct mad_stream mad_stream;
struct mad_frame mad_frame;
struct mad_synth mad_synth;

void output(struct mad_header const *header, struct mad_pcm *pcm);
static enum mad_flow output1(void *data,
                            struct mad_header const *header,
                            struct mad_pcm *pcm);
void check(int ret);

int main(int argc, char **argv) {
    // Parse command-line arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s [filename.mp3]", argv[0]);
        return 255;
    }

    // Set up PulseAudio 16-bit 44.1kHz stereo output
    // static const pa_sample_spec ss = { .format = PA_SAMPLE_S16LE, .rate = 44100, .channels = 2 };
    // if (!(device = pa_simple_new(NULL, "MP3 player", PA_STREAM_PLAYBACK, NULL, "playback", &ss, NULL, NULL, &error))) {
    //     printf("pa_simple_new() failed\n");
    //     return 255;
    // }
    /**TODO: set up alsa */
   
    check(snd_pcm_open(&pcm_handle, "default", SND_PCM_STREAM_PLAYBACK, 0));

    snd_pcm_hw_params_t *hw_params;
    snd_pcm_hw_params_alloca(&hw_params);
    //	check(snd_pcm_hw_params_malloc(&hw_params));

    check(snd_pcm_hw_params_any(pcm_handle, hw_params));
    check(snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED));
    check(snd_pcm_hw_params_set_format(pcm_handle, hw_params, SND_PCM_FORMAT_S16_LE));
    check(snd_pcm_hw_params_set_channels(pcm_handle, hw_params, 2));
    check(snd_pcm_hw_params_set_rate(pcm_handle, hw_params, SAMPLE_RATE, 0));
    check(snd_pcm_hw_params_set_periods(pcm_handle, hw_params, 10, 0));
    check(snd_pcm_hw_params_set_period_time(pcm_handle, hw_params, 100000, 0)); // 0.1 seconds period time

    check(snd_pcm_hw_params(pcm_handle, hw_params));

    // Initialize MAD library
    mad_stream_init(&mad_stream);
    mad_synth_init(&mad_synth);
    mad_frame_init(&mad_frame);

    // Filename pointer
    char *filename = argv[1];

    // File pointer
    FILE *fp = fopen(filename, "r");
    int fd = fileno(fp);

    // Fetch file size, etc
    struct stat metadata;
    if (fstat(fd, &metadata) >= 0) {
        printf("File size %d bytes\n", (int)metadata.st_size);
    } else {
        printf("Failed to stat %s\n", filename);
        fclose(fp);
        return 254;
    }

    // Let kernel do all the dirty job of buffering etc, map file contents to memory
    char *input_stream = mmap(0, metadata.st_size, PROT_READ, MAP_SHARED, fd, 0);

    // Copy pointer and length to mad_stream struct
    mad_stream_buffer(&mad_stream, input_stream, metadata.st_size);

    // Decode frame and synthesize loop
    while (1) {

        // Decode frame from the stream
        if (mad_frame_decode(&mad_frame, &mad_stream)) {
            if (MAD_RECOVERABLE(mad_stream.error)) {
                continue;
            } else if (mad_stream.error == MAD_ERROR_BUFLEN) {
                continue;
            } else {
                break;
            }
        }
        // Synthesize PCM data of frame
        mad_synth_frame(&mad_synth, &mad_frame);
        output1(NULL, &mad_frame.header, &mad_synth.pcm);
    }

    // Close
    fclose(fp);

    // Free MAD structs
    mad_synth_finish(&mad_synth);
    mad_frame_finish(&mad_frame);
    mad_stream_finish(&mad_stream);

    /**TODO: alsa close device */
    // Close PulseAudio output
    // if (device)
    //     pa_simple_free(device);
    check(snd_pcm_drain(pcm_handle));
    check(snd_pcm_close(pcm_handle));

    return EXIT_SUCCESS;
}

// Some helper functions, to be cleaned up in the future
int scale(mad_fixed_t sample) {
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


static enum mad_flow output1(void *data,
                            struct mad_header const *header,
                            struct mad_pcm *pcm)
{
    unsigned int nchannels, nsamples, n;
    mad_fixed_t const *left_ch, *right_ch;

    /* pcm->samplerate contains the sampling frequency */

    nchannels = pcm->channels;
    n = nsamples = pcm->length;
    left_ch = pcm->samples[0];
    right_ch = pcm->samples[1];

    unsigned char Output[6912], *OutputPtr;
    int fmt, wrote, speed, exact_rate, err, dir;

    OutputPtr = Output;

    while (nsamples--)
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

    OutputPtr = Output;
    snd_pcm_writei(pcm_handle, OutputPtr, n);
    OutputPtr = Output;

    return MAD_FLOW_CONTINUE;

#if 0
  unsigned int nchannels, nsamples;
  mad_fixed_t const *left_ch, *right_ch;

  /* pcm->samplerate contains the sampling frequency */

  nchannels = pcm->channels;
  nsamples  = pcm->length;
  left_ch   = pcm->samples[0];
  right_ch  = pcm->samples[1];

  while (nsamples--) {
    signed int sample;

    /* output sample(s) in 16-bit signed little-endian PCM */

    sample = scale(*left_ch++);
    putchar((sample >> 0) & 0xff);
    putchar((sample >> 8) & 0xff);

    if (nchannels == 2) {
      sample = scale(*right_ch++);
      putchar((sample >> 0) & 0xff);
      putchar((sample >> 8) & 0xff);
    }
  }

  return MAD_FLOW_CONTINUE;
#endif
}

void output(struct mad_header const *header, struct mad_pcm *pcm) {
    register int nsamples = pcm->length;
    mad_fixed_t const *left_ch = pcm->samples[0], *right_ch = pcm->samples[1];
    static char stream[1152*4];
    if (pcm->channels == 2) {
        while (nsamples--) {
            signed int sample;
            sample = scale(*left_ch++);
            stream[(pcm->length-nsamples)*4 ] = ((sample >> 0) & 0xff);
            stream[(pcm->length-nsamples)*4 +1] = ((sample >> 8) & 0xff);
            sample = scale(*right_ch++);
            stream[(pcm->length-nsamples)*4+2 ] = ((sample >> 0) & 0xff);
            stream[(pcm->length-nsamples)*4 +3] = ((sample >> 8) & 0xff);
        }

        /**TODO: set write ALSA PCM */
        // if (pa_simple_write(device, stream, (size_t)1152*4, &error) < 0) {
        //     fprintf(stderr, "pa_simple_write() failed: %s\n", pa_strerror(error));
        //     return;
        // }
        snd_pcm_writei(pcm_handle, stream, 1151);

    } else {
        printf("Mono not supported!");
    }
}


void check(int ret)
{
    if (ret < 0)
    {
        fprintf(stderr, "error: %s (%d)\n", snd_strerror(ret), ret);
        exit(1);
    }
}