#pragma once
#include "PluginProcessor.h"
#include <memory>
#include <vector>

// ---------- look: metal knobs, LED switches ----------
class DubLookAndFeel : public juce::LookAndFeel_V4
{
public:
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
};

// ---------- one rack module panel ----------
class ModulePanel : public juce::Component
{
public:
    ModulePanel (juce::AudioProcessorValueTreeState&, const juce::String& id,
                 const juce::String& title, const juce::String& subtitle, juce::Colour colour);
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    juce::String title, subtitle;
    juce::Colour colour;
    int seed;
    juce::ToggleButton onOff;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> attach;
};

// ---------- the whole rack, drawn at a fixed design size ----------
class RackView : public juce::Component, private juce::Timer
{
public:
    static constexpr int designW = 1115, designH = 812;
    explicit RackView (DubRackProcessor&);
    ~RackView() override { stopTimer(); }
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    DubRackProcessor& proc;
    DubLookAndFeel laf;
    std::vector<std::unique_ptr<ModulePanel>> panels;
    juce::Slider outputKnob, amountKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outAtt, amtAtt;
    float lvlL = 0, lvlR = 0, lvlPeak = 0, lvlAvg = 0, needle = 0;
};

// ---------- plugin window (scales with aspect ratio kept) ----------
class DubRackEditor : public juce::AudioProcessorEditor
{
public:
    explicit DubRackEditor (DubRackProcessor&);
    void resized() override;
private:
    RackView rack;
    juce::ComponentBoundsConstrainer constrainer;
};
