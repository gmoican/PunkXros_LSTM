// #include "DistortionProcessor.h"

// template <typename SampleType>
// void DistortionProcessor<SampleType>::setHPFreq (SampleType newCutOffFrequency)
// {
//     xrosFilter.state = *juce::dsp::IIR::Coefficients<SampleType>::makeHighPass(sampleRate, newCutOffFrequency);
// }

// template <typename SampleType>
// void DistortionProcessor<SampleType>::setCompensationLevel (SampleType newGainLevel)
// {
//     levelCompensation.setGainDecibels(newGainLevel);
// }

// template <typename SampleType>
// void DistortionProcessor<SampleType>::setMixLevel (SampleType newMixGain)
// {
//     mixLevel.setGainLinear(newMixGain);
// }

//==============================================================================
// template <typename SampleType>
// void DistortionProcessor<SampleType>::prepare (const juce::dsp::ProcessSpec& spec)
// {
//     jassert (spec.sampleRate > 0);
//     jassert (spec.numChannels > 0);
//
//     sampleRate = spec.sampleRate;
//
//     xrosFilter.prepare(spec);
//
//     preEmphasisEq.prepare(spec);
//     *preEmphasisEq.template get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 300.0, 1.0, 0.5);
//     *preEmphasisEq.template get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 700.0, 2.0, 0.3);
//     *preEmphasisEq.template get<2>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 1200.0, 0.7, 1.5);
//     *preEmphasisEq.template get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1200.0);
//
//     postEmphasisEq.prepare(spec);
//     *postEmphasisEq.template get<0>().state = *juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, 300.0, 1.0, 1.5);
//     *postEmphasisEq.template get<1>().state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 700.0, 1.0, 1.3);
//     *postEmphasisEq.template get<2>().state = *juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, 1200.0, 0.7, 0.7);
//     *postEmphasisEq.template get<3>().state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, 1200.0);
//
//     distortion.prepare(spec);
//
//     levelCompensation.prepare(spec);
//     levelCompensation.setRampDurationSeconds(0.1);
//     mixLevel.prepare(spec);
//     mixLevel.setRampDurationSeconds(0.1);
// }


//==============================================================================

// template <typename SampleType>
// void DistortionProcessor<SampleType>::processBlock(juce::AudioBuffer<SampleType>& buffer)
// {
//     juce::dsp::AudioBlock<SampleType> audioBlock(buffer);
//     xrosFilter.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
//
//     preEmphasisEq.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
//
//     // TODO: Add oversampling here
//     distortion.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
//
//     postEmphasisEq.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
//
//     levelCompensation.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
//     mixLevel.process(juce::dsp::ProcessContextReplacing<SampleType>(audioBlock));
//
//     processedBuffer.makeCopyOf(buffer);
// }
