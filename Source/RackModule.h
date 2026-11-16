#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

// Every effect in the rack implements this.
struct RackModule
{
    virtual ~RackModule() = default;
    virtual void prepare (double sampleRate, int maxBlock) = 0;
    virtual void reset() {}
    virtual void process (juce::AudioBuffer<float>& buffer, double bpm) = 0;
};

// Stage 1 placeholder: does nothing. Replaced by real DSP in later stages.
struct PassthroughModule : RackModule
{
    void prepare (double, int) override {}
    void process (juce::AudioBuffer<float>&, double) override {}
};
