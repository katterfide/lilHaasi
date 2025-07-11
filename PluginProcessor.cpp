#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                     #if ! JucePlugin_IsSynth
                      .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                     #endif
                      .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                    #endif
                     ),
      parameters(*this, nullptr, juce::Identifier("PARAMETERS"), createParameterLayout())
{
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor() {}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram (int index)
{
    juce::ignoreUnused (index);
}

const juce::String AudioPluginAudioProcessor::getProgramName (int index)
{
    juce::ignoreUnused (index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..


    // 500ms max delay * sampleRate = max samples needed
    int maxDelaySamples = static_cast<int>(0.5 * sampleRate);

    delayBuffer.setSize(2, maxDelaySamples); // stereo: 2 channels
    delayBuffer.clear();

    writePosition = 0;
}

void AudioPluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
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

void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    // 1. Get parameter value
    float delayMs = getDelayTimeMs();
    if (delayMs == 0.0f) return; // nothing to do!

    int sampleRate = static_cast<int>(getSampleRate());
    int maxDelaySamples = static_cast<int>(0.25 * sampleRate); // 250ms max
    int delaySamples = static_cast<int>(std::abs(delayMs) * sampleRate / 1000.0f);

    // 2. Prepare delay buffer if needed
    if (delayBuffer.getNumChannels() != numChannels || delayBuffer.getNumSamples() < maxDelaySamples + numSamples)
    {
        delayBuffer.setSize(numChannels, maxDelaySamples + numSamples, false, true, true);
        delayBuffer.clear();
        writePosition = 0;
    }

    // 3. Process
    for (int channel = 0; channel < numChannels; ++channel)
    {
        float* channelData = buffer.getWritePointer(channel);
        float* delayData = delayBuffer.getWritePointer(channel);

        bool applyDelay =
            (channel == 0 && delayMs < 0.0f) ||   // left channel, negative
            (channel == 1 && delayMs > 0.0f);     // right channel, positive

        for (int i = 0; i < numSamples; ++i)
        {
            int delayBufferSize = delayBuffer.getNumSamples();
            int readPosition = (writePosition + delayBufferSize - delaySamples) % delayBufferSize;

            float delayed = delayData[readPosition];
            delayData[writePosition] = channelData[i];

            channelData[i] = applyDelay ? delayed : channelData[i];

            writePosition = (writePosition + 1) % delayBufferSize;
        }
    }
}


//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

float AudioPluginAudioProcessor::getDelayTimeMs() const
{
    return parameters.getRawParameterValue("delay")->load();
}

juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // Range: -250 ms to +250 ms, centered at 0
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delay", "Delay (ms)", juce::NormalisableRange<float>(-250.0f, 250.0f), 0.0f));

    return { params.begin(), params.end() };
}
