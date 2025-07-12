#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
// constructor for setting up the processor and parameters
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
// plugin name
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

// midi support checks
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

// tail length of audio
double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

// program methods not really used for simple plugins
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
// called when starting playback or changing sample rate
void AudioPluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // calculate buffer size for up to 500 ms delay
    int maxDelaySamples = static_cast<int>(0.5 * sampleRate);

    // create a stereo buffer for the delay
    delayBuffer.setSize(2, maxDelaySamples);
    delayBuffer.clear();

    // reset write position to start
    delayWritePosition = 0;
}

// called when playback stops
void AudioPluginAudioProcessor::releaseResources()
{
    // nothing to do here for this plugin
}

// bus layout support check for hosts
bool AudioPluginAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // only allow mono or stereo
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    // input and output must match
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}

// this is called for every block of audio
void AudioPluginAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    float delayMs = getDelayTimeMs();
    int sampleRate = static_cast<int>(getSampleRate());
    int maxDelaySamples = static_cast<int>(0.25 * sampleRate);
    int delaySamples = static_cast<int>(std::abs(delayMs) * sampleRate / 1000.0f);

    // if buffer size or channels change we resize and clear the delay buffer
    if (delayBuffer.getNumChannels() != numChannels || delayBuffer.getNumSamples() < maxDelaySamples + numSamples)
    {
        delayBuffer.setSize(numChannels, maxDelaySamples + numSamples, false, true, true);
        delayBuffer.clear();
        delayWritePosition = 0;
    }

    bool delayLeft = (delayMs < 0.0f);
    bool delayRight = (delayMs > 0.0f);

    for (int i = 0; i < numSamples; ++i)
    {
        for (int channel = 0; channel < numChannels; ++channel)
        {
            float* channelData = buffer.getWritePointer(channel);
            float* delayData = delayBuffer.getWritePointer(channel);

            // write input sample to the delay buffer
            delayData[delayWritePosition] = channelData[i];

            float out = channelData[i];

            // read delayed sample if this channel is delayed
            if ((channel == 0 && delayLeft) || (channel == 1 && delayRight))
            {
                int delayBufferSize = delayBuffer.getNumSamples();
                int readPos = (delayWritePosition + delayBufferSize - delaySamples) % delayBufferSize;
                out = delayData[readPos];
            }
            channelData[i] = out;
        }
        // move the write position forward after each sample
        delayWritePosition++;
        if (delayWritePosition >= delayBuffer.getNumSamples())
            delayWritePosition = 0;
    }
}

//==============================================================================
// this tells the host the plugin has a gui editor
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;
}

// creates the gui editor window
juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    return new AudioPluginAudioProcessorEditor (*this);
}

//==============================================================================
// these are for saving and loading plugin state
void AudioPluginAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    juce::ignoreUnused (destData);
}

void AudioPluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    juce::ignoreUnused (data, sizeInBytes);
}

//==============================================================================
// factory method for making new plugin instances
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}

// get the delay parameter value in ms
float AudioPluginAudioProcessor::getDelayTimeMs() const
{
    return parameters.getRawParameterValue("delay")->load();
}

// create the parameter layout for the plugin
juce::AudioProcessorValueTreeState::ParameterLayout AudioPluginAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    // use a gentle skew so the slider is most sensitive near zero
    juce::NormalisableRange<float> delayRange(
        -250.0f, 250.0f,
        // map normalised slider to delay value
        [](float start, float end, float normalised) {
            float mid = (end + start) / 2.0f;
            float half = (end - start) / 2.0f;
            float skew = 1.6f;
            float norm = (normalised - 0.5f) * 2.0f;
            return mid + std::copysign(std::pow(std::abs(norm), skew) * half, norm);
        },
        // map delay value back to normalised slider
        [](float start, float end, float value) {
            float mid = (end + start) / 2.0f;
            float half = (end - start) / 2.0f;
            float skew = 1.6f;
            float norm = (value - mid) / half;
            return 0.5f + 0.5f * std::copysign(std::pow(std::abs(norm), 1.0f / skew), norm);
        }
    );

    // add the delay parameter with custom curve
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "delay", "Delay (ms)", delayRange, 0.0f));

    // return all parameters
    return { params.begin(), params.end() };
}