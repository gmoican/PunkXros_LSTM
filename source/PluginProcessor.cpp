#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PunkXrosProcessor::PunkXrosProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ), state(*this, nullptr, "parameters", createParams())
#endif
{
}

PunkXrosProcessor::~PunkXrosProcessor()
{
}

//==============================================================================
const juce::String PunkXrosProcessor::getName() const
{
    return JucePlugin_Name;
}

bool PunkXrosProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool PunkXrosProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool PunkXrosProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double PunkXrosProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int PunkXrosProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int PunkXrosProcessor::getCurrentProgram()
{
    return 0;
}

void PunkXrosProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused(index);
}

const juce::String PunkXrosProcessor::getProgramName (int index)
{
    juce::ignoreUnused(index);
    return {};
}

void PunkXrosProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

// =========== PARAMETER LAYOUT ====================
juce::AudioProcessorValueTreeState::ParameterLayout PunkXrosProcessor::createParams()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
        
    params.push_back(std::make_unique<juce::AudioParameterBool>(juce::ParameterID("ONOFF", 0), "On/Off", true));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("DRIVE", 0), "Drive", juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f), DEFAULT_DRIVE, ""));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LEVEL", 0), "Output level", juce::NormalisableRange<float>(-18.0f, 18.0f, 0.1f), DEFAULT_LEVEL, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("MIX", 0), "Mix", juce::NormalisableRange<float>(0.0f, 100.0f, 0.1f), DEFAULT_MIX, "%"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("MIDS", 0), "Mids gain", juce::NormalisableRange<float>(-10.0f, 10.0f, 0.1f), DEFAULT_MIDS, "dB"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("LOPASS", 0), "LowPass frequency", juce::NormalisableRange<float>(50.0f, 500.0f, 0.1f), DEFAULT_LOPASS, "Hz"));
    params.push_back(std::make_unique<juce::AudioParameterFloat>(juce::ParameterID("HIPASS", 0), "HighPass frequency", juce::NormalisableRange<float>(100.0f, 1000.0f, 0.1f), DEFAULT_HIPASS, "Hz"));
        
    return { params.begin(), params.end() };
}

// ============ VALUE UPDATERS =====================
void PunkXrosProcessor::updateOnOff()
{
    auto ONOFF = state.getRawParameterValue("ONOFF");
    on = ONOFF->load();
}

void PunkXrosProcessor::updateLevel()
{
    auto OUT = state.getRawParameterValue("LEVEL")->load();
    outputLevel = juce::Decibels::decibelsToGain(OUT);
}

void PunkXrosProcessor::updateDrive()
{
    driveGain = state.getRawParameterValue("DRIVE")->load();
    compGain = juce::Decibels::decibelsToGain( juce::jmap(driveGain, 0.f, 30.f) );
    compensationGain = juce::Decibels::decibelsToGain( juce::jmap(driveGain, 0.f, -12.f) );
}

void PunkXrosProcessor::updateMix()
{
    auto MIX = state.getRawParameterValue("MIX")->load();
    
    // Drive processor - Adjust mix level
    if (MIX < 50.f)
        driveMixGain = MIX / 50.f;
    else
        driveMixGain = 1.f;
    
    // Compression processor - Adjust mix level
    if (MIX > 50.0)
        compMixGain = (100.f - MIX) / 50.f;
    else
        compMixGain = 1.f;
}

void PunkXrosProcessor::updateEq()
{
    auto LOPASS = state.getRawParameterValue("LOPASS");
    auto MIDS = state.getRawParameterValue("MIDS");
    auto HIPASS = state.getRawParameterValue("HIPASS");
    
    double sampleRate = getSampleRate();
    
    // Drive hi pass
    *xrosHPFilter.state = *juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, HIPASS->load());
    
    // Compression low pass
    *xrosLPFilter.state = *juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, LOPASS->load());
    
    // Mids boost/cut applied after processing
    auto midsGain = juce::Decibels::decibelsToGain(MIDS->load());
    
    *mids.state = *juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, 500.f, 0.8f, midsGain);
}

