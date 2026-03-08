#include "audio_output.h"
#include <iostream>
#include <cstring>

AudioOutput::AudioOutput(AudioPipeline* pipeline)
    : pipeline(pipeline), stream(nullptr), running(false) {
}

AudioOutput::~AudioOutput() {
    stop();
}

bool AudioOutput::start() {
    if (!pipeline) return false;

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
}

void AudioOutput::stop() {
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
}

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
