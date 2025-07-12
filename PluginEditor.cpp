#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // set the plugin window size
    setSize (400, 220);

    // set up the delay slider
    delaySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    delaySlider.setRange(-250.0f, 250.0f, 0.01f); // very fine step size
    delaySlider.setSkewFactorFromMidPoint(0.0f); // this keeps the zero point central
    delaySlider.setValue(0.0f);
    delaySlider.setNumDecimalPlacesToDisplay(2);
    delaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 16);

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
    g.drawFittedText ("lilHaasi", getLocalBounds().removeFromTop(40), juce::Justification::centred, 1);

    // draw small subtitle under plugin name
    g.setFont (14.0f);
    g.drawFittedText ("just a simple haas effect", juce::Rectangle<int>(0, 40, getWidth(), 20), juce::Justification::centred, 1);

    // draw company name at bottom right
    g.setFont (12.0f);
    g.drawFittedText ("Altschuld", juce::Rectangle<int>(getWidth() - 100, getHeight() - 20, 100, 20), juce::Justification::centredRight, 1);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // define the area for controls, leaving space for the plugin name
    auto area = getLocalBounds().reduced(40);
    area.removeFromTop(40); // space for plugin name

    int sliderHeight = 30;
    int labelHeight = 20;

    // place slider horizontally
    delaySlider.setBounds(area.removeFromTop(sliderHeight));

    // place label below the slider
    delayLabel.setBounds(area.removeFromTop(labelHeight));
}