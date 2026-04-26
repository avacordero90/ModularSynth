#include "audio_output.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <algorithm>

// Helper to write little-endian integers.
static void writeLE(std::ofstream &out, uint32_t value, int bytes) {
    for (int i = 0; i < bytes; ++i) {
        out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
    }
}

// Construct audio output backend around the active pipeline.
AudioOutput::AudioOutput(AudioPipeline* pipeline, const std::string& filename)
    : pipeline(pipeline), filename(filename), running(false), dataChunkPos(0) {
    // nothing else to do; portaudio stream created on start()
}

// Ensure output stream/thread is stopped on destruction.
AudioOutput::~AudioOutput() {
    stop();
}

// Start real-time PortAudio stream or file-rendering fallback.
bool AudioOutput::start() {
    if (!pipeline) return false;

#ifdef USE_PORTAUDIO
    // initialize PortAudio and open stream
    PaError err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return false;
    }

    PaStreamParameters outputParameters;
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
        std::cerr << "No default output device found" << std::endl;
        Pa_Terminate();
        return false;
    }

    outputParameters.channelCount = 2; // stereo
    outputParameters.sampleFormat = paFloat32;
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = nullptr;

    err = Pa_OpenStream(&stream, nullptr, &outputParameters,
                       pipeline->getSampleRate(), pipeline->getBufferSize(),
                       paClipOff, audioCallback, this);
    if (err != paNoError) {
        std::cerr << "Failed to open PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return false;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "Failed to start PortAudio stream: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return false;
    }

    running = true;
    std::cout << "Real-time audio output started" << std::endl;
    return true;
#else
    // fallback to WAV file rendering
    outFile.open(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file " << filename << std::endl;
        return false;
    }

    writeWavHeader();
    running = true;
    worker = std::thread(&AudioOutput::runLoop, this);
    return true;
#endif
}

// Stop active output backend and release associated resources.
void AudioOutput::stop() {
#ifdef USE_PORTAUDIO
    if (running) {
        running = false;
        if (stream) {
            Pa_StopStream(stream);
            Pa_CloseStream(stream);
            stream = nullptr;
        }
        Pa_Terminate();
        std::cout << "Real-time audio output stopped" << std::endl;
    }
#else
    if (running) {
        running = false;
        if (worker.joinable()) worker.join();
        finalizeWavHeader();
    }
    if (outFile.is_open()) outFile.close();
#endif
}

// Write placeholder WAV header before streaming PCM samples.
void AudioOutput::writeWavHeader() {
    // simple 16-bit PCM header placeholder
    // we'll fill chunk sizes later
    outFile.write("RIFF", 4);
    writeLE(outFile, 0, 4); // placeholder for file size
    outFile.write("WAVE", 4);
    outFile.write("fmt ", 4);
    writeLE(outFile, 16, 4); // fmt chunk size
    writeLE(outFile, 1, 2);  // PCM
    writeLE(outFile, 2, 2);  // stereo
    writeLE(outFile, static_cast<uint32_t>(pipeline->getSampleRate()), 4);
    writeLE(outFile, static_cast<uint32_t>(pipeline->getSampleRate() * 4), 4); // byte rate
    writeLE(outFile, 4, 2);  // block align
    writeLE(outFile, 16, 2); // bits per sample
    outFile.write("data", 4);
    dataChunkPos = static_cast<uint32_t>(outFile.tellp());
    writeLE(outFile, 0, 4); // placeholder for data size
}

// Patch WAV header chunk sizes after rendering is complete.
void AudioOutput::finalizeWavHeader() {
    if (!outFile) return;
    uint32_t fileSize = static_cast<uint32_t>(outFile.tellp());
    uint32_t dataSize = fileSize - dataChunkPos - 4;
    // update RIFF chunk size
    outFile.seekp(4, std::ios::beg);
    writeLE(outFile, fileSize - 8, 4);
    // update data chunk size
    outFile.seekp(dataChunkPos, std::ios::beg);
    writeLE(outFile, dataSize, 4);
}

#ifdef USE_PORTAUDIO
// PortAudio callback bridge into pipeline processing.
int AudioOutput::audioCallback(const void* inputBuffer, void* outputBuffer,
                              unsigned long framesPerBuffer,
                              const PaStreamCallbackTimeInfo* timeInfo,
                              PaStreamCallbackFlags statusFlags,
                              void* userData) {
    AudioOutput* self = static_cast<AudioOutput*>(userData);
    float* out = static_cast<float*>(outputBuffer);

    // Prepare input/output pointers for pipeline
    float* inPtrs[1] = { nullptr }; // no input
    float* outPtrs[1] = { out };    // mono output

    // Process audio through pipeline (mono)
    self->pipeline->processBlock(inPtrs, outPtrs, 0, 1, framesPerBuffer);

    // Duplicate mono to stereo
    for (unsigned long i = 0; i < framesPerBuffer; ++i) {
        out[i * 2 + 1] = out[i * 2]; // right = left
    }

    return self->running ? paContinue : paComplete;
}
#else
// File-render loop used when PortAudio is not enabled.
void AudioOutput::runLoop() {
    const size_t bufferSize = pipeline->getBufferSize();
    std::vector<float> mono(bufferSize);
    float* outPtrs[1] = { mono.data() };
    float* inPtrs[1] = { nullptr };

    while (running) {
        pipeline->processBlock(inPtrs, outPtrs, 0, 1, bufferSize);
        // convert float [-1,1] to 16-bit PCM
        for (size_t i = 0; i < bufferSize && running; ++i) {
            int16_t sample = static_cast<int16_t>(std::max(-1.0f, std::min(1.0f, mono[i])) * 32767);
            writeLE(outFile, static_cast<uint16_t>(sample), 2);
            // write same sample for right channel
            writeLE(outFile, static_cast<uint16_t>(sample), 2);
        }
        // small sleep to avoid busy loop; simulate real-time
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}
#endif