#pragma once
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <atomic>
#include "orbita.h"

// ÓRBITA 2 plug-in: the same header-only engine the web edition runs as WebAssembly, and the same page as its panel.
class OrbitaProcessor : public juce::AudioProcessor {
public:
  OrbitaProcessor();
  ~OrbitaProcessor() override = default;
  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override {}
  bool isBusesLayoutSupported(const BusesLayout& l) const override { return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo(); }
  void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
  using AudioProcessor::processBlock;
  juce::AudioProcessorEditor* createEditor() override;
  bool hasEditor() const override { return true; }
  const juce::String getName() const override { return "Polaris Orbita"; }
  bool acceptsMidi() const override { return true; }
  bool producesMidi() const override { return true; }
  bool isMidiEffect() const override { return false; }
  double getTailLengthSeconds() const override { return 8.0; }
  int getNumPrograms() override { return 1; }
  int getCurrentProgram() override { return 0; }
  void setCurrentProgram(int) override {}
  const juce::String getProgramName(int) override { return "ORBITA"; }
  void changeProgramName(int, const juce::String&) override {}
  void getStateInformation(juce::MemoryBlock&) override;
  void setStateInformation(const void*, int) override;

  // ---- message thread (the page)
  void message(const juce::var& m);
  juce::var stateForPage();
  void setUi(const juce::String& s) { const juce::SpinLock::ScopedLockType l(uiLock); uiJson = s; }
  void snapshot(float* tele, float* L, float* R);
  std::vector<std::pair<int, float>> takeHostChanges();   // parameters the DAW moved, for the page

private:
  enum CmdT : uint8_t { C_PARAM, C_TARGET, C_CLEARTARGET, C_PLAY, C_STOP, C_KEY, C_CUSTOM, C_AUDITION, C_EVO };
  struct Cmd { CmdT t; int a, b; float v; };
  static constexpr int QN = 8192;
  Cmd q[QN]; juce::AbstractFifo fifo { QN };
  void push(Cmd c) { int s1, n1, s2, n2; fifo.prepareToWrite(1, s1, n1, s2, n2); if (n1 > 0) q[s1] = c; else if (n2 > 0) q[s2] = c; fifo.finishedWrite(n1 + n2); }
  void drain();

  // the DAW's parameters: a curated set of the engine's (the rest are in the page and the project)
  struct HostP { int eng; float lo, hi; bool stepped; };
  std::vector<HostP> hp; std::vector<juce::RangedAudioParameter*> params;
  int hostOf[orbita::P_COUNT];
  std::unique_ptr<std::atomic<float>[]> lastHost;      // the normalised value the engine has
  std::unique_ptr<std::atomic<uint8_t>[]> changed;     // 1 = the DAW moved it (for the page)
  void setHost(int engId, float v);                    // the page moved an engine parameter that the DAW has
  float norm(int i, float x) const { auto& h = hp[(size_t)i]; return juce::jlimit(0.f, 1.f, (x - h.lo) / (h.hi - h.lo)); }

  std::unique_ptr<orbita::Engine> eng;
  float shP[orbita::P_COUNT], shT[orbita::P_COUNT]; bool shHasT = false; int shC[orbita::NL] = {-1, -1, -1, -1};
  void shadowParam(int id, float v);
  double sr = 48000;
  std::atomic<bool> sync { true }, midiOut { false };
  juce::String uiJson; juce::SpinLock uiLock;
  juce::SpinLock teleLock; float tele[64] = {}, scL[256] = {}, scR[256] = {};
  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(OrbitaProcessor)
};
