#include "../src/core/audio_pipeline.h"
#include "../src/core/wavetable.h"
#include "../src/core/wavetable_manager.h"
#include "../src/modules/envelope.h"
#include "../src/modules/filter.h"
#include "../src/modules/oscillator.h"

#include <cmath>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

namespace {
// Small helper for floating-point comparisons in assertions.
bool almostEqual(float a, float b, float eps = 1e-3f) {
    return std::fabs(a - b) <= eps;
}

bool testEnvelopeSustainAndRelease() {
    Envelope env;
    env.setSampleRate(1000.0f);
    env.setAttackTime(0.01f);
    env.setDecayTime(0.01f);
    env.setSustainLevel(0.5f);
    env.setReleaseTime(0.01f);
    env.triggerAttack();

    for (int i = 0; i < 40; ++i) {
        env.process();
    }
    if (env.getCurrentSegment() != EnvelopeSegment::SUSTAIN || !almostEqual(env.getValue(), 0.5f, 0.05f)) {
        std::cerr << "  expected sustain segment/value but got segment="
                  << static_cast<int>(env.getCurrentSegment())
                  << " value=" << env.getValue() << std::endl;
        return false;
    }

    env.triggerRelease();
    for (int i = 0; i < 40; ++i) {
        env.process();
    }

    if (!env.isFinished()) {
        std::cerr << "  envelope did not finish after release window." << std::endl;
        return false;
    }

    return true;
}

bool testEnvelopeParameterClamping() {
    Envelope env;
    env.setAttackTime(0.0f);
    env.setDecayTime(-10.0f);
    env.setReleaseTime(0.0f);
    env.setSustainLevel(2.0f);

    if (!almostEqual(env.getAttackTime(), 0.001f)) return false;
    if (!almostEqual(env.getDecayTime(), 0.001f)) return false;
    if (!almostEqual(env.getReleaseTime(), 0.001f)) return false;
    if (!almostEqual(env.getSustainLevel(), 1.0f)) return false;

    env.setSustainLevel(-1.0f);
    if (!almostEqual(env.getSustainLevel(), 0.0f)) return false;

    return true;
}

bool testOscillatorSampleRateScaling() {
    std::vector<float> waveData(128, 1.0f);
    Wavetable wt("constant", waveData);
    Oscillator osc;
    osc.setWavetable(&wt);
    osc.setFrequency(440.0f);
    osc.setDetune(0.0f);

    osc.reset();
    osc.setSampleRate(44100.0f);
    osc.process();
    float phase44100 = osc.getPhase();

    osc.reset();
    osc.setSampleRate(88200.0f);
    osc.process();
    float phase88200 = osc.getPhase();

    if (!(phase88200 < phase44100)) {
        std::cerr << "  expected higher sample-rate to advance phase more slowly." << std::endl;
        return false;
    }

    return true;
}

bool testWavetableInterpolationAndWrap() {
    std::vector<float> data = {0.0f, 1.0f, 0.0f, -1.0f};
    Wavetable wt("shape", data);

    float mid = wt.getInterpolatedValue(0.5f);
    if (!almostEqual(mid, 0.5f, 1e-4f)) return false;

    float wrapped = wt.getInterpolatedValue(4.25f); // wraps to 0.25
    float expectedWrapped = 0.25f;
    if (!almostEqual(wrapped, expectedWrapped, 1e-4f)) {
        std::cerr << "  wrap interpolation mismatch: got " << wrapped
                  << " expected " << expectedWrapped << std::endl;
        return false;
    }

    return true;
}

bool testWavetableManagerAddRemoveAndGeneration() {
    WavetableManager wm;
    wm.generateSineWave("sine", 64);
    wm.generateSquareWave("square", 64);

    if (!wm.hasWavetable("sine") || !wm.hasWavetable("square")) return false;
    if (wm.getNumWavetables() < 2) return false;

    wm.addWavetable("custom", std::vector<float>{0.0f, 0.1f, 0.2f});
    if (!wm.hasWavetable("custom")) return false;

    wm.removeWavetable("custom");
    if (wm.hasWavetable("custom")) return false;

    return true;
}

bool testPipelineNoteOffMatching() {
    std::vector<float> waveData(128, 1.0f);
    Wavetable wt("constant", waveData);
    WavetableManager wm;
    AudioPipeline pipeline(&wm);
    auto pipelineOsc = std::make_unique<Oscillator>();
    pipelineOsc->setWavetable(&wt);
    auto pipelineEnv = std::make_unique<Envelope>();
    pipelineEnv->setSampleRate(1000.0f);
    pipelineEnv->setAttackTime(0.01f);
    pipelineEnv->setDecayTime(0.01f);
    pipelineEnv->setSustainLevel(0.6f);
    pipelineEnv->setReleaseTime(0.01f);

    Envelope* envPtr = pipelineEnv.get();
    pipeline.addOscillator(std::move(pipelineOsc));
    pipeline.addFilter(std::make_unique<Filter>());
    pipeline.addEnvelope(std::move(pipelineEnv));
    pipeline.connectOscillatorToFilter(0, 0);
    pipeline.connectFilterToEnvelope(0, 0);
    pipeline.connectEnvelopeToOutput(0, nullptr, 0);

    pipeline.noteOn(440.0f, 60);
    pipeline.noteOff(61); // should be ignored
    if (envPtr->getCurrentSegment() == EnvelopeSegment::RELEASE) {
        std::cerr << "  release should not trigger for non-matching note-off." << std::endl;
        return false;
    }

    pipeline.noteOff(60);
    if (envPtr->getCurrentSegment() != EnvelopeSegment::RELEASE) {
        std::cerr << "  release did not trigger for matching note-off." << std::endl;
        return false;
    }

    return true;
}

bool testPipelineLargeBufferProcessing() {
    WavetableManager wm;
    AudioPipeline pipeline(&wm);

    auto osc = std::make_unique<Oscillator>();
    Wavetable* sine = wm.getWavetable("sine");
    if (!sine) {
        std::cerr << "  missing default sine wavetable." << std::endl;
        return false;
    }
    osc->setWavetable(sine);
    osc->setFrequency(220.0f);

    auto filter = std::make_unique<Filter>(FilterType::LOWPASS);
    auto env = std::make_unique<Envelope>();
    env->setAttackTime(0.001f);
    env->setDecayTime(0.001f);
    env->setSustainLevel(1.0f);
    env->setReleaseTime(0.001f);

    pipeline.addOscillator(std::move(osc));
    pipeline.addFilter(std::move(filter));
    pipeline.addEnvelope(std::move(env));
    pipeline.connectOscillatorToFilter(0, 0);
    pipeline.connectFilterToEnvelope(0, 0);
    pipeline.connectEnvelopeToOutput(0, nullptr, 0);
    pipeline.noteOn(220.0f, 57);

    const size_t bigBuffer = 512;
    std::vector<float> output(bigBuffer, 0.0f);
    float* outputs[1] = { output.data() };
    float* inputs[1] = { nullptr };
    pipeline.processBlock(inputs, outputs, 0, 1, bigBuffer);

    bool anyNonZero = false;
    for (float sample : output) {
        if (!std::isfinite(sample)) {
            std::cerr << "  non-finite sample encountered." << std::endl;
            return false;
        }
        if (std::fabs(sample) > 1e-6f) {
            anyNonZero = true;
        }
    }

    if (!anyNonZero) {
        std::cerr << "  expected non-zero output for active note." << std::endl;
        return false;
    }

    return true;
}

bool runTest(const std::string& name, bool (*fn)(), int& passed, int& failed) {
    std::cout << "[ RUN  ] " << name << std::endl;
    bool ok = fn();
    if (ok) {
        ++passed;
        std::cout << "[ PASS ] " << name << std::endl;
    } else {
        ++failed;
        std::cout << "[ FAIL ] " << name << std::endl;
    }
    return ok;
}
}

