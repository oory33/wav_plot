#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
    : state(Stopped),
      thumbnailCache(5),
      thumbnail(512, formatManager, thumbnailCache)
{
    formatManager.registerBasicFormats();
    transportSource.addChangeListener(this);
    thumbnail.addChangeListener(this);

    addAndMakeVisible(&openButton);
    openButton.setButtonText("Open...");
    openButton.addListener(this);

    addAndMakeVisible(&playButton);
    playButton.setButtonText("Play");
    playButton.addListener(this);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    playButton.setEnabled(false);

    addAndMakeVisible(&stopButton);
    stopButton.setButtonText("Stop");
    stopButton.addListener(this);
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    stopButton.setEnabled(false);

    setAudioChannels(0, 2);
    setSize(600, 400);
    startTimer(40);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
}

//==============================================================================

void MainComponent::paintIfNoFileLoaded(juce::Graphics &g, const juce::Rectangle<int> &thumbnailBounds)
{
    g.setColour(juce::Colours::darkgrey);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::white);
    g.drawFittedText("No file loaded", thumbnailBounds, juce::Justification::centred, 1);
}

void MainComponent::paintIfFileLoaded(juce::Graphics &g, const juce::Rectangle<int> &thumbnailBounds)
{
    g.setColour(juce::Colours::white);
    g.fillRect(thumbnailBounds);
    g.setColour(juce::Colours::orange);

    auto audioLength = (float)thumbnail.getTotalLength();
    thumbnail.drawChannels(g, thumbnailBounds, 0.0, thumbnail.getTotalLength(), 0.8f);

    g.setColour(juce::Colours::greenyellow);

    auto audioPosition = (float)transportSource.getCurrentPosition();
    auto drawPosition = (audioPosition / audioLength) * (float)thumbnailBounds.getWidth() + (float)thumbnailBounds.getX();

    g.drawLine(drawPosition, (float)thumbnailBounds.getY(), drawPosition, (float)thumbnailBounds.getBottom(), 2.0f);
}

void MainComponent::paint(juce::Graphics &g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    juce::Rectangle<int> thumbnailBounds(10, 100, getWidth() - 20, getHeight() - 120);
    if (thumbnail.getNumChannels() == 0)
        paintIfNoFileLoaded(g, thumbnailBounds);
    else
        paintIfFileLoaded(g, thumbnailBounds);
}

void MainComponent::resized()
{
    // This is called when the MainComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    openButton.setBounds(10, 10, getWidth() - 20, 20);
    playButton.setBounds(10, 40, getWidth() - 20, 20);
    stopButton.setBounds(10, 70, getWidth() - 20, 20);
}

void MainComponent::transportSourceChanged()
{
    changeState(transportSource.isPlaying() ? Playing : Stopped);
}

void MainComponent::thumbnailChanged()
{
    repaint();
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster *source)
{
    if (source == &transportSource)
    {
        transportSourceChanged();
    }
    if (source == &thumbnail)
    {
        thumbnailChanged();
    }
}

void MainComponent::timerCallback()
{
    repaint();
}

void MainComponent::changeState(transport_state newState)
{
    if (state != newState)
    {
        state = newState;
        switch (state)
        {
        case Stopped:
            stopButton.setEnabled(false);
            playButton.setEnabled(true);
            transportSource.setPosition(0.0);
            break;
        case Starting:
            playButton.setEnabled(false);
            transportSource.start();
            break;
        case Playing:
            stopButton.setEnabled(true);
            break;
        case Stopping:
            transportSource.stop();
            break;
        }
    }
}

void MainComponent::getNextAudioBlock(const juce::AudioSourceChannelInfo &bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }
    transportSource.getNextAudioBlock(bufferToFill);
}

void MainComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate)
{
    transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::releaseResources()
{
    transportSource.releaseResources();
}

void MainComponent::buttonClicked(juce::Button *button)
{
    if (button == &openButton)
    {
        chooser = std::make_unique<juce::FileChooser>("Select a file to play...", juce::File{}, "*.wav;*.aif;*.aiff");

        auto chooserFlags = juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser &fc)
                             {
            auto file = fc.getResult();

            if (file != juce::File{})
            {
                auto* reader = formatManager.createReaderFor (file);
                if (reader != nullptr)
                {
                    auto newSource = std::make_unique<juce::AudioFormatReaderSource>(reader, true);
                    transportSource.setSource(newSource.get(), 0, nullptr, reader->sampleRate);
                    playButton.setEnabled(true);
                    thumbnail.setSource(new juce::FileInputSource(file));
                        readerSource.reset(newSource.release());
                }
            } });
    }
    else if (button == &playButton)
    {
        changeState(Starting);
    }
    else if (button == &stopButton)
    {
        changeState(Stopping);
    }
}
