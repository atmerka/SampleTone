/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class SampleTune1AudioProcessorEditor  : public juce::AudioProcessorEditor, public juce::Timer
{
public:
    SampleTune1AudioProcessorEditor (SampleTune1AudioProcessor&);
    ~SampleTune1AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

    
    void timerCallback() override;
    
    juce::String note;
private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    SampleTune1AudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTune1AudioProcessorEditor)
};
