/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/
class SampleTune1AudioProcessor  : public juce::AudioProcessor
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    SampleTune1AudioProcessor();
    ~SampleTune1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    
    //FFT
    void pushNextSampleIntoFifoL(float sample) noexcept;
    void pushNextSampleIntoFifoR(float sample) noexcept;
    

//    void pushNextSampleIntoFifoMid(float sample) noexcept;
//    void pushNextSampleIntoFifoSide(float sample) noexcept;
    
    //order (complexity of fft)
    enum
    {
        fftOrder = 9,
        fftSize = 1 << fftOrder
    };
    
    
    //functions to call values from the editor
    float getFFTL(int index);
    float getMaxLevelL();
    int getFFTSizeL();
    
    
    float getFFTR(int index);
    float getMaxLevelR();
    int getFFTSizeR();
    
//    float getFFTMid(int index);
//    float getMaxLevelMid();
//    int getFFTSizeMid();
//
//    float getFFTSide(int index);
//    float getMaxLevelSide();
//    int getFFTSizeSide();
    
    //fftdata arrays
    float fftDataL[2 * fftSize];
    float fftDataR[2 * fftSize];
//    float fftDataMid[2 * fftSize];
//    float fftDataSide[2 * fftSize];
    
    //define fft
    juce::dsp::FFT forwardFFT;
    
    //booleans for logic
    bool nextFFtBlockReadyL = false;
    bool nextFFtBlockReadyR = false;
//    bool nextFFtBlockReadyMid = false;
//    bool nextFFtBlockReadySide = false;
    bool readyToDrawL = false;
    bool readyToDrawR = false;
    
    float frequency;
    
    
    
    //for taking averages
    
    //total size (number of averages of 2nd dimension of array
    enum{ totalNoOfAvg = 500};
    float thisNoOfAvg = 5;
    int nextAvg;
    
    float fftLogL[fftSize/2][totalNoOfAvg];
    float getFFTAvgL(int index);
    
    float fftLogR[fftSize/2][totalNoOfAvg];
    float getFFTAvgR(int index);
    
//    float fftLogMid[fftSize/2][totalNoOfAvg];
//    float getFFTAvgMid(int index);
//
//    float fftLogSide[fftSize/2][totalNoOfAvg];
//    float getFFTAvgSide(int index);
    
    int noteNumberHighest=0;
    
private:
    
    //frequencies of chromatic
    std::vector<float> frequencies;
    
    //synth volumes L
    float synthVolumes[127] ;
    double thetas[127];
    float samps[127];
   
    //synth volumes R
     float synthVolumesR[127] ;
     double thetasR[127];
     float sampsR[127];
    
    float thisSample = 0;
    
    
    //for sin wave1
    float  samp60;
    float gain60;
    double frequency60;
    double theta60;
    
    
    
    
    
    //variables
    double currentSampleRate;
    
    //count for averaging
    int c = 0;
    
    
    //fifo array for DSP
    float fifoL[fftSize];
    int fifoIndexL;
    
    
    float fifoR[fftSize];
    int fifoIndexR;
    
    
//    float fifoMid[fftSize];
//    int fifoIndexMid;
//
//
//    float fifoSide[fftSize];
//    int fifoIndexSide;
//
    
    int numberOfBins = fftSize / 2;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleTune1AudioProcessor)
};
