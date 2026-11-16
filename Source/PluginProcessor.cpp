#include "PluginProcessor.h"
#include "PluginEditor.h"

using APVTS = juce::AudioProcessorValueTreeState;

APVTS::ParameterLayout DubRackProcessor::createLayout()
{
    APVTS::ParameterLayout layout;

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "output", 1 }, "Output",
        juce::NormalisableRange<float> (-24.f, 12.f, 0.1f), 0.f));

    layout.add (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { "amount", 1 }, "Amount",
        juce::NormalisableRange<float> (0.f, 100.f, 1.f), 100.f));

    const char* names[] = { "Filter", "Phaser", "LoFi", "Echo", "Dub", "Spring", "Plate" };
    for (int i = 0; i < numModules; ++i)
        layout.add (std::make_unique<juce::AudioParameterBool> (
            juce::ParameterID { "on_" + moduleIds()[i], 1 }, juce::String (names[i]) + " On", true));

    return layout;
}

DubRackProcessor::DubRackProcessor()
    : AudioProcessor (BusesProperties()
                          .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "DubRack", createLayout())
{
    for (int i = 0; i < numModules; ++i)
    {
        modules[(size_t) i] = std::make_unique<PassthroughModule>();
        onParams[(size_t) i] = apvts.getRawParameterValue ("on_" + moduleIds()[i]);
    }
    outputParam = apvts.getRawParameterValue ("output");
    amountParam = apvts.getRawParameterValue ("amount");
}

bool DubRackProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainInputChannelSet() == juce::AudioChannelSet::stereo()
        && l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void DubRackProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dryBuffer.setSize (2, samplesPerBlock);
    for (auto& m : modules) { m->prepare (sampleRate, samplesPerBlock); m->reset(); }
    lastGain = juce::Decibels::decibelsToGain (outputParam->load());
    lastAmount = amountParam->load() * 0.01f;
}

void DubRackProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();
    const int chans = juce::jmin (2, buffer.getNumChannels());
    if (n == 0 || chans == 0) return;

    // host tempo
    double bpm = 120.0;
    bool valid = false;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
            if (auto b = pos->getBpm()) { bpm = *b; valid = true; }
    hostBpm = bpm;
    hostBpmValid = valid;

    inPeakL = buffer.getMagnitude (0, 0, n);
    inPeakR = chans > 1 ? buffer.getMagnitude (1, 0, n) : inPeakL.load();

    // keep dry copy for the Amount control
    if (dryBuffer.getNumSamples() < n) dryBuffer.setSize (2, n, false, false, true);
    for (int c = 0; c < chans; ++c) dryBuffer.copyFrom (c, 0, buffer, c, 0, n);

    // the rack, in order
    for (int idx : order)
        if (onParams[(size_t) idx]->load() > 0.5f)
            modules[(size_t) idx]->process (buffer, bpm);

    // dry/wet + output gain (ramped per block, no zipper noise)
    const float amt = amountParam->load() * 0.01f;
    const float gain = juce::Decibels::decibelsToGain (outputParam->load());
    for (int c = 0; c < chans; ++c)
    {
        auto* w = buffer.getWritePointer (c);
        auto* d = dryBuffer.getReadPointer (c);
        for (int i = 0; i < n; ++i)
        {
            const float t = (float) i / (float) n;
            const float a = lastAmount + (amt - lastAmount) * t;
            const float g = lastGain + (gain - lastGain) * t;
            w[i] = (d[i] + (w[i] - d[i]) * a) * g;
        }
    }
    lastAmount = amt;
    lastGain = gain;

    outPeakL = buffer.getMagnitude (0, 0, n);
    outPeakR = chans > 1 ? buffer.getMagnitude (1, 0, n) : outPeakL.load();
    outRms = buffer.getRMSLevel (0, 0, n);
}

juce::AudioProcessorEditor* DubRackProcessor::createEditor() { return new DubRackEditor (*this); }

void DubRackProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    if (auto xml = apvts.copyState().createXml())
        copyXmlToBinary (*xml, dest);
}

void DubRackProcessor::setStateInformation (const void* data, int size)
{
    if (auto xml = getXmlFromBinary (data, size))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new DubRackProcessor(); }
