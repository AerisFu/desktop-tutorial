#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400, 300);

    // Decay
    addAndMakeVisible(decaySlider);
    decaySlider.setSliderStyle(juce::Slider::LinearVertical);
    decaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "decay", decaySlider);

    addAndMakeVisible(decayLabel);
    decayLabel.setText("Decay", juce::dontSendNotification);
    decayLabel.attachToComponent(&decaySlider, false);

    // Pre-Delay
    addAndMakeVisible(preDelaySlider);
    preDelaySlider.setSliderStyle(juce::Slider::LinearVertical);
    preDelaySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    preDelayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "preDelay", preDelaySlider);

    addAndMakeVisible(preDelayLabel);
    preDelayLabel.setText("Pre-Delay", juce::dontSendNotification);
    preDelayLabel.attachToComponent(&preDelaySlider, false);

    // Wet/Dry
    addAndMakeVisible(wetDrySlider);
    wetDrySlider.setSliderStyle(juce::Slider::LinearVertical);
    wetDrySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    wetDryAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "wetDry", wetDrySlider);

    addAndMakeVisible(wetDryLabel);
    wetDryLabel.setText("Wet/Dry", juce::dontSendNotification);
    wetDryLabel.attachToComponent(&wetDrySlider, false);

    // Size
    addAndMakeVisible(sizeSlider);
    sizeSlider.setSliderStyle(juce::Slider::LinearVertical);
    sizeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    sizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "size", sizeSlider);

    addAndMakeVisible(sizeLabel);
    sizeLabel.setText("Size", juce::dontSendNotification);
    sizeLabel.attachToComponent(&sizeSlider, false);

    // Damping
    addAndMakeVisible(dampingSlider);
    dampingSlider.setSliderStyle(juce::Slider::LinearVertical);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 80, 20);
    dampingAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(processor.parameters, "damping", dampingSlider);

    addAndMakeVisible(dampingLabel);
    dampingLabel.setText("Damping", juce::dontSendNotification);
    dampingLabel.attachToComponent(&dampingSlider, false);
}

PluginEditor::~PluginEditor()
{
}

//==============================================================================
void PluginEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    g.setColour (juce::Colours::white);
    g.setFont (15.0f);
    g.drawFittedText ("Ultimate Reverb", getLocalBounds(), juce::Justification::centred, 1);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto sliderWidth = area.getWidth() / 5;

    decaySlider.setBounds(area.removeFromLeft(sliderWidth).withTrimmedTop(20));
    preDelaySlider.setBounds(area.removeFromLeft(sliderWidth).withTrimmedTop(20));
    wetDrySlider.setBounds(area.removeFromLeft(sliderWidth).withTrimmedTop(20));
    sizeSlider.setBounds(area.removeFromLeft(sliderWidth).withTrimmedTop(20));
    dampingSlider.setBounds(area.removeFromLeft(sliderWidth).withTrimmedTop(20));
}