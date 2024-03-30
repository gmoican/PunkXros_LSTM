#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

#if (MSVC)
#include "ipps.h"
#endif

#define DEFAULT_DRIVE 15.0f
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
    using WaveShaper = juce::dsp::WaveShaper<float>;
    using Comp = juce::dsp::Compressor<float>;
    using Oversampling = juce::dsp::Oversampling<float>;
    using Mixer = juce::dsp::DryWetMixer<float>;
    
    //=================== INPUT PROCESSING ========================================
    float driveGain = 1.f, distGainCompensation = 1.f, compGainCompensation = 1.f;
    
    //================= DISTORTION PROCESSING =====================================
    FilterBand xrosHPFilter;
    
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand> preEmphasisEq, postEmphasisEq;
    
    Oversampling distOV { 2, 2, Oversampling::filterHalfBandPolyphaseIIR, true, false};
    WaveShaper distortion;
        
    //================ COMPRESSION PROCESSING =====================================
    FilterBand xrosLPFilter;
    
    Comp compressor;
    Mixer mixer;
    
    //=================== OUTPUT PROCESSING =======================================
    FilterBand mids;
        
    float driveMixGain = 1.f, compMixGain = 1.f;
    float outputLevel = 1.f;
    bool on;
    
    // Drive functions
    static float saturator(float sample) { return tanhClipper( softClipper(sample) ); }
    
    static float softClipper(float sample) { return sample / (abs(sample) + 1.f); }
    
    static float tanhClipper(float sample) { return 2.f / juce::MathConstants<float>::pi * juce::dsp::FastMathApproximations::tanh(sample); }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PunkXrosProcessor)
};
