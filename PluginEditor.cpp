/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SampleTune1AudioProcessorEditor::SampleTune1AudioProcessorEditor (SampleTune1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    setSize (400   , 200);
    
    startTimer(20);
}

SampleTune1AudioProcessorEditor::~SampleTune1AudioProcessorEditor()
{
}

//==============================================================================
void SampleTune1AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll(juce::Colours::black);

    g.setColour (juce::Colours::green);
    g.setFont (50.0f);
    
//     note = "NO NOTE";
    note = "MIDI NOTE: " + (juce::String)audioProcessor.noteNumberHighest;
    g.drawFittedText (note, getLocalBounds(), juce::Justification::centred, 1);
}

void SampleTune1AudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
}

void SampleTune1AudioProcessorEditor::timerCallback()
{
    repaint();
}
