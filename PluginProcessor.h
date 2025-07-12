#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

//==============================================================================
// main audio processor class for the plugin
class AudioPluginAudioProcessor final : public juce::AudioProcessor
{
public:
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    // parameter layout and access
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    float getDelayTimeMs() const;
    juce::AudioProcessorValueTreeState& getValueTree() { return parameters; }

    // audio processor overrides
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    // gui editor
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    // plugin info
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    // program management
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    // state saving
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessor)

    // parameters for the plugin
    juce::AudioProcessorValueTreeState parameters;

    // buffer for storing delayed audio
    juce::AudioBuffer<float> delayBuffer;

    // current write position in the delay buffer
    int delayWritePosition = 0;
};