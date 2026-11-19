#include "PluginEditor.h"
#include "BinaryData.h"
using Completion = juce::WebBrowserComponent::NativeFunctionCompletion;

OrbitaEditor::OrbitaEditor(OrbitaProcessor& p) : AudioProcessorEditor(p), proc(p) {
  web = std::make_unique<juce::WebBrowserComponent>(makeOptions());
  addAndMakeVisible(*web);
  setResizable(true, true);
  setResizeLimits(900, 600, 2600, 1800);
  setSize(1320, 860);
  web->goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
  startTimerHz(30);
}
OrbitaEditor::~OrbitaEditor() { stopTimer(); }
void OrbitaEditor::resized() { web->setBounds(getLocalBounds()); }

std::optional<juce::WebBrowserComponent::Resource> OrbitaEditor::getResource(const juce::String& url) {
  if (url == "/" || url.startsWith("/index.html")) {
    std::vector<std::byte> data((size_t)BinaryData::index_htmlSize);
    std::memcpy(data.data(), BinaryData::index_html, data.size());
    return juce::WebBrowserComponent::Resource { std::move(data), "text/html; charset=utf-8" };
  }
  return std::nullopt;
}

juce::WebBrowserComponent::Options OrbitaEditor::makeOptions() {
  return juce::WebBrowserComponent::Options {}
    .withNativeIntegrationEnabled()
    .withKeepPageLoadedWhenBrowserIsHidden()
    .withResourceProvider([this](const auto& url) { return getResource(url); })
    .withNativeFunction("msg", [this](const juce::Array<juce::var>& a, Completion done) { if (a.size() > 0) proc.message(juce::JSON::parse(a[0].toString())); done(juce::var()); })
    .withNativeFunction("getState", [this](const juce::Array<juce::var>&, Completion done) { done(proc.stateForPage()); })
    .withNativeFunction("setUi", [this](const juce::Array<juce::var>& a, Completion done) { if (a.size() > 0) proc.setUi(a[0].toString()); done(juce::var()); })
    .withNativeFunction("dragMidi", [this](const juce::Array<juce::var>& a, Completion done) {
      // a[0] file name, a[1] the .mid in base64: written to a temporary folder and dragged out of the plug-in onto a track
      if (a.size() < 2) { done(false); return; }
      juce::MemoryOutputStream mo; juce::Base64::convertFromBase64(mo, a[1].toString());
      auto dir = juce::File::getSpecialLocation(juce::File::tempDirectory).getChildFile("ORBITA MIDI"); dir.createDirectory();
      auto f = dir.getChildFile(juce::File::createLegalFileName(a[0].toString()));
      const bool ok = f.replaceWithData(mo.getData(), mo.getDataSize());
      if (ok) juce::DragAndDropContainer::performExternalDragDropOfFiles({ f.getFullPathName() }, false, this);
      done(ok); })
    .withNativeFunction("openUrl", [](const juce::Array<juce::var>& a, Completion done) {
      // only the donation page, the project's pages and Polaris Sound Lab open outside the plug-in
      auto url = a.size() > 0 ? a[0].toString() : juce::String();
      if (url.startsWith("https://www.paypal.com/donate/") || url.startsWith("https://github.com/drophlex/") || url.startsWith("https://drophlex.github.io/")
          || url.startsWith("https://www.instagram.com/polaris.sound.lab") || url.startsWith("https://www.youtube.com/@PolarisSoundLab"))
        juce::URL(url).launchInDefaultBrowser();
      done(juce::var()); });
}

void OrbitaEditor::timerCallback() {
  proc.snapshot(tele, L, R);
  juce::DynamicObject::Ptr o = new juce::DynamicObject();
  juce::Array<juce::var> t, l, r;
  for (float v : tele) t.add(v);
  for (int k = 0; k < 256; k += 2) { l.add(L[k]); l.add(L[k + 1]); r.add(R[k]); r.add(R[k + 1]); }
  o->setProperty("tele", t); o->setProperty("L", l); o->setProperty("R", r);
  web->emitEventIfBrowserIsVisible("tele", juce::var(o.get()));
  auto ch = proc.takeHostChanges();
  if (!ch.empty()) { juce::Array<juce::var> a; for (auto& c : ch) { juce::Array<juce::var> e; e.add(c.first); e.add(c.second); a.add(e); }
    juce::DynamicObject::Ptr p = new juce::DynamicObject(); p->setProperty("list", a); web->emitEventIfBrowserIsVisible("params", juce::var(p.get())); }
}
