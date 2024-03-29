#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

template <typename SampleType>
class DistortionProcessor
{
public:
    DistortionProcessor() {}
    ~DistortionProcessor() {}
    
    //==============================================================================
    /** Sets the cutoff frequency of the input HPF.*/
    void setHPFreq (SampleType newCutOffFrequency)
    {
        xrosFilter.state = *juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(sampleRate, newCutOffFrequency);
    }
    
    /** Sets the gain to compensate the level at the output.*/
    void setCompensationLevel (SampleType newGainLevel){
        levelCompensation.setGainDecibels(newGainLevel);
    }
    
    /** Sets the gain to blend the distortion signal with the compressed signal.*/
    void setMixLevel (SampleType newMixGain)
    {
        mixLevel.setGainLinear(newMixGain);
    }
    
    //==============================================================================
    void prepare (const juce::dsp::ProcessSpec& spec)
    {
        sampleRate = spec.sampleRate;
                
        xrosFilter.prepare(spec);
        
        preEmphasisEq.prepare(spec);
        *preEmphasisEq.template get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 300.0, 1.0, 0.5);
        *preEmphasisEq.template get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 700.0, 2.0, 0.3);
        *preEmphasisEq.template get<2>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 1200.0, 0.7, 1.5);
        *preEmphasisEq.template get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1200.0);
        
        postEmphasisEq.prepare(spec);
        *postEmphasisEq.template get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 300.0, 1.0, 1.5);
        *postEmphasisEq.template get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 700.0, 1.0, 1.3);
        *postEmphasisEq.template get<2>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 1200.0, 0.7, 0.7);
        *postEmphasisEq.template get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1200.0);
        
        distortion.prepare(spec);
        
        levelCompensation.prepare(spec);
        levelCompensation.setRampDurationSeconds(0.1);
        mixLevel.prepare(spec);
        mixLevel.setRampDurationSeconds(0.1);
    }
    
    //==============================================================================
    /** Processes the input and output samples supplied in the processing context. */
    void processBlock (juce::AudioBuffer<SampleType>& buffer)
    {
        juce::dsp::AudioBlock<SampleType> audioBlock(buffer);
        xrosFilter.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
        
        preEmphasisEq.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
        
        // TODO: Add oversampling here
        distortion.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
        
        postEmphasisEq.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
        
        levelCompensation.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
        mixLevel.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
    }
        
private:
    double sampleRate = 44100.0;
    SampleType cutoffFrequency = 550.0;
    
    using FilterBand = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<SampleType>, juce::dsp::IIR::Coefficients<SampleType>>;
    using WaveShaper = juce::dsp::WaveShaper<SampleType>;
    using Gain = juce::dsp::Gain<SampleType>;
    
    FilterBand xrosFilter;
    juce::dsp::ProcessorChain<FilterBand, FilterBand, FilterBand, FilterBand> preEmphasisEq, postEmphasisEq;
    juce::dsp::ProcessorChain<WaveShaper, Gain> distortion;
    
    Gain levelCompensation, mixLevel;
    
    // Drive functions
    static SampleType saturator(SampleType sample) { return tanhClipper( softClipper(sample) ); }
    
    static SampleType softClipper(SampleType sample) { return sample / (abs(sample) + 1.0); }
    
    static SampleType tanhClipper(SampleType sample) { return 2.0 / juce::MathConstants<SampleType>::pi * juce::dsp::FastMathApproximations::tanh(sample); }
};
