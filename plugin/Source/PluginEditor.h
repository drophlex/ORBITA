#pragma once
#include "PluginProcessor.h"

// The panel is the web edition's page, in the system's web view, talking to the native engine.
class OrbitaEditor : public juce::AudioProcessorEditor, private juce::Timer {
public:
  explicit OrbitaEditor(OrbitaProcessor&);
  ~OrbitaEditor() override;
  void resized() override;
  void paint(juce::Graphics& g) override { g.fillAll(juce::Colour(0xffe9e8df)); }
private:
  void timerCallback() override;
  juce::WebBrowserComponent::Options makeOptions();
  std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);
  OrbitaProcessor& proc;
  std::unique_ptr<juce::WebBrowserComponent> web;
  float tele[64] = {}, L[256] = {}, R[256] = {};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaEditor)
};
