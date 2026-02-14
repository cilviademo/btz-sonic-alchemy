#include "PluginEditor.h"
#include <cstdlib>

BTZAudioProcessorEditor::BTZAudioProcessorEditor(BTZAudioProcessor& p)
    : juce::AudioProcessorEditor(&p), processor(p)
{
    setSize(1280, 760);

    const auto indexPath = getFrontendIndexFilePath();
    if (indexPath.isEmpty())
    {
        statusMessage = "Frontend UI not found.\nBuild it with: npm run build (in btz-sonic-alchemy-main root).";
        return;
    }

    auto options = juce::WebBrowserComponent::Options{};
    webView = std::make_unique<juce::WebBrowserComponent>(options);
    addAndMakeVisible(*webView);

    auto url = "file:///" + indexPath.replaceCharacter('\\', '/');
    webView->goToURL(url);
}

void BTZAudioProcessorEditor::paint(juce::Graphics& g)
{
    if (webView != nullptr)
        return;

    g.fillAll(juce::Colour::fromRGB(10, 10, 14));
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawFittedText("BTZ - UI unavailable", getLocalBounds().removeFromTop(42), juce::Justification::centred, 1);

    g.setFont(15.0f);
    g.drawFittedText(statusMessage,
                     getLocalBounds().reduced(24),
                     juce::Justification::centredTop,
                     4);
}

void BTZAudioProcessorEditor::resized()
{
    if (webView != nullptr)
        webView->setBounds(getLocalBounds());
}

juce::String BTZAudioProcessorEditor::getFrontendIndexFilePath() const
{
    if (const auto* envDist = std::getenv("BTZ_UI_DIST"))
    {
        const juce::File fromEnv(envDist);
        const juce::File envIndex = fromEnv.isDirectory() ? fromEnv.getChildFile("index.html") : fromEnv;
        if (envIndex.existsAsFile())
            return envIndex.getFullPathName();
    }

#ifdef BTZ_UI_DIST_PATH
    const juce::File fromCompileDef(BTZ_UI_DIST_PATH);
    const juce::File compileDefIndex = fromCompileDef.getChildFile("index.html");
    if (compileDefIndex.existsAsFile())
        return compileDefIndex.getFullPathName();
#endif

    return {};
}
