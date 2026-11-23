#include "PluginEditor.h"

DownloadWorker::DownloadWorker(juce::String url, juce::File destination,
                               std::function<void(juce::String)> completion)
    : juce::Thread("SSLoader Download"), address(std::move(url)),
      output(std::move(destination)), done(std::move(completion)) {}

void DownloadWorker::run()
{
    juce::URL url(address);
    const auto scheme = url.getScheme().toLowerCase();
    if (scheme != "https" && scheme != "http") { done("Only HTTP/HTTPS URLs are supported"); return; }

    auto options = juce::URL::InputStreamOptions(juce::URL::ParameterHandling::inAddress)
                       .withConnectionTimeoutMs(15000)
                       .withNumRedirectsToFollow(3);
    auto stream = url.createInputStream(options);
    if (stream == nullptr) { done("Connection failed or source unavailable"); return; }

    const auto responseType = stream->getTotalLength();
    if (responseType > 500LL * 1024 * 1024) { done("File exceeds 500 MB limit"); return; }
    juce::TemporaryFile temp(output);
    auto fileStream = std::unique_ptr<juce::FileOutputStream>(temp.getFile().createOutputStream());
    if (!fileStream) { done("Cannot write destination file"); return; }

    char buffer[32768];
    int64_t total = 0;
    while (!threadShouldExit())
    {
        const int n = stream->read(buffer, sizeof(buffer));
        if (n < 0) { done("Download read error"); return; }
        if (n == 0) break;
        total += n;
        if (total > 500LL * 1024 * 1024) { done("File exceeds 500 MB limit"); return; }
        if (!fileStream->write(buffer, static_cast<size_t>(n))) { done("Disk write failed"); return; }
    }
    if (threadShouldExit()) return;
    fileStream->flush();
    fileStream.reset();
    if (total == 0 || (responseType >= 0 && total != responseType))
    { done("Incomplete or empty download"); return; }
    if (!temp.overwriteTargetFileWithTemporary()) { done("Could not save completed file"); return; }
    done("Saved: " + output.getFullPathName());
}

SSLoaderAudioProcessorEditor::SSLoaderAudioProcessorEditor(SSLoaderAudioProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    setSize(760, 420);
    heading.setText("SSLoader", juce::dontSendNotification);
    heading.setFont(juce::FontOptions(30.0f, juce::Font::bold));
    heading.setColour(juce::Label::textColourId, juce::Colours::white);
    subtitle.setText("Download authorized direct audio files", juce::dontSendNotification);
    subtitle.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    urlLabel.setText("DIRECT AUDIO URL", juce::dontSendNotification);
    urlLabel.setColour(juce::Label::textColourId, juce::Colours::lightblue);
    urlInput.setTextToShowWhenEmpty("https://example.com/audio.wav", juce::Colours::grey);
    urlInput.setColour(juce::TextEditor::backgroundColourId, juce::Colour(0xff202b39));
    urlInput.setColour(juce::TextEditor::textColourId, juce::Colours::white);
    urlInput.setColour(juce::TextEditor::outlineColourId, juce::Colour(0xff40566a));
    formatBox.addItem("Keep original format", 1);
    formatBox.setSelectedId(1);
    formatBox.setEnabled(false); // Conversion comes in V2.
    status.setText("Ready. Direct file URLs only in V1.", juce::dontSendNotification);
    status.setColour(juce::Label::textColourId, juce::Colours::lightgrey);
    for (auto* c : { static_cast<juce::Component*>(&heading), &subtitle, &urlLabel,
                      &urlInput, &pasteButton, &downloadButton, &formatBox, &status })
        addAndMakeVisible(c);
    pasteButton.onClick = [this] { urlInput.setText(juce::SystemClipboard::getTextFromClipboard()); };
    downloadButton.onClick = [this] { startDownload(); };
    startTimerHz(5);
}

SSLoaderAudioProcessorEditor::~SSLoaderAudioProcessorEditor()
{
    stopTimer();
    if (worker) worker->stopThread(20000);
}

void SSLoaderAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff111923));
    g.setColour(juce::Colour(0xff1b2735));
    g.fillRoundedRectangle(22.0f, 112.0f, getWidth() - 44.0f, 260.0f, 15.0f);
    g.setColour(juce::Colour(0xff50d4e7));
    g.fillRoundedRectangle(25.0f, 113.0f, 5.0f, 258.0f, 2.0f);
}

void SSLoaderAudioProcessorEditor::resized()
{
    heading.setBounds(28, 22, 420, 44);
    subtitle.setBounds(30, 66, 620, 26);
    urlLabel.setBounds(48, 132, 500, 24);
    urlInput.setBounds(48, 165, 530, 44);
    pasteButton.setBounds(592, 165, 110, 44);
    formatBox.setBounds(48, 231, 240, 38);
    downloadButton.setBounds(48, 292, 654, 45);
    status.setBounds(48, 345, 650, 24);
}

void SSLoaderAudioProcessorEditor::startDownload()
{
    if (worker && worker->isThreadRunning()) return;
    const auto address = urlInput.getText().trim();
    const juce::URL url(address);
    if (!url.isWellFormed() || (url.getScheme() != "https" && url.getScheme() != "http"))
    { status.setText("Enter a valid HTTP/HTTPS URL", juce::dontSendNotification); return; }
    const auto path = url.getSubPath();
    const auto filename = juce::File::createLegalFileName(path.fromLastOccurrenceOf("/", false, false));
    const auto ext = juce::File(filename).getFileExtension().toLowerCase();
    if (!juce::StringArray { ".mp3", ".wav", ".flac", ".m4a", ".ogg", ".aac" }.contains(ext))
    { status.setText("V1 requires a direct audio file URL (.mp3/.wav/.flac/...)", juce::dontSendNotification); return; }

    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                      .getChildFile("SSLoader").getChildFile("Downloads");
    if (!folder.createDirectory().wasOk())
    { status.setText("Cannot create Downloads folder", juce::dontSendNotification); return; }
    auto destination = folder.getNonexistentChildFile(juce::File(filename).getFileNameWithoutExtension(), ext, false);
    { const juce::ScopedLock lock(resultLock); result.reset(); }
    worker = std::make_unique<DownloadWorker>(address, destination, [this](juce::String message)
    {
        const juce::ScopedLock lock(resultLock);
        result = std::make_shared<juce::String>(std::move(message));
    });
    status.setText("Downloading...", juce::dontSendNotification);
    downloadButton.setEnabled(false);
    worker->startThread();
}

void SSLoaderAudioProcessorEditor::timerCallback()
{
    std::shared_ptr<juce::String> completed;
    { const juce::ScopedLock lock(resultLock); completed.swap(result); }
    if (completed)
    {
        status.setText(*completed, juce::dontSendNotification);
        downloadButton.setEnabled(true);
    }
}
