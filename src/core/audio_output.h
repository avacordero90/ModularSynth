#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include "audio_pipeline.h"
<<<<<<< HEAD
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
=======
#include <thread>
#include <fstream>
#include <atomic>
#include <vector>

// Simple audio output that writes pipeline output to a WAV file.  This
// allows the synthesizer to produce audible results without requiring a
// MIDI device or any external audio library.  The file is generated in the
// working directory as "output.wav" by default.

class AudioOutput {
public:
    AudioOutput(AudioPipeline* pipeline, const std::string& filename = "output.wav");
    ~AudioOutput();

    // Start generating audio to file.  Returns true if the file was
    // successfully opened and the background thread launched.
>>>>>>> 5ffc43519522ce0e9298fdc8c58162ba0a4e9e39
    bool start();
    void stop();

    bool isRunning() const { return running.load(); }

private:
<<<<<<< HEAD
    static int audioCallback(const void* inputBuffer, void* outputBuffer,
                           unsigned long framesPerBuffer,
                           const PaStreamCallbackTimeInfo* timeInfo,
                           PaStreamCallbackFlags statusFlags,
                           void* userData);

    AudioPipeline* pipeline;
    PaStream* stream;
    std::atomic<bool> running;
=======
    void runLoop();
    void writeWavHeader();
    void finalizeWavHeader();

    AudioPipeline* pipeline;
    std::string filename;
    std::ofstream outFile;
    std::thread worker;
    std::atomic<bool> running;
    uint32_t dataChunkPos;
>>>>>>> 5ffc43519522ce0e9298fdc8c58162ba0a4e9e39
};

#endif // AUDIO_OUTPUT_H
