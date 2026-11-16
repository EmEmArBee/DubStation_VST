#include "PluginEditor.h"

using namespace juce;
static const Colour kOrange (0xffff8a2a), kBlack (0xff0a0806), kPale (0xffe8e0c8);

// ================= LookAndFeel =================
void DubLookAndFeel::drawRotarySlider (Graphics& g, int x, int y, int w, int h, float pos,
                                       float startAngle, float endAngle, Slider&)
{
    auto b = Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
    const float cx = b.getCentreX(), cy = b.getCentreY();
    const float r = jmin (b.getWidth(), b.getHeight()) * 0.5f - 6.f;

    for (int i = 0; i <= 10; ++i)   // tick marks
    {
        const float a = startAngle + (endAngle - startAngle) * (float) i / 10.f;
        const float rr = r + 4.f;
        g.setColour (Colour (0xffcfc8b0));
        g.fillEllipse (cx + rr * std::sin (a) - 1.f, cy - rr * std::cos (a) - 1.f, 2.f, 2.f);
    }
    g.setColour (Colours::black.withAlpha (0.5f));
    g.fillEllipse (cx - r, cy - r + 2.f, r * 2.f, r * 2.f);               // shadow
    g.setGradientFill (ColourGradient (Colour (0xfff4f4f4), cx - r * 0.5f, cy - r * 0.6f,
                                       Colour (0xff6a6a6a), cx + r * 0.6f, cy + r * 0.8f, false));
    g.fillEllipse (cx - r, cy - r, r * 2.f, r * 2.f);                      // body
    g.setColour (Colour (0xff1c1c1c));
    g.fillEllipse (cx - r * 0.72f, cy - r * 0.72f, r * 1.44f, r * 1.44f);  // cap
    g.setGradientFill (ColourGradient (Colour (0xff4a4a4a), cx, cy - r * 0.7f, Colour (0xff111111), cx, cy + r * 0.7f, false));
    g.fillEllipse (cx - r * 0.66f, cy - r * 0.66f, r * 1.32f, r * 1.32f);

    const float a = startAngle + pos * (endAngle - startAngle);
    g.setColour (kPale);
    g.drawLine (cx + r * 0.2f * std::sin (a), cy - r * 0.2f * std::cos (a),
                cx + r * 0.62f * std::sin (a), cy - r * 0.62f * std::cos (a), 2.5f);
}

void DubLookAndFeel::drawToggleButton (Graphics& g, ToggleButton& b, bool, bool)
{
    auto r = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();
    g.setColour (kPale);
    g.setFont (Font (FontOptions (11.f, Font::bold)));
    g.drawText ("OFF", r.removeFromLeft (24.f), Justification::centredRight);
    auto sw = r.removeFromLeft (30.f).reduced (3.f, 6.f);
    g.setColour (Colour (0xff111111));
    g.fillRoundedRectangle (sw, sw.getHeight() * 0.5f);
    g.setColour (on ? kOrange : Colour (0xff8a8a8a));
    const float d = sw.getHeight() + 2.f;
    g.fillEllipse (on ? sw.getRight() - d + 1.f : sw.getX() - 1.f, sw.getY() - 1.f, d, d);
    g.setColour (kPale);
    g.drawText ("ON", r.reduced (4.f, 0.f), Justification::centredLeft);
}

// ================= ModulePanel =================
ModulePanel::ModulePanel (AudioProcessorValueTreeState& apvts, const String& id,
                          const String& t, const String& s, Colour c)
    : title (t), subtitle (s), colour (c), seed (t.hashCode())
{
    addAndMakeVisible (onOff);
    attach = std::make_unique<AudioProcessorValueTreeState::ButtonAttachment> (apvts, "on_" + id, onOff);
}

void ModulePanel::resized() { onOff.setBounds (8, 6, 84, 26); }

void ModulePanel::paint (Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (1.f);
    g.setGradientFill (ColourGradient (colour.brighter (0.15f), 0, 0, colour.darker (0.35f), 0, r.getHeight(), false));
    g.fillRoundedRectangle (r, 6.f);

    Random rng (seed);                       // scratches
    g.setColour (Colours::white.withAlpha (0.08f));
    for (int i = 0; i < 40; ++i)
    {
        const float x = rng.nextFloat() * r.getWidth(), y = rng.nextFloat() * r.getHeight();
        g.drawLine (x, y, x + rng.nextFloat() * 40.f - 20.f, y + rng.nextFloat() * 40.f - 20.f, 0.6f);
    }
    g.setColour (Colour (0xffb9b9b9));
    g.drawRoundedRectangle (r, 6.f, 2.f);

    g.setColour (kPale);
    g.setFont (Font (FontOptions (20.f, Font::bold)));
    g.drawText (title.toUpperCase(), 96, 6, 160, 26, Justification::centredLeft);
    g.setFont (Font (FontOptions (10.f)));
    g.setColour (kPale.withAlpha (0.8f));
    g.drawText (subtitle.toUpperCase(), 96, 28, getWidth() - 100, 14, Justification::centredLeft);

    g.setColour (Colours::black.withAlpha (0.35f));
    g.fillRoundedRectangle (10.f, 50.f, r.getWidth() - 18.f, r.getHeight() - 62.f, 4.f);
    g.setColour (kPale.withAlpha (0.45f));
    g.setFont (Font (FontOptions (12.f)));
    g.drawText ("Coming in a later stage", 10, 50, getWidth() - 18, getHeight() - 62, Justification::centred);
}

