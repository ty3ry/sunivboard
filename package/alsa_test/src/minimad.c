/*
 * libmad - MPEG audio decoder library
 * Copyright (C) 2000-2004 Underbit Technologies, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * $Id: minimad.c,v 1.4 2004/01/23 09:41:32 rob Exp $
 */

#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <alsa/asoundlib.h>
#include <mad.h>

#if 1

#define BUFSIZE 10 * 1024
struct buffer
{
    FILE *fp;                    /*file pointer*/
    unsigned int flen;           /*file length*/
    unsigned char fbuf[BUFSIZE]; /*buffer*/
    unsigned int fbsize;         /*indeed size of buffer*/
};
typedef struct buffer mp3_file;

static enum mad_flow input(void *data,
                           struct mad_stream *stream)
{
    mp3_file *mp3fp;
    int unproc_data_size; /*the unprocessed data's size*/
    int copy_size;
    int read_size;

    mp3fp = (mp3_file *)data;
    //记录剩余的未被解码的MPEG帧的数据的字节数
    unproc_data_size = stream->bufend - stream->next_frame;
    // fprintf(stderr,"unproc_data_size = %d\n", unproc_data_size);
    //将剩余未被解码的数据拷贝到buff的头部准备进行下次解码
    memcpy(mp3fp->fbuf, mp3fp->fbuf + mp3fp->fbsize - unproc_data_size, unproc_data_size);
    copy_size = mp3fp->fbsize - unproc_data_size;
    //继续从文件中读取数据，将buff填满。
    read_size = fread(mp3fp->fbuf + unproc_data_size, 1, copy_size, mp3fp->fp);
    // fprintf(stderr,"read_size = %d\n", read_size);

    mad_stream_buffer(stream, mp3fp->fbuf, read_size + unproc_data_size);
    if (0 != read_size)
    {
        return MAD_FLOW_CONTINUE;
    }

    else
    {
        fprintf(stderr, "MAD_FLOW_STOP\n");
        return MAD_FLOW_STOP;
    }
}

/*
 * This is perhaps the simplest example use of the MAD high-level API.
 * Standard input is mapped into memory via mmap(), then the high-level API
 * is invoked with three callbacks: input, output, and error. The output
 * callback converts MAD's high-resolution PCM samples to 16 bits, then
 * writes them to standard output in little-endian, stereo-interleaved
 * format.
 */

static int decode(mp3_file *);
#define BUFF_SIZE 8192 * 11
int main(int argc, char *argv[])
{

    mp3_file *mp3fp = (mp3_file *)malloc(sizeof(mp3_file));
    if (mp3fp == NULL)
    {
        fprintf(stderr, "malloc mp3_file err\n");
        return -1;
    }
    if (argc != 2)
    {
        fprintf(stderr, "usage: ./mp3play filename\n");
        return -1;
    }
    printf("%s\n", argv[1]);
    mp3fp->fp = fopen(argv[1], "rb");
    if (mp3fp->fp == NULL)
    {
        fprintf(stderr, "usage: ./mp3play filename\n");
        return -1;
    }
    mp3fp->fbsize = BUFSIZE;

    decode(mp3fp);

    fclose(mp3fp->fp);
    free(mp3fp);
    mp3fp = NULL;

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

/* open a PCM device */

snd_pcm_t *playback_handle;
int open_device()
{
    int err;
    snd_pcm_hw_params_t *hw_params;
    char *pcm_name = "plughw:0,0";
    int rate = 44100;
    int channels = 2;

    if ((err = snd_pcm_open(&playback_handle,
                            pcm_name, SND_PCM_STREAM_PLAYBACK, 0)) < 0)
    {
        printf("cannot open audio device %s (%s)\n", pcm_name, snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0)
    {
        printf("cannot allocate hardware parameter structure (%s)\n",
               snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0)
    {
        printf("cannot initialize hardware parameter structure (%s)\n",
               snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params,
                                            SND_PCM_ACCESS_RW_INTERLEAVED)) < 0)
    {
        printf("cannot set access type (%s)\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_format(playback_handle,
                                            hw_params, SND_PCM_FORMAT_S16_LE)) < 0)
    {
        printf("cannot set sample format (%s)\n", snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params_set_rate_near(playback_handle,
                                               hw_params, &rate, 0)) < 0)
    {
        printf("cannot set sample rate (%s)\n", snd_strerror(err));
        return -1;
    }

    if ((err = snd_pcm_hw_params_set_channels(playback_handle,
                                              hw_params, channels)) < 0)
    {
        printf("cannot set channel count (%s)\n", snd_strerror(err));
        return -1;
    }
    if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0)
    {
        printf("cannot set parameters (%s)\n",
               snd_strerror(err));
        return -1;
    }
    snd_pcm_hw_params_free(hw_params);
    if ((err = snd_pcm_prepare(playback_handle)) < 0)
    {
        printf("cannot prepare audio interface for use (%s)\n", snd_strerror(err));
        return -1;
    }

    return 0;
}

/*
 * This is the output callback function. It is called after each frame of
 * MPEG audio data has been completely decoded. The purpose of this callback
 * is to output (or play) the decoded PCM audio.
 */

static enum mad_flow output(void *data,
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
    snd_pcm_writei(playback_handle, OutputPtr, n);
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

/*
 * This is the error callback function. It is called whenever a decoding
 * error occurs. The error is indicated by stream->error; the list of
 * possible MAD_ERROR_* errors can be found in the mad.h (or stream.h)
 * header file.
 */

static enum mad_flow error(void *data,
                           struct mad_stream *stream,
                           struct mad_frame *frame)
{
    struct buffer *buffer = data;

    fprintf(stderr, "decoding error 0x%04x (%s) at byte offset %u\n",
            stream->error, mad_stream_errorstr(stream),
            stream->this_frame);

    /* return MAD_FLOW_BREAK here to stop decoding (and propagate an error) */

    return MAD_FLOW_CONTINUE;
}

/*
 * This is the function called by main() above to perform all the decoding.
 * It instantiates a decoder object and configures it with the input,
 * output, and error callback functions above. A single call to
 * mad_decoder_run() continues until a callback function returns
 * MAD_FLOW_STOP (to stop decoding) or MAD_FLOW_BREAK (to stop decoding and
 * signal an error).
 */

static int decode(mp3_file *mp3_fp)
{
    struct mad_decoder decoder;
    int result;
    mp3_file *buffer = mp3_fp;

    /* initialize our private message structure */

    /* configure input, output, and error functions */

    mad_decoder_init(&decoder, (void *)buffer,
                     input, 0 /* header */, 0 /* filter */, output,
                     error, 0 /* message */);
    open_device();
    /* start decoding */

    result = mad_decoder_run(&decoder, MAD_DECODER_MODE_SYNC);

    /* release the decoder */

    mad_decoder_finish(&decoder);
    return result;
}
#endif