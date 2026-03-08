#include "synth_component.h"
#include "../core/audio_output.h"
#include <cstdio>
#include <iostream>

SynthComponent::SynthComponent()
    : main_window(nullptr),
      wavetableManager(nullptr),
      pipeline(nullptr) {
    
    // Create main window
    main_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(main_window), "Modular Wavetable Synthesizer");
    gtk_window_set_default_size(GTK_WINDOW(main_window), 600, 800);
    gtk_container_set_border_width(GTK_CONTAINER(main_window), 10);
    
    // Create main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_add(GTK_CONTAINER(main_window), vbox);
    
    // Create notebook for tabbed interface
    notebook = gtk_notebook_new();
    gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);
    
    // Setup sections
    setupOscillatorSection();
    setupFilterSection();
    setupEnvelopeSection();
    
    // Create status text view
    GtkWidget *status_frame = gtk_frame_new("Status");
    gtk_box_pack_start(GTK_BOX(vbox), status_frame, FALSE, FALSE, 0);
    
    status_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(status_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(status_text), GTK_WRAP_WORD);
    gtk_container_add(GTK_CONTAINER(status_frame), status_text);
    
    gtk_widget_show_all(main_window);
    
    g_signal_connect(main_window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
}

SynthComponent::~SynthComponent() {
    cleanupSynth();
}

void SynthComponent::setupOscillatorSection() {
    // Create oscillator frame
    GtkWidget *osc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(osc_box), 10);
    
    // Frequency slider
    freq_label = gtk_label_new("Frequency (Hz): 440");
    gtk_box_pack_start(GTK_BOX(osc_box), freq_label, FALSE, FALSE, 0);
    
    freq_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20.0, 20000.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(freq_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(freq_scale), 440.0);
    gtk_box_pack_start(GTK_BOX(osc_box), freq_scale, FALSE, FALSE, 0);
    
    // Detune slider
    detune_label = gtk_label_new("Detune (cents): 0");
    gtk_box_pack_start(GTK_BOX(osc_box), detune_label, FALSE, FALSE, 0);
    
    detune_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1200.0, 1200.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(detune_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(detune_scale), 0.0);
    gtk_box_pack_start(GTK_BOX(osc_box), detune_scale, FALSE, FALSE, 0);
    
    // Waveform combo
    waveform_label = gtk_label_new("Waveform:");
    gtk_box_pack_start(GTK_BOX(osc_box), waveform_label, FALSE, FALSE, 0);
    
    waveform_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Sine");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Square");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Sawtooth");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Triangle");
    gtk_combo_box_set_active(GTK_COMBO_BOX(waveform_combo), 0);
    gtk_box_pack_start(GTK_BOX(osc_box), waveform_combo, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), osc_box, gtk_label_new("Oscillator"));
}

void SynthComponent::setupFilterSection() {
    GtkWidget *filter_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(filter_box), 10);
    
    // Cutoff frequency slider
    cutoff_label = gtk_label_new("Cutoff (Hz): 1000");
    gtk_box_pack_start(GTK_BOX(filter_box), cutoff_label, FALSE, FALSE, 0);
    
    cutoff_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20.0, 20000.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(cutoff_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(cutoff_scale), 1000.0);
    gtk_box_pack_start(GTK_BOX(filter_box), cutoff_scale, FALSE, FALSE, 0);
    
    // Resonance slider
    res_label = gtk_label_new("Resonance: 0.71");
    gtk_box_pack_start(GTK_BOX(filter_box), res_label, FALSE, FALSE, 0);
    
    res_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.1, 10.0, 0.1);
    gtk_scale_set_value_pos(GTK_SCALE(res_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(res_scale), 0.707);
    gtk_box_pack_start(GTK_BOX(filter_box), res_scale, FALSE, FALSE, 0);
    
    // Filter type combo
    filter_type_label = gtk_label_new("Filter Type:");
    gtk_box_pack_start(GTK_BOX(filter_box), filter_type_label, FALSE, FALSE, 0);
    
    filter_type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_type_combo), NULL, "Lowpass");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_type_combo), NULL, "Highpass");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_type_combo), NULL, "Bandpass");
    gtk_combo_box_set_active(GTK_COMBO_BOX(filter_type_combo), 0);
    gtk_box_pack_start(GTK_BOX(filter_box), filter_type_combo, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), filter_box, gtk_label_new("Filter"));
}

