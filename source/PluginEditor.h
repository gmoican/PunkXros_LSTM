#pragma once

#include "PluginProcessor.h"
#include "BinaryData.h"

#define DEG2RADS 0.0174533f

//==============================================================================
/**
*/
class PunkXrosEditor  : public juce::AudioProcessorEditor
{
public:
    PunkXrosEditor (PunkXrosProcessor&);
    ~PunkXrosEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;
    
    //=================== PARAMETER MANIPULATION ===================================
    void setSliderComponent(juce::Slider& slider, std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment>& sliderAttachment, juce::String paramName);
    void setToggleComponent(juce::ToggleButton& button, std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment>& buttonAttachment, juce::String paramName);
    juce::AffineTransform knobRotation(float radians, float posX, float posY, float scaleFactor);
    
private:
    // Parameters
    juce::Slider driveKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> driveKnobAttachment;
    
    juce::Slider levelKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> levelKnobAttachment;
    
    juce::Slider mixKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> mixKnobAttachment;
    
    juce::Slider midsKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> midsKnobAttachment;
    
    juce::Slider loPassKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> loPassKnobAttachment;
    
    juce::Slider hiPassKnob;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> hiPassKnobAttachment;
    
    juce::ToggleButton onToggle;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> onToggleAttachment;
    
    // Assets
    juce::Image background;
    juce::Image lightOff;
    juce::Image knobImage;
        
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    PunkXrosProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkXrosEditor)
};
