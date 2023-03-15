/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
SampleTune1AudioProcessor::SampleTune1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     :forwardFFT(fftOrder), fifoIndexL(0),fifoIndexR(0), currentSampleRate(44100),
AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    for(int i = 0; i<128;i++)
        {
//            float f =juce::MidiMessage::getMidiNoteInHertz(i);
            frequencies.push_back(juce::MidiMessage::getMidiNoteInHertz(i));
        }
    
}

SampleTune1AudioProcessor::~SampleTune1AudioProcessor()
{
}

//==============================================================================
const juce::String SampleTune1AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SampleTune1AudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SampleTune1AudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SampleTune1AudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SampleTune1AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SampleTune1AudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SampleTune1AudioProcessor::getCurrentProgram()
{
    return 0;
}

void SampleTune1AudioProcessor::setCurrentProgram (int index)
{
}

const juce::String SampleTune1AudioProcessor::getProgramName (int index)
{
    return {};
}

void SampleTune1AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void SampleTune1AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
          currentSampleRate = sampleRate;
        
}

void SampleTune1AudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SampleTune1AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SampleTune1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
  
    
    //define audio buffer
    juce::AudioSampleBuffer mainInputOutput = getBusBuffer(buffer, true, 0);
      
      
    juce::ScopedNoDenormals noDenormals;
      
//      auto mainInputChannels = mainInputOutput.getNumChannels();
//      auto mainOutputChannels = mainInputOutput.getNumChannels();
      
      
      //clear main
//      for (auto i = 0; i < 2; ++i)
//      {
//          mainInputOutput.clear(i, 0, mainInputOutput.getNumSamples());
//          buffer.clear(i,0, buffer.getNumSamples());
//      }

      
      auto* channelDataL = mainInputOutput.getWritePointer(0);
      
      
      //push samples to fifo LR
      for (auto sample = 0; sample < mainInputOutput.getNumSamples(); ++sample)
      {
          float const samp = channelDataL[sample];
          
          
          pushNextSampleIntoFifoL(samp);
      }
      
      auto* channelDataR = mainInputOutput.getWritePointer(1);
      
      for (auto sample = 0; sample < mainInputOutput.getNumSamples(); ++sample)
      {
          float const samp = channelDataR[sample];
          
          
          pushNextSampleIntoFifoR(samp);
      }
      
      
      //STORE DATA FOR AVERAGES - L
      //for the length
      for(int i =  0; i < fftSize/2;i++)
      {
          //add the fft values at one of the 10 points
          
          fftLogL[i][c] = getFFTL(i);
      }
    
      //STORE DATA FOR AVERAGES - R
      //for the length
      for(int i =  0; i < fftSize/2;i++)
      {
          //add the fft values at one of the 10 points
          
          fftLogR[i][c] = getFFTR(i);
      }
      
      
      
      //incrase count (so we go to next row of 2d array nec time)
      c++;
      //if we are at max of array then go back to start
      if(c == thisNoOfAvg)
      {
          c = 0;
      }
      
      //SEE IF THE AVG SHOULD CHANGE
      if(nextAvg != thisNoOfAvg && nextAvg>1 && nextAvg<1000)
      {
          thisNoOfAvg = nextAvg;
          c=0;
      }
    
    
    
    //TUNING ALGORITHM
    //process FFT into relative volumes of chromatic scale
    //work out what is the closest match for each bin => chromatic frequency
        //for each bin
   
    for(int i=1;i<numberOfBins;i++)
    {
        //multiply by the ratio: *22050/noOfBins
        float thisF = i*22050/numberOfBins;
        
        //get midinote
        
//        int m  =  12*log2(thisF/440 ) + 69;
        int m = log(thisF/440.0)/log(2) * 12 + 69;
        
        
        if(thisF == 0) m=0;
        
        if(m>128) break;
        
        
        //** NEED A WAY SO THAT WHEN THE BINS OVERLAP PER MIDI NOTE
        //** WE AVG THE VOL OF EACH BIN THAT RELATES TO THAT NOTE
        
        //add value to synth volume array
        
        if(m !=0)
        {
            m=m+1;
            synthVolumes[m] = getFFTAvgL(i)/1000;
        
            synthVolumesR[m] = getFFTAvgR(i)/1000;
        }
        
        
    }
    
     noteNumberHighest = 0;
    float biggest = 0;
    for(int i=0; i<128;i++)
    {
        
        
        if(synthVolumes[i]>biggest)
        {
            biggest = synthVolumes[i];
            
            noteNumberHighest = i;
            
        }
    }
    
    //ADD THE VOLUMES OF EACH SIN WAVE TO THE BUFFER
    
    for(int i=0; i< mainInputOutput.getNumSamples();i++)
    {
        //LEFT CHANNEL
        auto* dataL = buffer.getWritePointer (0);

        
        
        //get volumes relative to theta position
        for(int n=0;n<128;n++)
        {
                samps[n] = std::sin(thetas[n]) * synthVolumes[n];
        }

        //pass to output
        thisSample=0;
        for(int x=0;x<128;x++)
        {
            thisSample += samps[x];
        }
        dataL[i] = thisSample;

        //increment sin waves
        for(int y=0;y<128;y++)
        {
            thetas[y] = thetas[y] +  frequencies[y] * 2* juce::MathConstants<double>::pi/currentSampleRate;
        }
    
    
        //RIGHT CHANNEL
        auto* dataR = buffer.getWritePointer (1);

        //get volumes relative to theta position
        for(int n=0;n<128;n++)
        {
                sampsR[n] = std::sin(thetasR[n]) * synthVolumesR[n];
            
        }
        
        //pass to output
        thisSample=0;
        for(int x=0;x<128;x++)
        {
            thisSample += sampsR[x];
        }
        
        dataR[i] = thisSample;
            
        //increment sin waves
        for(int y=0;y<128;y++)
        {
            thetasR[y] = thetasR[y] +  frequencies[y] * 2* juce::MathConstants<double>::pi/currentSampleRate;
        }
    }
    
}

