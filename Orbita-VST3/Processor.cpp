#include <juce_audio_utils/juce_audio_utils.h>
#include "Engine.h"

class Orbita final : public juce::AudioProcessor {
    using State = juce::AudioProcessorValueTreeState;
    static State::ParameterLayout layout() {
        State::ParameterLayout p;
        const auto id = [](const char* s){return juce::ParameterID(s, 1);};
        p.add(std::make_unique<juce::AudioParameterFloat>(id("ratio"), "Ratio FM", .25f, 8.f, 2.f));
        p.add(std::make_unique<juce::AudioParameterFloat>(id("depth"), "Indice FM", 0.f, 8.f, 1.8f));
        p.add(std::make_unique<juce::AudioParameterFloat>(id("phase"), "Fase", 0.f, 1.f, .16f));
        p.add(std::make_unique<juce::AudioParameterInt>(id("pulses"), "Pulsos / 16", 1, 16, 5));
        p.add(std::make_unique<juce::AudioParameterInt>(id("root"), "Nota raiz MIDI", 24, 84, 48));
        p.add(std::make_unique<juce::AudioParameterBool>(id("auto"), "Secuencia con Live", false));
        p.add(std::make_unique<juce::AudioParameterFloat>(id("gain"), "Salida", 0.f, 1.f, .65f));
        return p;
    }
    State state{*this, nullptr, "Orbita", layout()};
    OrbitaEngine engine;
    juce::SmoothedValue<float> ratio, depth, phase, gain;
    juce::int64 prevStep = -1;
    int gateSamples = 0, autoNote = -1;
    OrbitaEngine sequenceEngine;
    double fallbackPpq = 0;
    float value(const char* id) const {return state.getRawParameterValue(id)->load();}
public:
    Orbita() : juce::AudioProcessor(BusesProperties().withOutput("Stereo", juce::AudioChannelSet::stereo(), true)) {}
    const juce::String getName() const override {return "Polaris Orbita";}
    bool acceptsMidi() const override {return true;}
    bool producesMidi() const override {return false;}
    bool isMidiEffect() const override {return false;}
    double getTailLengthSeconds() const override {return 1.2;}
    int getNumPrograms() override {return 1;}
    int getCurrentProgram() override {return 0;}
    void setCurrentProgram(int) override {}
    const juce::String getProgramName(int) override {return {};}
    void changeProgramName(int,const juce::String&) override {}
    void prepareToPlay(double sr, int) override {
        engine.prepare(sr); sequenceEngine.prepare(sr);
        for(auto* s : {&ratio,&depth,&phase,&gain}) s->reset(sr,.015);
        ratio.setCurrentAndTargetValue(value("ratio")); depth.setCurrentAndTargetValue(value("depth"));
        phase.setCurrentAndTargetValue(value("phase")); gain.setCurrentAndTargetValue(value("gain"));
        prevStep = -1; gateSamples = 0; autoNote = -1; fallbackPpq = 0;
    }
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& b) const override {
        auto out = b.getMainOutputChannelSet();
        return out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo();
    }
    void processBlock(juce::AudioBuffer<float>& out, juce::MidiBuffer& midi) override {
        juce::ScopedNoDenormals guard;
        double ppq = fallbackPpq, bpm = 120;
        bool run = false;
        if(auto* head = getPlayHead()) if(auto pos = head->getPosition()) {
            if(auto q = pos->getPpqPosition()) ppq = *q;
            if(auto t = pos->getBpm()) bpm = juce::jlimit(1.,999.,*t);
            run = pos->getIsPlaying() && value("auto") > .5f;
        }
        const auto increment = bpm / (60 * getSampleRate());
        ratio.setTargetValue(value("ratio")); depth.setTargetValue(value("depth"));
        phase.setTargetValue(value("phase")); gain.setTargetValue(value("gain"));
        const int pulses = int(value("pulses")), root = int(value("root"));
        constexpr int scale[] = {0,3,5,7,10,7,5,3};
        if(!run) {sequenceEngine.lift(-1); prevStep = -1; gateSamples = 0;}
        auto event = midi.begin();
        for(int i=0; i<out.getNumSamples(); ++i) {
            while(event != midi.end() && (*event).samplePosition <= i) {
                const auto m = (*event).getMessage();
                if(m.isNoteOn()) engine.trigger(m.getNoteNumber(),m.getFloatVelocity());
                else if(m.isNoteOff()) engine.lift(m.getNoteNumber());
                else if(m.isAllNotesOff() || m.isAllSoundOff()) engine.lift(-1);
                ++event;
            }
            if(run) {
                const auto step = static_cast<juce::int64>(std::floor((ppq + increment*i)*4));
                if(gateSamples > 0 && --gateSamples == 0) sequenceEngine.lift(autoNote);
                if(step != prevStep && OrbitaEngine::hit(int(step%16),pulses)) {
                    sequenceEngine.lift(-1);
                    const int slot = int((step%16+16)%16);
                    autoNote = root + scale[(slot*pulses/16)%8];
                    sequenceEngine.trigger(autoNote,.82);
                    gateSamples = std::max(1,int(.18/increment));
                }
                prevStep = step;
            }
            auto r=ratio.getNextValue(), d=depth.getNextValue(), p=phase.getNextValue(), g=gain.getNextValue();
            auto sample = (engine.render(r,d,p,g) + sequenceEngine.render(r,d,p,g))*.5f;
            for(int c=0; c<out.getNumChannels(); ++c) out.setSample(c,i,sample);
        }
        if(run) fallbackPpq = ppq + increment*out.getNumSamples();
        midi.clear();
    }
    bool hasEditor() const override {return true;}
    juce::AudioProcessorEditor* createEditor() override;
    void getStateInformation(juce::MemoryBlock& dest) override {
        if(auto xml = state.copyState().createXml()) copyXmlToBinary(*xml,dest);
    }
    void setStateInformation(const void* data,int bytes) override {
        if(auto xml=getXmlFromBinary(data,bytes); xml && xml->hasTagName("Orbita"))
            state.replaceState(juce::ValueTree::fromXml(*xml));
    }
    class Panel : public juce::AudioProcessorEditor, private juce::Timer {
        Orbita& synth;
        juce::ComboBox language;
        juce::Label labels[7];
        juce::Slider knobs[6];
        juce::ToggleButton sequence;
        std::unique_ptr<State::SliderAttachment> links[6];
        std::unique_ptr<State::ButtonAttachment> sequenceLink;
        void localise() {
            const bool en = language.getSelectedId() == 2;
            const char* esNames[] = {"Relacion FM", "Indice FM", "Fase", "Pulsos / 16", "Nota raiz MIDI", "Salida", "Secuencia con Live"};
            const char* enNames[] = {"FM ratio", "FM index", "Phase", "Pulses / 16", "MIDI root note", "Output", "Sequence with Live"};
            for(int i=0;i<7;++i) labels[i].setText(en?enNames[i]:esNames[i],juce::dontSendNotification);
            synth.state.state.setProperty("uiLanguage",en?"en":"es",nullptr);
            repaint();
        }
        void timerCallback() override {repaint(0,0,getWidth(),145);}
    public:
        explicit Panel(Orbita& s) : AudioProcessorEditor(s), synth(s) {
            const char* ids[] = {"ratio","depth","phase","pulses","root","gain"};
            for(int i=0;i<7;++i) addAndMakeVisible(labels[i]);
            for(int i=0;i<6;++i) {
                addAndMakeVisible(knobs[i]);
                knobs[i].setSliderStyle(juce::Slider::LinearHorizontal);
                knobs[i].setTextBoxStyle(juce::Slider::TextBoxRight,false,66,24);
                links[i]=std::make_unique<State::SliderAttachment>(synth.state,ids[i],knobs[i]);
            }
            addAndMakeVisible(sequence);
            sequenceLink=std::make_unique<State::ButtonAttachment>(synth.state,"auto",sequence);
            addAndMakeVisible(language);
            language.addItem(juce::String::fromUTF8("Español"),1); language.addItem("English",2);
            language.setSelectedId(synth.state.state.getProperty("uiLanguage","es").toString()=="en"?2:1,juce::dontSendNotification);
            language.onChange=[this]{localise();};
            localise(); setSize(480,428);
            startTimerHz(12);
        }
        void resized() override {
            language.setBounds(16,106,112,26);
            for(int i=0;i<7;++i) labels[i].setBounds(16,146+i*38,150,32);
            for(int i=0;i<6;++i) knobs[i].setBounds(168,146+i*38,getWidth()-185,32);
            sequence.setBounds(168,146+6*38,42,32);
        }
        void paint(juce::Graphics& g) override {
            g.fillAll(juce::Colour(0xff102221));
            g.setColour(juce::Colour(0xffa5f8cd));
            g.setFont(16.f); g.drawText("POLARIS SOUND LAB",16,14,200,24,juce::Justification::left);
            g.setFont(34.f); g.drawText("ORBITA",16,44,200,42,juce::Justification::left);
            
            juce::Path path;
            for(int i=0;i<=600;i++) {
                auto t = i * juce::MathConstants<double>::twoPi/600.;
                auto x = float(341 + 104*std::sin(t));
                auto y = float(72 + 54*std::sin(t*synth.value("ratio") + synth.value("phase")*6.2831853 + synth.value("depth")*.22*std::sin(t)));
                if(i==0) path.startNewSubPath(x,y); else path.lineTo(x,y);
            }
            g.strokePath(path,juce::PathStrokeType(1.4f));
        }
    };
};
juce::AudioProcessorEditor* Orbita::createEditor(){return new Panel(*this);}
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter(){return new Orbita();}
