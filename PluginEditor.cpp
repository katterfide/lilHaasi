#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // set the plugin window size
    setSize (400, 220);

    // set up the delay slider
    delaySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag); // use rotary slider for a modern look
    delaySlider.setRange(-250.0f, 250.0f, 0.1f);
    delaySlider.setValue(0.0f);
    delaySlider.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0); // no textbox for cleaner look
    delaySlider.setPopupDisplayEnabled(true, false, this); // show value popup on drag
    delaySlider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colour(0xff1abc9c)); // teal accent

    // attach the slider to the parameter in the processor
    delayAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        audioProcessor.getValueTree(), "delay", delaySlider));

    // add the slider to the editor window
    addAndMakeVisible(delaySlider);

    // set up the label for the slider
    delayLabel.setText("delay ms", juce::dontSendNotification);
    delayLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(delayLabel);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    // nothing to clean up manually here
}

void AudioPluginAudioProcessorEditor::paint (juce::Graphics& g)
{
    // fill the background with dark gray
    g.fillAll (juce::Colour (0xff23272e));

    // set color and draw the plugin name at the top
    g.setColour (juce::Colours::white);
    g.setFont (24.0f);
    g.drawFittedText ("simple Haas", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // layout the controls
    auto area = getLocalBounds().reduced(40);
    area.removeFromTop(40); // remove space for plugin name

    // place the slider in the center
    auto sliderArea = area.removeFromTop(120);
    delaySlider.setBounds(sliderArea.withSizeKeepingCentre(120, 120));

    // place the label just below the slider
    delayLabel.setBounds(sliderArea.withTrimmedTop(110).withHeight(20));
}