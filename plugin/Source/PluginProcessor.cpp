#include "PluginProcessor.h"
#include "PluginEditor.h"
using namespace orbita;

OrbitaProcessor::OrbitaProcessor()
  : AudioProcessor(BusesProperties().withOutput("Output", juce::AudioChannelSet::stereo(), true)) {
  eng = std::make_unique<Engine>(); eng->init(48000);
  for (auto& h : hostOf) h = -1;
  // what the DAW can automate (the page shows everything)
  struct D { const char* id; const char* name; int e; bool stepped; };
  std::vector<D> list = {
    {"gravity", "Gravity", G_GRAVITY, false}, {"density", "Density", G_DENSITY, true}, {"phase", "Phase", G_PHASE, true}, {"ratio", "FM ratio", G_RATIO, false},
    {"evolve", "Evolve", G_EVOLVE, false}, {"chaos", "Chaos", G_CHAOS, false}, {"bloom", "Bloom", G_BLOOM, false}, {"space", "Space", G_SPACE, false},
    {"echo", "Echo", G_ECHO, false}, {"warmth", "Warmth", G_WARMTH, false}, {"echo_fb", "Echo feedback", G_ECHOFB, false}, {"size", "Space size", G_SIZE, false},
    {"chorus", "Chorus", G_CHORUS, false}, {"width", "Width", G_WIDTH, false}, {"swing", "Swing", G_SWING, false}, {"human", "Human", G_HUMAN, false},
    {"tilt", "Tilt", G_TILT, false}, {"master", "Master", G_MASTER, false}, {"root", "Root", G_ROOT, true}, {"scale", "Scale", G_SCALE, true},
    {"geometry", "Geometry", G_GEOM, true}, {"keymode", "Keys mode", G_KEYMODE, true}, {"tempo", "Own tempo", G_BPM, false} };
  static const char* ln[NL] = {"Sub", "Prism", "Crystal", "Halo"};
  std::vector<std::string> keep;   // the ids and names (copied by the parameters)
  for (int l = 0; l < NL; l++) {
    const struct { const char* k; const char* n; int p; bool s; } lp[] = {
      {"on", "On", L_ON, true}, {"level", "Level", L_LEVEL, false}, {"pulses", "Pulses", L_PULSES, true}, {"steps", "Steps", L_STEPS, true}, {"rotate", "Rotate", L_ROT, true},
      {"index", "FM index", L_INDEX, false}, {"cutoff", "Cutoff", L_CUT, false}, {"ratio", "Ratio", L_RATIO, false}, {"echo", "Echo send", L_ECHO, false}, {"space", "Space send", L_SPACE, false} };
    for (auto& x : lp) { keep.push_back("l" + std::to_string(l + 1) + "_" + x.k); keep.push_back(std::string(ln[l]) + " " + x.n); }
  }
  size_t ki = 0;
  for (int l = 0; l < NL; l++) {
    const int ps[] = {L_ON, L_LEVEL, L_PULSES, L_STEPS, L_ROT, L_INDEX, L_CUT, L_RATIO, L_ECHO, L_SPACE};
    for (int p : ps) { const bool st = p == L_ON || p == L_PULSES || p == L_STEPS || p == L_ROT; list.push_back({keep[ki].c_str(), keep[ki + 1].c_str(), LPID(l, p), st}); ki += 2; }
  }
  lastHost.reset(new std::atomic<float>[list.size()]); changed.reset(new std::atomic<uint8_t>[list.size()]);
  for (size_t i = 0; i < list.size(); i++) {
    const auto& d = list[i]; Range r = range(d.e);
    hp.push_back({d.e, r.lo, r.hi, d.stepped});
    juce::RangedAudioParameter* p;
    const float def = eng->params[d.e];   // the engine's own default (each voice has its own)
    if (d.stepped) p = new juce::AudioParameterInt(juce::ParameterID{d.id, 1}, d.name, (int)r.lo, (int)r.hi, (int)std::round(def));
    else p = new juce::AudioParameterFloat(juce::ParameterID{d.id, 1}, d.name, juce::NormalisableRange<float>(r.lo, r.hi), def);
    addParameter(p); params.push_back(p);
    lastHost[i] = p->getValue(); changed[i] = 0; hostOf[d.e] = (int)i;
  }
  std::copy(eng->params, eng->params + P_COUNT, shP); std::copy(eng->params, eng->params + P_COUNT, shT);
}

