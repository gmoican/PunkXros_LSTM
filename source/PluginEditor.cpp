#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkXrosEditor::PunkXrosEditor (PunkXrosProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    juce::ignoreUnused(audioProcessor);
    
    // ================= PARAMETERS ====================
    setSliderComponent(driveKnob, driveKnobAttachment, "DRIVE");
    setSliderComponent(levelKnob, levelKnobAttachment, "LEVEL");
    
    setSliderComponent(mixKnob, mixKnobAttachment, "MIX");
    setSliderComponent(midsKnob, midsKnobAttachment, "MIDS");
    
    setSliderComponent(loPassKnob, loPassKnobAttachment, "LOPASS");
    setSliderComponent(hiPassKnob, hiPassKnobAttachment, "HIPASS");

    setToggleComponent(onToggle, onToggleAttachment, "ONOFF");

    // ================= ASSETS =======================
    background = juce::ImageCache::getFromMemory(BinaryData::background_png, BinaryData::background_pngSize);
    lightOff = juce::ImageCache::getFromMemory(BinaryData::lightOff_png, BinaryData::lightOff_pngSize);
    knobImage = juce::ImageCache::getFromMemory(BinaryData::knob_png, BinaryData::knob_pngSize);
        
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (180, 320);
}

PunkXrosEditor::~PunkXrosEditor()
{
}

//==============================================================================
void PunkXrosEditor::paint (juce::Graphics& g)
{
    g.drawImageWithin(background, 0, 0, getWidth(), getHeight(), juce::RectanglePlacement::stretchToFit);
        
    // =========== On/Off state ====================
    if (!onToggle.getToggleState()) {
        juce::AffineTransform t;
        t = t.scaled(0.485f);
        t = t.translated(75.5, 163.5);
        g.drawImageTransformed(lightOff, t);
    }
    
    // ========== Parameter knobs angle in radians ==================
    auto driveRadians = juce::jmap(driveKnob.getValue(), 0.0, 30.0, -150.0, 150.0) * DEG2RADS;
    auto levelRadians = juce::jmap(levelKnob.getValue(), -18.0, 18.0, -150.0, 150.0) * DEG2RADS;
    auto mixRadians = juce::jmap(mixKnob.getValue(), 0.0, 100.0, -150.0, 150.0) * DEG2RADS;
    auto midsRadians = juce::jmap(midsKnob.getValue(), -10.0, 10.0, -150.0, 150.0) * DEG2RADS;
    auto loPassRadians = juce::jmap(loPassKnob.getValue(), 50.0, 500.0, -150.0, 150.0) * DEG2RADS;
    auto hiPassRadians = juce::jmap(hiPassKnob.getValue(), 100.0, 1000.0, -150.0, 150.0) * DEG2RADS;
    
    // ========== Draw parameter knobs ==================
    g.drawImageTransformed(knobImage, knobRotation(driveRadians, 23.5, 23.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(levelRadians, 112.5, 23.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(mixRadians, 23.5, 91.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(midsRadians, 112.5, 91.0, 0.48));
    g.drawImageTransformed(knobImage, knobRotation(loPassRadians, 23.5, 158.0, 0.24));
    g.drawImageTransformed(knobImage, knobRotation(hiPassRadians, 132.5, 158.0, 0.24));
}

void PunkXrosEditor::resized()
{
    // Upper row
    driveKnob.setBounds(24, 23, 46, 46);
    levelKnob.setBounds(113, 23, 46, 46);
    
    // Middle row
    mixKnob.setBounds(24, 91, 46, 46);
    midsKnob.setBounds(113, 91, 46, 46);
    
    // Bottom row
    loPassKnob.setBounds(24, 158, 24, 24);
    hiPassKnob.setBounds(133, 158, 24, 24);
    
    // OnOff
    onToggle.setBounds(65, 240, 50, 50);
}

void PunkXrosEditor::setSliderComponent(juce::Slider &slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> &sliderAttachment, juce::String paramName){
    sliderAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(audioProcessor.state, paramName, slider);
    slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::NoTextBox, true, 0, 0);
    addAndMakeVisible(slider);
    slider.setAlpha(0);
}

void PunkXrosEditor::setToggleComponent(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& buttonAttachment, juce::String paramName){
    buttonAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment>(audioProcessor.state, paramName, button);
    addAndMakeVisible(button);
    button.setAlpha(0);
}

juce::AffineTransform PunkXrosEditor::knobRotation(float radians, float posX, float posY, float scaleFactor){
    juce::AffineTransform t;
    t = t.rotated(radians, 46.0f, 46.0f);
    t = t.scaled(scaleFactor);
    t = t.translated(posX, posY);
    return t;
}
