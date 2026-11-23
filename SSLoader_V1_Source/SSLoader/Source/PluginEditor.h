#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include <atomic>
#include <memory>

class DownloadWorker final : public juce::Thread
{
public:
    DownloadWorker(juce::String url, juce::File destination,
                   std::function<void(juce::String)> completion);
    void run() override;
private:
    juce::String address;
    juce::File output;
    std::function<void(juce::String)> done;
};

class SSLoaderAudioProcessorEditor final : public juce::AudioProcessorEditor,
                                           private juce::Timer
{
public:
    explicit SSLoaderAudioProcessorEditor(SSLoaderAudioProcessor&);
    ~SSLoaderAudioProcessorEditor() override;
    void paint(juce::Graphics&) override;
    void resized() override;
private:
    void startDownload();
    void timerCallback() override;
    SSLoaderAudioProcessor& processor;
    juce::Label heading, subtitle, urlLabel, status;
    juce::TextEditor urlInput;
    juce::TextButton pasteButton { "Paste" }, downloadButton { "Download" };
    juce::ComboBox formatBox;
    std::unique_ptr<DownloadWorker> worker;
    std::shared_ptr<juce::String> result;
    juce::CriticalSection resultLock;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SSLoaderAudioProcessorEditor)
};
