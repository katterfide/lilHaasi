#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // this sets the plugin window size
    setSize (400, 200);

    // set up the delay slider
    delaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    delaySlider.setRange(-250.0f, 250.0f, 0.1f);
    delaySlider.setValue(0.0f);
    delaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);

    // this attaches the slider to the parameter in the processor
    delayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTree(), "delay", delaySlider));

    // add the slider to the editor window
    addAndMakeVisible(delaySlider);

    // label for the delay slider
    delayLabel.setText("delay ms", juce::dontSendNotification);
    delayLabel.attachToComponent(&delaySlider, false);
    addAndMakeVisible(delayLabel);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    // nothing to clean up manually here
}

void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // fill the background with a solid color
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // set the color and draw the plugin name in the center
    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("lilhaasi delay plugin", getLocalBounds(), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // set the position and size of the slider
    delaySlider.setBounds (40, 80, getWidth() - 80, 40);
}