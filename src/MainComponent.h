#pragma once

// CMake builds don't use an AppConfig.h, so it's safe to include juce module headers
// directly. If you need to remain compatible with Projucer-generated builds, and
// have called `juce_generate_juce_header(<thisTarget>)` in your CMakeLists.txt,
// you could `#include <JuceHeader.h>` here instead, to make all your module headers visible.
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/
class MainComponent : public juce::AudioAppComponent,
                      public juce::Button::Listener,
                      public juce::ChangeListener,
                      private juce::Timer
{
public:
    //==============================================================================
    MainComponent();

    ~MainComponent() override;

    //==============================================================================
    void paint(juce::Graphics &) override;
    void resized() override;
    enum transport_state
    {
        Stopped,
        Starting,
        Playing,
        Stopping
    };

    void buttonClicked(juce::Button *) override;

    void changeListenerCallback(juce::ChangeBroadcaster *) override;

    void changeState(transport_state);

    void getNextAudioBlock(const juce::AudioSourceChannelInfo &) override;

    void prepareToPlay(int, double) override;

    void releaseResources() override;

    void transportSourceChanged();

    void thumbnailChanged();

    void paintIfNoFileLoaded(juce::Graphics &, const juce::Rectangle<int> &);

    void paintIfFileLoaded(juce::Graphics &, const juce::Rectangle<int> &);

    void timerCallback() override;

private:
    //==============================================================================
    // Your private member variables go here...
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;

    std::unique_ptr<juce::FileChooser> chooser;

    juce::AudioFormatManager formatManager;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    juce::AudioTransportSource transportSource;
    transport_state state;

    juce::AudioThumbnailCache thumbnailCache;
    juce::AudioThumbnail thumbnail;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