void SynthComponent::setupEnvelopeSection() {
    GtkWidget *env_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(env_box), 10);
    
    // Attack slider
    attack_label = gtk_label_new("Attack (s): 0.01");
    gtk_box_pack_start(GTK_BOX(env_box), attack_label, FALSE, FALSE, 0);
    
    attack_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.001, 5.0, 0.001);
    gtk_scale_set_value_pos(GTK_SCALE(attack_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(attack_scale), 0.01);
    gtk_box_pack_start(GTK_BOX(env_box), attack_scale, FALSE, FALSE, 0);
    
    // Decay slider
    decay_label = gtk_label_new("Decay (s): 0.1");
    gtk_box_pack_start(GTK_BOX(env_box), decay_label, FALSE, FALSE, 0);
    
    decay_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.001, 5.0, 0.001);
    gtk_scale_set_value_pos(GTK_SCALE(decay_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(decay_scale), 0.1);
    gtk_box_pack_start(GTK_BOX(env_box), decay_scale, FALSE, FALSE, 0);
    
    // Sustain slider
    sustain_label = gtk_label_new("Sustain: 0.7");
    gtk_box_pack_start(GTK_BOX(env_box), sustain_label, FALSE, FALSE, 0);
    
    sustain_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.01);
    gtk_scale_set_value_pos(GTK_SCALE(sustain_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(sustain_scale), 0.7);
    gtk_box_pack_start(GTK_BOX(env_box), sustain_scale, FALSE, FALSE, 0);
    
    // Release slider
    release_label = gtk_label_new("Release (s): 0.3");
    gtk_box_pack_start(GTK_BOX(env_box), release_label, FALSE, FALSE, 0);
    
    release_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.001, 5.0, 0.001);
    gtk_scale_set_value_pos(GTK_SCALE(release_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(release_scale), 0.3);
    gtk_box_pack_start(GTK_BOX(env_box), release_scale, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), env_box, gtk_label_new("Envelope"));
}

void SynthComponent::initializeSynth() {
    try {
        // Create and initialize wavetable manager
        wavetableManager = new WavetableManager();
        
        // Generate standard wavetables with larger size for better quality
        wavetableManager->generateSineWave("sine", 2048);
        wavetableManager->generateSquareWave("square", 2048);
        wavetableManager->generateSawtoothWave("sawtooth", 2048);
        wavetableManager->generateTriangleWave("triangle", 2048);
        
        // Create audio pipeline with 44.1kHz sample rate
        pipeline = new AudioPipeline(wavetableManager, 44100.0f);
        
        // Create and configure modules
        oscillator = std::make_unique<Oscillator>();
        filter = std::make_unique<Filter>(FilterType::LOWPASS);
        envelope = std::make_unique<Envelope>();
        
        // Set initial oscillator configuration with sine wavetable
        Wavetable* sineWave = wavetableManager->getWavetable("sine");
        if (sineWave) {
            oscillator->setWavetable(sineWave);
            oscillator->setWavetableName("sine");
            oscillator->setFrequency(440.0f);
        }
        
        // Add modules to pipeline
        pipeline->addOscillator(std::move(oscillator));
        pipeline->addFilter(std::move(filter));
        pipeline->addEnvelope(std::move(envelope));
        
        // Configure oscillators with wavetables
        pipeline->configureOscillatorsWithWavetables();

        // begin audio output automatically (no MIDI needed)
        audioOutput = new AudioOutput(pipeline);
        if (audioOutput) {
            bool started = audioOutput->start();
            if (!started) {
                std::cerr << "Warning: unable to start audio output" << std::endl;
            }
        }
        
        // Update status with initialization details
        char statusMsg[512];
        snprintf(statusMsg, sizeof(statusMsg), 
                 "Synthesizer initialized successfully!\\n"
                 "Wavetables: %zu (sine, square, sawtooth, triangle)\\n"
                 "Sample Rate: %.1f kHz\\n"
                 "Audio Buffer: 64 samples\\n"
                 "Oscillator: Sine @ 440 Hz\\n"
                 "Filter: Lowpass enabled",
                 wavetableManager->getNumWavetables(),
                 pipeline->getSampleRate() / 1000.0f);
        updateStatus(statusMsg);
        std::cout << "SynthComponent initialized with wavetable audio generation" << std::endl;
    } catch (const std::exception& e) {
        updateStatus("Error initializing synth");
        std::cerr << "Initialization error: " << e.what() << std::endl;
    }
}

void SynthComponent::cleanupSynth() {
    if (audioOutput) {
        audioOutput->stop();
        delete audioOutput;
        audioOutput = nullptr;
    }
    if (pipeline) {
        delete pipeline;
        pipeline = nullptr;
    }
    if (wavetableManager) {
        delete wavetableManager;
        wavetableManager = nullptr;
    }
}

void SynthComponent::show() {
    gtk_widget_show_all(main_window);
    gtk_main();
}

GtkWidget* SynthComponent::getMainWindow() {
    return main_window;
}

void SynthComponent::updateStatus(const char* message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(status_text));
    gtk_text_buffer_set_text(buffer, message, -1);
}