//==============================================================================
bool SampleTune1AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* SampleTune1AudioProcessor::createEditor()
{
    return new SampleTune1AudioProcessorEditor (*this);
}

//==============================================================================
void SampleTune1AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SampleTune1AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void SampleTune1AudioProcessor::pushNextSampleIntoFifoL(float sample) noexcept
{
    //if the index has reached the end
    if (fifoIndexL == fftSize)
    {
        //if next block wasn't already ready
        if (!nextFFtBlockReadyL)
        {
            juce::zeromem(fftDataL, sizeof(fftDataL));//clears fftData
            memcpy(fftDataL, fifoL, sizeof(fifoL));//copies memory from fifo to fftData
            nextFFtBlockReadyL = true;//ready to perform FFT
        }
        fifoIndexL = 0;//resests the array counter on fifo to 0
    }
    //if next block is ready
    if (nextFFtBlockReadyL)
    {
        //perform fft calculations
        forwardFFT.performFrequencyOnlyForwardTransform(fftDataL);
        nextFFtBlockReadyL = false;
        readyToDrawL = true;
    }
    
    fifoL[fifoIndexL++] = sample;//information from sample gets pushed into fifo at array point fifoIndex.
    
}


void SampleTune1AudioProcessor::pushNextSampleIntoFifoR(float sample) noexcept
{
    //Ok, so here we are filling fifo with sample data. When fifo gets full, that data
    //gets copied to the first half of fftData. fifo then gets rewritten by next set of data.
    //Also to make the loop work fftData needs to be cleared
    
    
    //if the index has reached the end
    if (fifoIndexR == fftSize)
    {
        //if next block wasn't already ready
        if (!nextFFtBlockReadyR)
        {
            juce::zeromem(fftDataR, sizeof(fftDataR));//clears fftData
            memcpy(fftDataR, fifoR, sizeof(fifoR));//copies memory from fifo to fftData
            nextFFtBlockReadyR = true;//ready to perform FFT
        }
        fifoIndexR = 0;//resests the array counter on fifo to 0
    }
    //if next block is ready
    if (nextFFtBlockReadyR)
    {
        //perform fft calculations
        forwardFFT.performFrequencyOnlyForwardTransform(fftDataR);
        nextFFtBlockReadyR = false;
        readyToDrawR = true;
    }
    
    fifoR[fifoIndexR++] = sample;//information from sample gets pushed into fifo at array point fifoIndex.
    
}