// the message thread's copy of the project (what the page and the saved project read; the engine gets the same through the queue)
void OrbitaProcessor::shadowParam(int id, float v) {
  if (id < 0 || id >= P_COUNT) return; Range r = range(id); shP[id] = juce::jlimit(r.lo, r.hi, v);
  if (id == G_DENSITY || id == G_PHASE) for (auto& c : shC) c = -1;
  if (id >= G_NG) { const int k = (id - G_NG) % L_NP; if (k == L_PULSES || k == L_STEPS || k == L_ROT) shC[(id - G_NG) / L_NP] = -1; }
}

void OrbitaProcessor::prepareToPlay(double rate, int) {
  if (std::abs(rate - sr) < 1 && eng->sr == (float)rate) return;
  // a new rate: start the engine again at that rate, with the same sound and steps
  auto keep = std::make_unique<Engine>(*eng);
  eng->init((float)rate); sr = rate;
  std::copy(keep->params, keep->params + P_COUNT, eng->params); std::copy(keep->target, keep->target + P_COUNT, eng->target); eng->hasTarget = keep->hasTarget;
  for (int l = 0; l < NL; l++) { eng->custom[l] = keep->custom[l]; eng->useCustom[l] = keep->useCustom[l]; }
  eng->computeEff();
}

void OrbitaProcessor::drain() {
  int s1, n1, s2, n2; fifo.prepareToRead(fifo.getNumReady(), s1, n1, s2, n2);
  auto run = [&](const Cmd& c) {
    switch (c.t) {
      case C_PARAM: eng->setParam(c.a, c.v); if (hostOf[c.a] >= 0) lastHost[(size_t)hostOf[c.a]] = norm(hostOf[c.a], eng->params[c.a]); break;
      case C_TARGET: eng->setTarget(c.a, c.v); break;
      case C_CLEARTARGET: eng->clearTarget(); break;
      case C_PLAY: eng->play(); break;
      case C_STOP: eng->stop(); break;
      case C_KEY: if (c.b > 0) eng->keyDown(c.a, c.b); else eng->keyUp(c.a); break;
      case C_CUSTOM: eng->setCustom(c.a, (uint16_t)c.b); break;
      case C_AUDITION: eng->noteOn(c.a, c.b, c.v, (int)(eng->stepLen() * 2)); break;
      case C_EVO: eng->evoPhase = c.a; eng->evoDen = c.b; eng->evoRatio = c.v; break;
    }
  };
  for (int i = 0; i < n1; i++) run(q[s1 + i]);
  for (int i = 0; i < n2; i++) run(q[s2 + i]);
  fifo.finishedRead(n1 + n2);
}

