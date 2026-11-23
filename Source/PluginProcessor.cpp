#include "PluginProcessor.h"
#include "PluginEditor.h"

SSLoaderAudioProcessor::SSLoaderAudioProcessor()
    : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)) {}

void SSLoaderAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear(); // Utility plugin: no audio generation in V1.
}

juce::AudioProcessorEditor* SSLoaderAudioProcessor::createEditor()
{
    return new SSLoaderAudioProcessorEditor(*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SSLoaderAudioProcessor();
}