// ================= RackView =================
RackView::RackView (DubRackProcessor& p) : proc (p)
{
    setLookAndFeel (&laf);
    setSize (designW, designH);

    struct Def { const char* id; const char* title; const char* sub; uint32 col; Rectangle<int> bounds; };
    const Def defs[] = {
        { "filter", "Filter", "Big knob high-pass",        0xff4a4f58, {  46, 147, 205, 361 } },
        { "phaser", "Phaser", "4 to 12 stages",            0xff3d4f6e, { 263, 147, 184, 361 } },
        { "lofi",   "LoFi",   "Retro colour: tape, vinyl, sampler", 0xff8a8157, { 459, 147, 326, 361 } },
        { "echo",   "Echo",   "Echo styles, modes, groove", 0xff4f7a4a, { 798, 147, 282, 361 } },
        { "dub",    "Dub",    "Tape echo + spring",        0xffc8642e, {  46, 520, 508, 275 } },
        { "spring", "Spring", "Dispersive spring reverb",  0xff5f7a4e, { 566, 520, 252, 275 } },
        { "plate",  "Plate",  "Plate reverb",              0xffb8a578, { 830, 520, 250, 275 } },
    };
    for (auto& d : defs)
    {
        panels.push_back (std::make_unique<ModulePanel> (p.apvts, d.id, d.title, d.sub, Colour (d.col)));
        panels.back()->setBounds (d.bounds);
        addAndMakeVisible (*panels.back());
    }

    for (auto* s : { &outputKnob, &amountKnob })
    {
        s->setSliderStyle (Slider::RotaryHorizontalVerticalDrag);
        s->setTextBoxStyle (Slider::TextBoxBelow, false, 70, 18);
        s->setRotaryParameters (MathConstants<float>::pi * 1.2f, MathConstants<float>::pi * 2.8f, true);
        s->setColour (Slider::textBoxTextColourId, kOrange);
        s->setColour (Slider::textBoxBackgroundColourId, kBlack);
        s->setColour (Slider::textBoxOutlineColourId, Colours::transparentBlack);
        addAndMakeVisible (*s);
    }
    outputKnob.setTextValueSuffix (" dB"); outputKnob.setNumDecimalPlacesToDisplay (1);
    amountKnob.setTextValueSuffix (" %");  amountKnob.setNumDecimalPlacesToDisplay (0);
    outAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "output", outputKnob);
    amtAtt = std::make_unique<AudioProcessorValueTreeState::SliderAttachment> (p.apvts, "amount", amountKnob);

    startTimerHz (30);
}

void RackView::resized()
{
    outputKnob.setBounds (790, 38, 76, 100);
    amountKnob.setBounds (870, 38, 76, 100);
}

void RackView::timerCallback()
{
    auto follow = [] (float& v, float target) { v = target > v ? target : v * 0.88f; };
    follow (lvlL, proc.inPeakL);  follow (lvlR, proc.inPeakR);
    follow (lvlPeak, jmax (proc.outPeakL.load(), proc.outPeakR.load()));
    lvlAvg += (proc.outRms - lvlAvg) * 0.2f;
    const float db = Decibels::gainToDecibels (lvlAvg, -40.f);
    const float tgt = jmap (jlimit (-20.f, 3.f, db), -20.f, 3.f, -1.f, 1.f);
    needle += (tgt - needle) * 0.3f;
    repaint (0, 0, designW, 140);
}

static void drawTube (Graphics& g, Rectangle<float> r, float level, const String& label)
{
    const float lv = jlimit (0.f, 1.f, level);
    g.setColour (Colour (0xff1a1410));
    g.fillRoundedRectangle (r, r.getWidth() * 0.45f);
    g.setGradientFill (ColourGradient (kOrange.withAlpha (0.25f + 0.75f * lv), r.getCentreX(), r.getCentreY(),
                                       Colour (0xff1a1410), r.getX(), r.getY(), true));
    g.fillRoundedRectangle (r.reduced (3.f), r.getWidth() * 0.4f);
    g.setColour (kPale.withAlpha (0.5f));
    g.drawRoundedRectangle (r, r.getWidth() * 0.45f, 1.5f);
    g.setColour (kPale);
    g.setFont (Font (FontOptions (9.f, Font::bold)));
    g.drawText (label, r.getX() - 10.f, r.getBottom() + 4.f, r.getWidth() + 20.f, 12.f, Justification::centred);
}

