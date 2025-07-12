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
    return 1;
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
    // set up a delay buffer for up to 500ms stereo delay
    int maxDelaySamples = static_cast<int>(0.5 * sampleRate);
    delayBuffer.setSize(2, maxDelaySamples);
    delayBuffer.clear();
    delayWritePosition = 0;
}

void AudioPluginAudioProcessor::releaseResources()
{
    // nothing to release
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
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

    float delayMs = getDelayTimeMs();
    int sampleRate = static_cast<int>(getSampleRate());
    int delaySamples = static_cast<int>(delayMs * sampleRate / 1000.0f);

    int side = getDelaySide(); // 0 = left, 1 = right

    // setup delay buffer if needed
    int maxDelaySamples = static_cast<int>(0.5 * sampleRate);
    if (delayBuffer.getNumChannels() != numChannels || delayBuffer.getNumSamples() < maxDelaySamples + numSamples)
    {
        delayBuffer.setSize(numChannels, maxDelaySamples + numSamples, false, true, true);
        delayBuffer.clear();
        delayWritePosition = 0;
    }

    for (int i = 0; i < numSamples; ++i)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float* delayData = delayBuffer.getWritePointer(channel);

            delayData[delayWritePosition] = channelData[i];
            float out = channelData[i];

            // only delay the chosen channel
            if (channel == side)
            {
                int delayBufferSize = delayBuffer.getNumSamples();
                int readPos = (delayWritePosition + delayBufferSize - delaySamples) % delayBufferSize;
                out = delayData[readPos];
            }
            channelData[i] = out;
        }
        delayWritePosition++;
        if (delayWritePosition >= delayBuffer.getNumSamples())
            delayWritePosition = 0;
    }
}

//==============================================================================

bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================

void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

// get the delay time parameter value in ms
float AudioPluginAudioProcessor::getDelayTimeMs() const
{
    return parameters.getRawParameterValue("delay")->load();
}

// get the selected side parameter (0 = left, 1 = right)
int AudioPluginAudioProcessor::getDelaySide() const
{
    return (int)parameters.getRawParameterValue("side")->load();
}

// define the parameter layout for the plugin
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // create a positive only skewed range for delay in ms
    auto delayRange = juce::NormalisableRange<float>(0.01f, 250.0f, 0.01f);
    delayRange.setSkewForCentre(5.0f); // this makes center more sensitive

    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delay", "delay ms", delayRange, 1.0f));

    params.push_back(std::make_unique<juce::AudioParameterChoice>(
        "side", "side", juce::StringArray{ "left", "right" }, 0));

    return { params.begin(), params.end() };
}