void PunkXrosProcessor::updateState()
{
    updateOnOff();
    updateDrive();
    updateMix();
    updateEq();
    updateLevel();
}

//==============================================================================
void PunkXrosProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    juce::dsp::ProcessSpec spec;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    spec.sampleRate = sampleRate;
            
    // DISTORTION PROCESSING
    xrosHPFilter.prepare(spec);
    
    // Read json model
    juce::MemoryInputStream jsonInputStream(BinaryData::b3k_model_json, BinaryData::b3k_model_jsonSize, false);
    nlohmann::json weights_json = nlohmann::json::parse(jsonInputStream.readEntireStreamAsString().toStdString());
    LSTM1.reset();
    LSTM1.load_json(weights_json);
    LSTM2.reset();
    LSTM2.load_json(weights_json);
        
    // COMPRESSION PROCESSING
    xrosLPFilter.prepare(spec);
    
    compressor.prepare(spec);
    compressor.setRatio(4.f);
    compressor.setAttack(30.0f);
    compressor.setRelease(40.f);
    compressor.setThreshold(-18.f);
    
    mixer.prepare(spec);
    mixer.setWetMixProportion(0.9f);
    
    // OUTPUT PROCESSING
    mids.prepare(spec);
    mids.reset();
}

void PunkXrosProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool PunkXrosProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void PunkXrosProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused(midiMessages);
    
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    updateState();
    if(on)
    {
        distortedBuffer.makeCopyOf(buffer);
        compressedBuffer.makeCopyOf(buffer);
        
        juce::dsp::AudioBlock<float> distBlock = juce::dsp::AudioBlock<float>(distortedBuffer);
        juce::dsp::AudioBlock<float> compBlock = juce::dsp::AudioBlock<float>(compressedBuffer);
    
        
        //================= DISTORTION PROCESSING =====================================
        xrosHPFilter.process(juce::dsp::ProcessContextReplacing<float>(distBlock));
        
        for (int ch = 0; ch < distortedBuffer.getNumChannels(); ++ch) {
            if (ch == 0)
                LSTM1.process(distortedBuffer.getReadPointer(ch), distortedBuffer.getWritePointer(ch), driveGain, distortedBuffer.getNumSamples());
            else
                LSTM2.process(distortedBuffer.getReadPointer(ch), distortedBuffer.getWritePointer(ch), driveGain, distortedBuffer.getNumSamples());
        }
        
        //================ COMPRESSION PROCESSING =====================================
        xrosLPFilter.process(juce::dsp::ProcessContextReplacing<float>(compBlock));
        
        mixer.pushDrySamples(compBlock);
        
        compressedBuffer.applyGain(compGain);
        compressor.process(juce::dsp::ProcessContextReplacing<float>(compBlock));
        compressedBuffer.applyGain(compensationGain);
        
        mixer.mixWetSamples(compBlock);
        
        //================== SUM PARALLEL BUFFERS =====================================
        distortedBuffer.applyGain(driveMixGain);
        compressedBuffer.applyGain(compMixGain);
        
        buffer.makeCopyOf(distortedBuffer);
        for (auto i = 0; i < buffer.getNumChannels(); ++i)
            buffer.addFrom(i, 0, compressedBuffer, i, 0, buffer.getNumSamples());
        
        //=================== OUTPUT PROCESSING =======================================
        juce::dsp::AudioBlock<float> audioBlock = juce::dsp::AudioBlock<float>(buffer);
        
        mids.process(juce::dsp::ProcessContextReplacing<float>(audioBlock));
        
        buffer.applyGain(outputLevel);
    }
}

//==============================================================================
bool PunkXrosProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* PunkXrosProcessor::createEditor()
{
    return new PunkXrosEditor (*this);
}

//==============================================================================
void PunkXrosProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    
    juce::ignoreUnused(destData);
}

void PunkXrosProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PunkXrosProcessor();
}
