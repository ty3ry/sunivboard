#pragma once

#include <vector>
#include "portaudio.h"
#include <cstdint>
#include <iostream>
#include "AudioSink.h"

class PortAudioSink : public AudioSink
{
public:
    PortAudioSink();
    ~PortAudioSink() override;
    void feedPCMFrames(const uint8_t *buffer, size_t bytes) override;
	bool setParams(uint32_t sampleRate, uint8_t channelCount, uint8_t bitDepth) override;
    
private:
    PaStream *stream = nullptr;
};
