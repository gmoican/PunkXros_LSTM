#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "DistortionProcessor.h"
// #include "CompressionProcessor.h"

#if (MSVC)
#include "ipps.h"
#endif

#define DEFAULT_DRIVE 5.0f
#define DEFAULT_LEVEL 0.0f
#define DEFAULT_MIX 50.0f
#define DEFAULT_MIDS 0.0f
#define DEFAULT_LOPASS 250.0f
#define DEFAULT_HIPASS 550.0f

//==============================================================================
/**
*/

class PunkXrosProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    PunkXrosProcessor();
    ~PunkXrosProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    //=============== MY STUFF =====================================================
    juce::AudioProcessorValueTreeState state;
    
    // Updaters
    void updateOnOff();
    void updateLevel();
    void updateDrive();
    void updateMix();
    void updateEq();
    void updateState();
    
    void process(float* samples, int numSamples);

private:
    juce::AudioBuffer<float> distortedBuffer;
    // juce::AudioBuffer<float> compressedBuffer;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using WaveShaper = juce::dsp::WaveShaper<float>;
    using Gain = juce::dsp::Gain<float>;
    
    FilterBand mids;
    
    DistortionProcessor<float> drive;
    
    Gain driveLevel, outputLevel;
    bool on;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkXrosProcessor)
};
