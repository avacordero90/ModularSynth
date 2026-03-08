#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include "audio_pipeline.h"
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
    bool start();
    void stop();

    bool isRunning() const { return running.load(); }

private:
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