void OrbitaProcessor::processBlock(juce::AudioBuffer<float>& buf, juce::MidiBuffer& midi) {
  juce::ScopedNoDenormals nd;
  drain();
  // the DAW's automation -> the engine
  for (size_t i = 0; i < hp.size(); i++) {
    const float v = params[i]->getValue();
    if (std::abs(v - lastHost[i].load()) > 1e-6f) {
      const auto& h = hp[i]; float x = h.lo + (h.hi - h.lo) * v; if (h.stepped) x = std::round(x);
      eng->setParam(h.eng, x); lastHost[i] = v; changed[i] = 1;
    }
  }
  // the DAW's transport
  bool hostPlay = false; double bpm = 120, ppq = 0;
  if (auto* ph = getPlayHead()) if (auto pos = ph->getPosition()) {
    hostPlay = pos->getIsPlaying(); if (auto b = pos->getBpm()) bpm = *b; if (auto p = pos->getPpqPosition()) ppq = *p;
  }
  eng->host(hostPlay, (float)bpm, ppq, sync.load());

  // MIDI in, sample accurate: the engine runs in pieces between the events
  juce::MidiBuffer out;
  const int n = buf.getNumSamples(); float* L = buf.getWritePointer(0); float* R = buf.getNumChannels() > 1 ? buf.getWritePointer(1) : nullptr;
  float tl[128], tr[128];
  auto it = midi.cbegin(); int pos = 0;
  const bool mo = midiOut.load();
  auto runTo = [&](int end) {
    while (pos < end) {
      const int len = std::min(128, end - pos);
      eng->nev = 0; eng->process(tl, tr, len);
      std::memcpy(L + pos, tl, sizeof(float) * (size_t)len); if (R) std::memcpy(R + pos, tr, sizeof(float) * (size_t)len);
      if (mo) for (int k = 0; k < eng->nev; k++) { const auto& e = eng->ev[k]; const int at = pos + juce::jlimit(0, len - 1, e.at);
        out.addEvent(e.kind ? juce::MidiMessage::noteOn(e.lane + 1, e.note, (juce::uint8)juce::jmax(1, (int)e.vel)) : juce::MidiMessage::noteOff(e.lane + 1, e.note), at); }
      pos += len;
    }
  };
  for (; it != midi.cend(); ++it) {
    const auto m = (*it).getMessage(); const int at = juce::jlimit(0, n, (*it).samplePosition);
    runTo(at);
    if (m.isNoteOn()) eng->keyDown(m.getNoteNumber(), m.getVelocity());
    else if (m.isNoteOff()) eng->keyUp(m.getNoteNumber());
    else if (m.isAllNotesOff() || m.isAllSoundOff()) { eng->allOff(); eng->held = 0; eng->nHeld = 0; eng->keyRoot = -1; }
  }
  runTo(n);
  for (int c = 2; c < buf.getNumChannels(); c++) buf.clear(c, 0, n);
  midi.swapWith(out);
  { const juce::SpinLock::ScopedTryLockType tl2(teleLock); if (tl2.isLocked()) { std::memcpy(tele, eng->tele, sizeof(tele));
      for (int k = 0; k < 256; k++) { const int j = (eng->scopeW - 256 + k + 512) & 511; scL[k] = eng->scopeL[j]; scR[k] = eng->scopeR[j]; } } }
}

void OrbitaProcessor::snapshot(float* t, float* l, float* r) { const juce::SpinLock::ScopedLockType tl(teleLock); std::memcpy(t, tele, sizeof(tele)); std::memcpy(l, scL, sizeof(scL)); std::memcpy(r, scR, sizeof(scR)); }

std::vector<std::pair<int, float>> OrbitaProcessor::takeHostChanges() {
  std::vector<std::pair<int, float>> v;
  for (size_t i = 0; i < hp.size(); i++) if (changed[i].exchange(0)) { const auto& h = hp[i]; float x = h.lo + (h.hi - h.lo) * lastHost[i].load(); if (h.stepped) x = std::round(x); v.push_back({h.eng, x}); shadowParam(h.eng, x); }
  return v;
}

void OrbitaProcessor::setHost(int e, float v) {
  const int i = e >= 0 && e < P_COUNT ? hostOf[e] : -1; if (i < 0) return;
  Range r = range(e); const float x = juce::jlimit(r.lo, r.hi, v), nv = norm(i, x);
  if (std::abs(params[(size_t)i]->getValue() - nv) < 1e-6f) return;
  lastHost[(size_t)i] = nv;                        // so the audio thread doesn't read it back as automation
  params[(size_t)i]->setValueNotifyingHost(nv);
}

