#include "ALSAAudioSink.h"

int ALSAAudioSink::MixerInit(void)
{
    /** add mixer */
    const char *device_name = "hw:1";
    const char *mixer_ctrl_name = "Master";
    snd_mixer_selem_channel_id_t ch_id = SND_MIXER_SCHN_FRONT_LEFT;

    snd_mixer_open(&mixer, 0);
    snd_mixer_attach(mixer, device_name);
    snd_mixer_selem_register(mixer, NULL, NULL);
    snd_mixer_load(mixer);

    snd_mixer_selem_id_alloca(&ident);
    snd_mixer_selem_id_set_index(ident, 0);
    snd_mixer_selem_id_set_name(ident, mixer_ctrl_name);
    elem = snd_mixer_find_selem(mixer, ident);
    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_get_playback_volume(elem, ch_id, &old_volume);
    printf("Min %ld max %ld current volume %ld\n", min, max, old_volume);
    volume = old_volume;

    return 0;
}

ALSAAudioSink::ALSAAudioSink(std::string device) : Task("", 0, 0, 0)
{
    snd_pcm_uframes_t     period_size_min;
    snd_pcm_uframes_t     period_size_max;
    snd_pcm_uframes_t     buffer_size_min;
    snd_pcm_uframes_t     buffer_size_max;

    static unsigned int   buffer_time = 0;	            /* ring buffer length in us */
    static unsigned int   period_time = 0;	            /* period time in us */
    static unsigned int   nperiods    = 4;                  /* number of periods */

    static snd_pcm_uframes_t  buffer_size;
    static snd_pcm_uframes_t  period_size;
    unsigned int                rate = 44100;

    /* Open the PCM device in playback mode */
    if (pcm = snd_pcm_open(&pcm_handle, device.c_str(),
                           SND_PCM_STREAM_PLAYBACK, 0) < 0)
    {
        printf("ERROR: Can't open \"%s\" PCM device. %s\n",
               device.c_str(), snd_strerror(pcm));
    }

    /* Allocate parameters object and fill it with default values*/
    snd_pcm_hw_params_alloca(&params);

    snd_pcm_hw_params_any(pcm_handle, params);

    /* Set parameters */
    if (pcm = snd_pcm_hw_params_set_access(pcm_handle, params,
                                           SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
        printf("ERROR: Can't set interleaved mode. %s\n", snd_strerror(pcm));

    if (pcm = snd_pcm_hw_params_set_format(pcm_handle, params,
                                           SND_PCM_FORMAT_S16_LE) < 0)
        printf("ERROR: Can't set format. %s\n", snd_strerror(pcm));

    if (pcm = snd_pcm_hw_params_set_channels(pcm_handle, params, 2) < 0)
        printf("ERROR: Can't set channels number. %s\n", snd_strerror(pcm));

    
    if (pcm = snd_pcm_hw_params_set_rate_near(pcm_handle, params, &rate, 0) < 0)
        printf("ERROR: Can't set rate. %s\n", snd_strerror(pcm));
    

    snd_pcm_hw_params_get_buffer_size_min(params, &buffer_size_min);
    snd_pcm_hw_params_get_buffer_size_max(params, &buffer_size_max);
    snd_pcm_hw_params_get_period_size_min(params, &period_size_min, NULL);
    snd_pcm_hw_params_get_period_size_max(params, &period_size_max, NULL);
    printf("Buffer size range from %lu to %lu\n",buffer_size_min, buffer_size_max);
    printf("Period size range from %lu to %lu\n",period_size_min, period_size_max);

    buffer_size = buffer_size_max;
    buffer_size = (buffer_size / nperiods) * nperiods;
    printf("Using max buffer size %lu\n", buffer_size);
    if ( (pcm = snd_pcm_hw_params_set_buffer_size_near(pcm_handle, params, &buffer_size)) < 0) 
    {
        printf("Unabled to set buffer size %lu for playback: %s \n", buffer_size, snd_strerror(pcm));
    }

    if ( (pcm = snd_pcm_hw_params_set_periods_near(pcm_handle, params, &nperiods, NULL)) < 0)
    {
        printf("Unabled to set nperiodes %u for playback: %s \n", nperiods, snd_strerror(pcm));
    }
    /*
    unsigned int periodTime = 800;
    int dir = -1;
    snd_pcm_hw_params_set_period_time_near(pcm_handle, params, &periodTime, &dir);
    */

    /* Write parameters */
    if (pcm = snd_pcm_hw_params(pcm_handle, params) < 0)
        printf("ERROR: Can't set harware parameters. %s\n", snd_strerror(pcm));

    /* Resume information */
    printf("PCM name: '%s'\n", snd_pcm_name(pcm_handle));

    printf("PCM state: %s\n", snd_pcm_state_name(snd_pcm_state(pcm_handle)));
    unsigned int tmp;
    snd_pcm_hw_params_get_channels(params, &tmp);
    printf("channels: %i ", tmp);
    if (tmp == 1)
        printf("(mono)\n");
    else if (tmp == 2)
        printf("(stereo)\n");

    snd_pcm_hw_params_get_period_time(params, &tmp, NULL);
    printf("period_time = %d\n", tmp);
    snd_pcm_hw_params_get_period_size(params, &frames, 0);

    this->buff_size = frames * 2 * 2 /* 2 -> sample size */;
    printf("required buff_size: %d\n", buff_size);
    
    /* prepare pcm */
    snd_pcm_prepare(pcm_handle);

    /** init mixer */
    //this->MixerInit(); // reserved

    this->startTask();
}

ALSAAudioSink::~ALSAAudioSink()
{
    snd_pcm_drain(pcm_handle);
    snd_pcm_close(pcm_handle);
}

void ALSAAudioSink::runTask()
{
    std::unique_ptr<std::vector<uint8_t>> dataPtr;
    while (true)
    {
        if (!this->ringbuffer.pop(dataPtr))
        {
            usleep(100);
            continue;
        }
        if (pcm = snd_pcm_writei(pcm_handle, dataPtr->data(), this->frames) == -EPIPE)
        {

            snd_pcm_prepare(pcm_handle);
        }
        else if (pcm < 0)
        {
            printf("ERROR. Can't write to PCM device. %s\n", snd_strerror(pcm));
        }
    }
}

void ALSAAudioSink::feedPCMFrames(const uint8_t *buffer, size_t bytes)
{

    buff.insert(buff.end(), buffer, buffer + bytes);
    while (buff.size() > this->buff_size)
    {
        auto ptr = std::make_unique<std::vector<uint8_t>>(this->buff.begin(), this->buff.begin() + this->buff_size);
        this->buff = std::vector<uint8_t>(this->buff.begin() + this->buff_size, this->buff.end());
        while (!this->ringbuffer.push(ptr))
        {
            usleep(100);
        };
    }
}
