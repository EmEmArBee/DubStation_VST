#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <atomic>
#include <memory>
#include "RackModule.h"

class DubRackProcessor : public juce::AudioProcessor
{
public:
    static constexpr int numModules = 7;
    static const juce::StringArray& moduleIds()
    {
        static const juce::StringArray ids { "filter", "phaser", "lofi", "echo", "dub", "spring", "plate" };
        return ids;
    }

    DubRackProcessor();
    ~DubRackProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "DubRack"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 10.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState apvts;

    // read by the editor (meters, tempo display)
    std::atomic<double> hostBpm { 120.0 };
    std::atomic<bool>   hostBpmValid { false };
    std::atomic<float>  inPeakL { 0.f }, inPeakR { 0.f }, outPeakL { 0.f }, outPeakR { 0.f }, outRms { 0.f };

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    std::array<std::unique_ptr<RackModule>, numModules> modules;
    std::array<int, numModules> order { 0, 1, 2, 3, 4, 5, 6 }; // rack order (reordering comes in stage 5)
    std::array<std::atomic<float>*, numModules> onParams {};
    std::atomic<float>* outputParam = nullptr;
    std::atomic<float>* amountParam = nullptr;

    juce::AudioBuffer<float> dryBuffer;
    float lastGain = 1.f, lastAmount = 1.f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DubRackProcessor)
};