// the page's messages (the same shapes the web worklet gets)
void OrbitaProcessor::message(const juce::var& m) {
  const auto t = m["t"].toString();
  if (t == "param") { const int id = m["id"]; const float v = (float)(double)m["v"]; push({C_PARAM, id, 0, v}); setHost(id, v); shadowParam(id, v); }
  else if (t == "params") { if (auto* a = m["list"].getArray()) for (auto& x : *a) { const int id = x[0]; const float v = (float)(double)x[1]; push({C_PARAM, id, 0, v}); setHost(id, v); shadowParam(id, v); } }
  else if (t == "target") { if (auto* a = m["list"].getArray()) for (auto& x : *a) { const int id = x[0]; const float v = (float)(double)x[1]; push({C_TARGET, id, 0, v}); if (id >= 0 && id < P_COUNT) shT[id] = v; } shHasT = true; }
  else if (t == "clearTarget") { push({C_CLEARTARGET, 0, 0, 0}); shHasT = false; }
  else if (t == "play") push({C_PLAY, 0, 0, 0});
  else if (t == "stop") push({C_STOP, 0, 0, 0});
  else if (t == "key") push({C_KEY, (int)m["note"], (int)m["vel"], 0});
  else if (t == "custom") { const int l = m["lane"]; push({C_CUSTOM, l, (int)m["mask"], 0}); if (l >= 0 && l < NL) shC[l] = (int)m["mask"]; }
  else if (t == "audition") push({C_AUDITION, (int)m["lane"], (int)m["note"], (float)(int)m["vel"] / 127.f});
  else if (t == "evo") push({C_EVO, (int)m["p"], (int)m["d"], (float)(double)m["r"]});
  else if (t == "sync") sync = (bool)m["on"];
  else if (t == "midiOut") midiOut = (bool)m["on"];
}

juce::var OrbitaProcessor::stateForPage() {
  juce::DynamicObject::Ptr o = new juce::DynamicObject();
  juce::Array<juce::var> p; for (int i = 0; i < P_COUNT; i++) p.add(shP[i]);
  o->setProperty("params", p); o->setProperty("sync", sync.load()); o->setProperty("midiOut", midiOut.load()); o->setProperty("playing", eng->playing);
  { const juce::SpinLock::ScopedLockType l(uiLock); o->setProperty("ui", uiJson); }
  return juce::var(o.get());
}

// the project: every engine value, GRAVITY's target, the steps you clicked, the page's own state
void OrbitaProcessor::getStateInformation(juce::MemoryBlock& dest) {
  juce::DynamicObject::Ptr o = new juce::DynamicObject();
  juce::Array<juce::var> p, tg, cu;
  for (int i = 0; i < P_COUNT; i++) { p.add(shP[i]); tg.add(shT[i]); }
  for (int l = 0; l < NL; l++) cu.add(shC[l]);
  o->setProperty("format", "orbita2"); o->setProperty("params", p); o->setProperty("target", tg); o->setProperty("hasTarget", shHasT);
  o->setProperty("custom", cu); o->setProperty("sync", sync.load()); o->setProperty("midiOut", midiOut.load());
  { const juce::SpinLock::ScopedLockType l(uiLock); o->setProperty("ui", uiJson); }
  const auto s = juce::JSON::toString(juce::var(o.get()), true);
  dest.replaceAll(s.toRawUTF8(), s.getNumBytesAsUTF8());
}

void OrbitaProcessor::setStateInformation(const void* data, int size) {
  const auto v = juce::JSON::parse(juce::String::fromUTF8((const char*)data, size));
  if (v["format"].toString() != "orbita2") return;                // ÓRBITA 1 projects: the new instrument starts from its first scene
  if (auto* a = v["params"].getArray()) for (int i = 0; i < juce::jmin(P_COUNT, a->size()); i++) { const float x = (float)(double)(*a)[i]; push({C_PARAM, i, 0, x}); setHost(i, x); shadowParam(i, x); }
  shHasT = (bool)v["hasTarget"];
  if (shHasT) if (auto* a = v["target"].getArray()) for (int i = 0; i < juce::jmin(P_COUNT, a->size()); i++) { shT[i] = (float)(double)(*a)[i]; push({C_TARGET, i, 0, shT[i]}); }
  if (auto* a = v["custom"].getArray()) for (int l = 0; l < juce::jmin(NL, a->size()); l++) { shC[l] = (int)(*a)[l]; if (shC[l] >= 0) push({C_CUSTOM, l, shC[l], 0}); }
  sync = (bool)v["sync"]; midiOut = (bool)v["midiOut"];
  setUi(v["ui"].toString());
}

juce::AudioProcessorEditor* OrbitaProcessor::createEditor() { return new OrbitaEditor(*this); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new OrbitaProcessor(); }
