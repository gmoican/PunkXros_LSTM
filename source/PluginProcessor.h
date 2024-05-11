#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "RTNeuralLSTM.h"

#if (MSVC)
#include "ipps.h"
#endif

#define DEFAULT_DRIVE 0.5f
#define DEFAULT_LEVEL 0.0f
#define DEFAULT_MIX 50.0f
#define DEFAULT_MIDS 0.0f
#define DEFAULT_LOPASS 275.0f
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
    juce::AudioBuffer<float> compressedBuffer;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParams();
    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    using Comp = juce::dsp::Compressor<float>;
    using Mixer = juce::dsp::DryWetMixer<float>;
    
    //=================== INPUT PROCESSING ========================================
    float driveGain = DEFAULT_DRIVE, compGain, compensationGain;
    
    //================= DISTORTION PROCESSING =====================================
    FilterBand xrosHPFilter;
    
    RT_LSTM LSTM1;
    RT_LSTM LSTM2;
    
    //================ COMPRESSION PROCESSING =====================================
    FilterBand xrosLPFilter;
    
    Comp compressor;
    Mixer mixer;
    
    //=================== OUTPUT PROCESSING =======================================
    FilterBand mids;
        
    float driveMixGain = 1.f, compMixGain = 1.f;
    float outputLevel = 1.f;
    bool on;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkXrosProcessor)
};