//void SampleTune1AudioProcessor::pushNextSampleIntoFifoMid(float sample) noexcept
//{
//    //Ok, so here we are filling fifo with sample data. When fifo gets full, that data
//    //gets copied to the first half of fftData. fifo then gets rewritten by next set of data.
//    //Also to make the loop work fftData needs to be cleared
//
//
//    //if the index has reached the end
//    if (fifoIndexMid == fftSize)
//    {
//        //if next block wasn't already ready
//        if (!nextFFtBlockReadyMid   )
//        {
//            juce::zeromem(fftDataMid, sizeof(fftDataMid));//clears fftData
//            memcpy(fftDataMid, fifoMid, sizeof(fifoMid));//copies memory from fifo to fftData
//            nextFFtBlockReadyMid = true;//ready to perform FFT
//        }
//        fifoIndexMid = 0;//resests the array counter on fifo to 0
//    }
//    //if next block is ready
//    if (nextFFtBlockReadyMid)
//    {
//        //perform fft calculations
//        forwardFFT.performFrequencyOnlyForwardTransform(fftDataMid);
//        nextFFtBlockReadyMid = false;
////        readyToDrawR = true;
//    }
//
//    fifoMid[fifoIndexMid++] = sample;//information from sample gets pushed into fifo at array point fifoIndex.
//
//}
//
//
//void SampleTune1AudioProcessor::pushNextSampleIntoFifoSide(float sample) noexcept
//{
//    //Ok, so here we are filling fifo with sample data. When fifo gets full, that data
//    //gets copied to the first half of fftData. fifo then gets rewritten by next set of data.
//    //Also to make the loop work fftData needs to be cleared
//
//
//    //if the index has reached the end
//    if (fifoIndexSide == fftSize)
//    {
//        //if next block wasn't already ready
//        if (!nextFFtBlockReadySide)
//        {
//            juce::zeromem(fftDataSide, sizeof(fftDataSide));//clears fftData
//            memcpy(fftDataSide, fifoSide, sizeof(fifoSide));//copies memory from fifo to fftData
//            nextFFtBlockReadySide = true;//ready to perform FFT
//        }
//        fifoIndexSide = 0;//resests the array counter on fifo to 0
//    }
//    //if next block is ready
//    if (nextFFtBlockReadySide)
//    {
//        //perform fft calculations
//        forwardFFT.performFrequencyOnlyForwardTransform(fftDataSide);
//        nextFFtBlockReadySide = false;
////        readyToDrawR = true;
//    }
//
//    fifoSide[fifoIndexSide++] = sample;//information from sample gets pushed into fifo at array point fifoIndex.
//
//}


int SampleTune1AudioProcessor::getFFTSizeL()
{
    return fftSize;
}


int SampleTune1AudioProcessor::getFFTSizeR()
{
    return fftSize;
}

//int SampleTune1AudioProcessor::getFFTSizeMid()
//{
//    return fftSize;
//}
//
//int SampleTune1AudioProcessor::getFFTSizeSide()
//{
//    return fftSize;
//}



float SampleTune1AudioProcessor::getFFTL(int index)
{
    return fftDataL[index];
}


float SampleTune1AudioProcessor::getFFTR(int index)
{
    return fftDataR[index];
}
//float SampleTune1AudioProcessor::getFFTMid(int index)
//{
//    return fftDataMid[index];
//}
//float SampleTune1AudioProcessor::getFFTSide(int index)
//{
//    return fftDataSide[index];
//}

float SampleTune1AudioProcessor::getFFTAvgL(int index)
{
    float total = 0;
    for (int i = 0 ; i <thisNoOfAvg ; i++)
    {
        float thisVal = fftLogL[index][i];
        
        total = total+thisVal;
    }
    float fftAVGL = total/thisNoOfAvg;
    
    return fftAVGL;
    
}

float SampleTune1AudioProcessor::getFFTAvgR(int index)
{
    float total = 0;
    for (int i = 0 ; i <thisNoOfAvg ; i++)
    {
        float thisVal = fftLogR[index][i];
        
        total = total+thisVal;
    }
    float fftAVGR = total/thisNoOfAvg;
    
    return fftAVGR;
    
}

//float SampleTune1AudioProcessor::getFFTAvgMid(int index)
//{
//    float total = 0;
//    for (int i = 0 ; i <thisNoOfAvg ; i++)
//    {
//        float thisVal = fftLogMid[index][i];
//
//        total = total+thisVal;
//    }
//    float fftAVGMid = total/thisNoOfAvg;
//
//    return fftAVGMid;
//
//}
//
//float SampleTune1AudioProcessor::getFFTAvgSide(int index)
//{
//    float total = 0;
//    for (int i = 0 ; i <thisNoOfAvg ; i++)
//    {
//        float thisVal = fftLogSide[index][i];
//
//        total = total+thisVal;
//    }
//    float fftAVGSide = total/thisNoOfAvg;
//
//    return fftAVGSide;
//
//}



float SampleTune1AudioProcessor::getMaxLevelL()
{
    juce::Range<float> maxAndMin = juce::FloatVectorOperations::findMinAndMax(fftDataL, fftSize);
    
    return maxAndMin.getEnd();
}


float SampleTune1AudioProcessor::getMaxLevelR()
{
    juce::Range<float> maxAndMin = juce::FloatVectorOperations::findMinAndMax(fftDataR, fftSize);
    
    return maxAndMin.getEnd();
}
//float SampleTune1AudioProcessor::getMaxLevelMid()
//{
//    juce::Range<float> maxAndMin = juce::FloatVectorOperations::findMinAndMax(fftDataMid, fftSize);
//    
//    return maxAndMin.getEnd();
//}
//float SampleTune1AudioProcessor::getMaxLevelSide()
//{
//    juce::Range<float> maxAndMin = juce::FloatVectorOperations::findMinAndMax(fftDataSide, fftSize);
//    
//    return maxAndMin.getEnd();
//}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SampleTune1AudioProcessor();
}
