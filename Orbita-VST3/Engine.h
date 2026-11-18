#pragma once
#include <array>
#include <algorithm>
#include <cmath>

class OrbitaEngine {
    static constexpr double tau = 6.283185307179586;
    struct Voice {
        int note = -1;
        bool released = false;
        double angle = 0, modAngle = 0, amplitude = 0, velocity = 0, hz = 0;
    };
    std::array<Voice, 12> voices{};
    double sampleRate = 48000, attack = 0, release = 0;
public:
    void prepare(double sr) {
        sampleRate = sr;
        attack = 1 - std::exp(-1 / (sr * .008));
        release = std::exp(-1 / (sr * .11));
        voices = {};
    }
    void trigger(int note, double velocity) {
        auto* choice = &voices.front();
        for (auto& voice : voices) {
            if (voice.note < 0) { choice = &voice; break; }
            if (voice.amplitude < choice->amplitude) choice = &voice;
        }
        *choice = {note, false, 0, 0, 0, velocity, 440 * std::exp2((note - 69) / 12.0)};
    }
    void lift(int note) {
        for (auto& v : voices) if (v.note == note || note < 0) v.released = true;
    }
    static bool hit(int step, int pulses) {
        return ((step % 16 + 16) % 16 * pulses) % 16 < pulses;
    }
    float render(double ratio, double index, double phase, double gain) {
        double sum = 0;
        for (auto& v : voices) {
            if (v.note < 0) continue;
            v.amplitude = v.released ? v.amplitude * release : v.amplitude + (1 - v.amplitude) * attack;
            if (v.released && v.amplitude < .00005) { v.note = -1; continue; }
            const double step = tau * v.hz / sampleRate;
            sum += std::sin(v.angle + index * std::sin(v.modAngle + phase * tau)) * v.amplitude * v.velocity;
            v.angle = std::fmod(v.angle + step, tau);
            v.modAngle = std::fmod(v.modAngle + step * ratio, tau);
        }
        return float(std::tanh(sum * .32) * gain);
    }
};
