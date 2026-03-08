#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include "audio_pipeline.h"
#include <cstdint>
#include <thread>
#include <fstream>
#include <atomic>
#include <vector>

#ifdef USE_PORTAUDIO
#include <portaudio.h>
#endif

// AudioOutput provides two modes of operation:
// * real-time playback via PortAudio when compiled with USE_PORTAUDIO
// * file-based WAV rendering otherwise (default)
// The GUI will create the object without needing to know which mode is
// active; start()/stop() behave accordingly.

class AudioOutput {
public:
    // If running in WAV mode, the optional filename specifies where to
    // write audio data (default "output.wav").  In PortAudio mode the
    // argument is ignored.
    AudioOutput(AudioPipeline* pipeline, const std::string& filename = "output.wav");
    ~AudioOutput();

    bool start();
    void stop();

    bool isRunning() const { return running.load(); }

private:
#ifdef USE_PORTAUDIO
    static int audioCallback(const void* inputBuffer, void* outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void* userData);

    PaStream* stream = nullptr;
#endif

    // WAV file helpers (unused if USE_PORTAUDIO)
    void runLoop();
    void writeWavHeader();
    void finalizeWavHeader();

    AudioPipeline* pipeline;
    std::string filename;
    std::ofstream outFile;
    std::thread worker;
    std::atomic<bool> running;
    uint32_t dataChunkPos;
};

#endif // AUDIO_OUTPUT_H
