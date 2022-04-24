/**
 * show_type_format.c
 * 
 * user application to display some PCM and formats
*/
#include <alsa/asoundlib.h>

int main()
{
    int val;

    /* show alsa lib version */
    printf("ALSA Library version : %s \n", SND_LIB_VERSION_STR);

    /* show PCM stream types */
    printf("\nPCM Stream Types:\n");
    for(val = 0; val <= SND_PCM_STREAM_LAST; val+=1) {
        printf("    %s \n", snd_pcm_stream_name((snd_pcm_stream_t) val));
    }

    /* show PCM access type */
    printf("\nPCM Access Type: \n");
    for(val = 0; val <= SND_PCM_ACCESS_LAST; val+=1) {
        printf("    %s \n", snd_pcm_stream_name((snd_pcm_access_t) val));
    }

    /* show PCM format */
    printf("\nPCM formats: \n");
    for(val = 0; val <= SND_PCM_FORMAT_LAST; val+=1) {
        if (snd_pcm_format_name((snd_pcm_format_t) val != NULL)) {
            printf("    %s (%s)\n", snd_pcm_format_name((snd_pcm_format_t) val), \
                                    snd_pcm_format_description((snd_pcm_format_t)val));
        }
        
    }

    /* show PCM subformat */
    printf("\nPCM subformats: \n");
    for(val = 0; val <= SND_PCM_SUBFORMAT_LAST; val+=1) {
        if (snd_pcm_subformat_name((snd_pcm_subformat_t) val != NULL)) {
            printf("    %s (%s)\n", snd_pcm_subformat_name((snd_pcm_subformat_t) val), \
                                    snd_pcm_subformat_description((snd_pcm_subformat_t)val));
        }
        
    }

    /* show PCM states */
    printf("\nPCM states: \n");
    for(val = 0; val <= SND_PCM_STATE_LAST; val+=1) {
        if (snd_pcm_state_name((snd_pcm_state_t) val != NULL)) {
            printf("    %s\n", snd_pcm_state_name((snd_pcm_state_t) val));
        }
        
    }

    return 0;
}