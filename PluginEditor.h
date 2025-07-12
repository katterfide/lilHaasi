#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "PluginProcessor.h"

//==============================================================================
class AudioPluginAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    // reference to the processor to access parameters
    AudioPluginAudioProcessor& audioProcessor;

    // slider for controlling the delay parameter
    juce::Slider delaySlider;

    // label for the delay slider
    juce::Label delayLabel;

    // attachment for linking the slider to the processor parameter
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};