// Entry point for synth core smoke tests.
int main() {
    std::cout << "=== Synth Core Test Runner ===" << std::endl;
    int passed = 0;
    int failed = 0;

    runTest("Envelope sustain and release stages", testEnvelopeSustainAndRelease, passed, failed);
    runTest("Envelope parameter clamping", testEnvelopeParameterClamping, passed, failed);
    runTest("Oscillator sample-rate scaling", testOscillatorSampleRateScaling, passed, failed);
    runTest("Wavetable interpolation and wrap", testWavetableInterpolationAndWrap, passed, failed);
    runTest("Wavetable manager add/remove/generation", testWavetableManagerAddRemoveAndGeneration, passed, failed);
    runTest("Pipeline note-off matching", testPipelineNoteOffMatching, passed, failed);
    runTest("Pipeline large-buffer processing", testPipelineLargeBufferProcessing, passed, failed);

    std::cout << "=== Test Summary ===" << std::endl;
    std::cout << "Passed: " << passed << std::endl;
    std::cout << "Failed: " << failed << std::endl;
    std::cout << "Total : " << (passed + failed) << std::endl;

    if (failed == 0) {
        std::cout << "Result: SUCCESS" << std::endl;
        return 0;
    }

    std::cout << "Result: FAILURE" << std::endl;
    return 1;
}