void RackView::paint (Graphics& g)
{
    g.fillAll (Colour (0xff17181a));
    // rack ears
    g.setColour (Colour (0xff4a4d4a));
    g.fillRect (0, 0, 40, designH);
    g.fillRect (designW - 40, 0, 40, designH);

    // header plate
    auto hdr = Rectangle<float> (40.f, 10.f, designW - 80.f, 125.f);
    g.setGradientFill (ColourGradient (Colour (0xff566a4e), 0, 10, Colour (0xff3f5139), 0, 135, false));
    g.fillRoundedRectangle (hdr, 6.f);

    g.setColour (kPale);
    g.setFont (Font (FontOptions (44.f, Font::bold)));
    g.drawText ("DUBRACK", 56, 22, 260, 50, Justification::centredLeft);
    g.setColour (kOrange);
    g.setFont (Font (FontOptions (11.f, Font::bold)));
    g.drawText ("ASFALTO SONORO", 58, 72, 200, 14, Justification::centredLeft);
    g.setColour (kPale.withAlpha (0.8f));
    g.setFont (Font (FontOptions (9.f, Font::bold)));
    g.drawText ("DUB EFFECT RACK  -  7 MODULES", 58, 88, 250, 12, Justification::centredLeft);

    // hazard stripe
    {
        Graphics::ScopedSaveState ss (g);
        g.reduceClipRegion (58, 104, 150, 12);
        g.fillAll (Colour (0xffe6b800));
        g.setColour (Colours::black);
        for (int x = 40; x < 230; x += 16)
        {
            Path p; p.addQuadrilateral ((float) x, 116.f, (float) x + 8.f, 116.f, (float) x + 20.f, 104.f, (float) x + 12.f, 104.f);
            g.fillPath (p);
        }
    }

    // tube display
    g.setColour (Colour (0xff141210));
    g.fillRoundedRectangle (240.f, 20.f, 225.f, 105.f, 6.f);
    drawTube (g, { 254.f, 32.f, 36.f, 56.f }, lvlL * 1.5f, "IN L");
    drawTube (g, { 304.f, 32.f, 36.f, 56.f }, lvlPeak * 1.5f, "PEAK");
    drawTube (g, { 354.f, 32.f, 36.f, 56.f }, lvlAvg * 3.f, "AVG");
    drawTube (g, { 404.f, 32.f, 36.f, 56.f }, lvlR * 1.5f, "IN R");

    // tempo readout (proves host tempo sync works)
    g.setColour (kBlack);
    g.fillRoundedRectangle (500.f, 100.f, 190.f, 22.f, 3.f);
    g.setColour (kOrange);
    g.setFont (Font (FontOptions (13.f)));
    g.drawText (proc.hostBpmValid ? "HOST TEMPO  " + String (proc.hostBpm.load(), 1) + " BPM"
                                  : "HOST TEMPO  --- (no host info)",
                500, 100, 190, 22, Justification::centred);

    g.setColour (kPale);
    g.setFont (Font (FontOptions (11.f, Font::bold)));
    g.drawText ("OUTPUT", 790, 22, 76, 14, Justification::centred);
    g.drawText ("AMOUNT", 870, 22, 76, 14, Justification::centred);

    // VU meter
    auto vu = Rectangle<float> (960.f, 24.f, 110.f, 90.f);
    g.setColour (Colour (0xff2a2a2a));
    g.fillRoundedRectangle (vu.expanded (4.f), 4.f);
    g.setColour (Colour (0xffe9e1c4));
    g.fillRoundedRectangle (vu, 3.f);
    const float px = vu.getCentreX(), py = vu.getBottom() - 8.f, len = 70.f;
    const float ang = needle * 0.85f;   // radians from vertical
    g.setColour (Colours::black);
    g.drawLine (px, py, px + len * std::sin (ang), py - len * std::cos (ang), 1.6f);
    g.setFont (Font (FontOptions (11.f, Font::bold)));
    g.drawText ("VU", vu.withTrimmedTop (28.f).withHeight (14.f), Justification::centred);
}

// ================= Editor =================
DubRackEditor::DubRackEditor (DubRackProcessor& p) : AudioProcessorEditor (&p), rack (p)
{
    addAndMakeVisible (rack);
    constrainer.setFixedAspectRatio ((double) RackView::designW / RackView::designH);
    constrainer.setSizeLimits (RackView::designW / 2, RackView::designH / 2,
                               RackView::designW * 2, RackView::designH * 2);
    setConstrainer (&constrainer);
    setResizable (true, true);
    setSize (RackView::designW, RackView::designH);
}

void DubRackEditor::resized()
{
    const float s = (float) getWidth() / (float) RackView::designW;
    rack.setTransform (AffineTransform::scale (s));
    rack.setBounds (0, 0, RackView::designW, RackView::designH);
}
