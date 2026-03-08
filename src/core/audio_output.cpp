#include "audio_output.h"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>

// Helper to write little-endian integers
static void writeLE(std::ofstream &out, uint32_t value, int bytes) {
    for (int i = 0; i < bytes; ++i) {
        out.put(static_cast<char>((value >> (8 * i)) & 0xFF));
    }
}

AudioOutput::AudioOutput(AudioPipeline* pipeline, const std::string& filename)
    : pipeline(pipeline), filename(filename), running(false), dataChunkPos(0) {
}

AudioOutput::~AudioOutput() {
    stop();
}

bool AudioOutput::start() {
    if (!pipeline) return false;

    outFile.open(filename, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to open output file " << filename << std::endl;
        return false;
    }

    writeWavHeader();
    running = true;
    worker = std::thread(&AudioOutput::runLoop, this);
    return true;
}

void AudioOutput::stop() {
    if (running) {
        running = false;
        if (worker.joinable()) worker.join();
        finalizeWavHeader();
    }
    if (outFile.is_open()) outFile.close();
}

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
