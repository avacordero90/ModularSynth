#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include "audio_pipeline.h"
#include <portaudio.h>
#include <thread>
#include <atomic>
#include <vector>

// Real-time audio output using PortAudio for audible playback.
// Processes the audio pipeline and streams output to the system's audio device.

class AudioOutput {
public:
    AudioOutput(AudioPipeline* pipeline);
    ~AudioOutput();

    // Start real-time audio output. Returns true if PortAudio stream opened successfully.
    bool start();
    void stop();

    bool isRunning() const { return running.load(); }

private:
    static int audioCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);

    AudioPipeline* pipeline;
    PaStream* stream;
    std::atomic<bool> running;
};

#endif // AUDIO_OUTPUT_H
