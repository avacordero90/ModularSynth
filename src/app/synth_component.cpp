#include "synth_component.h"
#include "../core/audio_output.h"
#include <cstdio>
#include <algorithm>
#include <iostream>
#include <vector>
#include <cmath>

// Build GTK UI shell and create all synth control sections.
SynthComponent::SynthComponent()
    : main_window(nullptr),
      wavetableManager(nullptr),
      freq_entry(nullptr),
      pipeline(nullptr),
      isArmed(false),
      activeMidiNote(-1),
      columns_spin(nullptr),
      time_sig_num_spin(nullptr),
      time_sig_den_combo(nullptr),
      sequencerColumns(16),
      timeSignatureNumerator(4),
      timeSignatureDenominator(4) {
    
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
    
    // Setup sections (sequencer first so it becomes notebook page 0)
    setupSequencerSection();
    setupOscillatorSection();
    setupFilterSection();
    setupEnvelopeSection();
    setupArmControl();
    
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

// Stop sequencer/audio and release synth resources.
SynthComponent::~SynthComponent() {
    stopSequencer();
    cleanupSynth();
}

// Create oscillator controls and bind callbacks.
void SynthComponent::setupOscillatorSection() {
    // Create oscillator frame
    GtkWidget *osc_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(osc_box), 10);
    
    // Frequency slider + entry (20–5000 Hz realistic audioband)
    freq_label = gtk_label_new("Frequency (Hz): 440");
    gtk_box_pack_start(GTK_BOX(osc_box), freq_label, FALSE, FALSE, 0);
    
    freq_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20.0, 5000.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(freq_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(freq_scale), 440.0);
    g_signal_connect(freq_scale, "value-changed", G_CALLBACK(SynthComponent::onFreqChanged), this);
    gtk_box_pack_start(GTK_BOX(osc_box), freq_scale, FALSE, FALSE, 0);

    // Text entry for frequency to allow direct numeric input
    freq_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(freq_entry), "440");
    gtk_box_pack_start(GTK_BOX(osc_box), freq_entry, FALSE, FALSE, 0);
    g_signal_connect(freq_entry, "activate", G_CALLBACK(SynthComponent::onFreqEntryActivated), this);
    
    // Detune slider
    detune_label = gtk_label_new("Detune (cents): 0");
    gtk_box_pack_start(GTK_BOX(osc_box), detune_label, FALSE, FALSE, 0);
    
    detune_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1200.0, 1200.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(detune_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(detune_scale), 0.0);
    g_signal_connect(detune_scale, "value-changed", G_CALLBACK(SynthComponent::onDetuneChanged), this);
    gtk_box_pack_start(GTK_BOX(osc_box), detune_scale, FALSE, FALSE, 0);

    // Text entry for detune so user can type precise cents value
    detune_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(detune_entry), "0");
    gtk_box_pack_start(GTK_BOX(osc_box), detune_entry, FALSE, FALSE, 0);
    g_signal_connect(detune_entry, "activate", G_CALLBACK(SynthComponent::onDetuneEntryActivated), this);
    
    // Waveform combo
    waveform_label = gtk_label_new("Waveform:");
    gtk_box_pack_start(GTK_BOX(osc_box), waveform_label, FALSE, FALSE, 0);
    
    waveform_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Sine");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Square");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Sawtooth");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(waveform_combo), NULL, "Triangle");
    gtk_combo_box_set_active(GTK_COMBO_BOX(waveform_combo), 0);
    g_signal_connect(waveform_combo, "changed", G_CALLBACK(SynthComponent::onWaveformChanged), this);
    gtk_box_pack_start(GTK_BOX(osc_box), waveform_combo, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), osc_box, gtk_label_new("Oscillator"));
}

// Create filter controls and bind callbacks.
void SynthComponent::setupFilterSection() {
    GtkWidget *filter_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(filter_box), 10);
    
    // Cutoff frequency slider
    cutoff_label = gtk_label_new("Cutoff (Hz): 1000");
    gtk_box_pack_start(GTK_BOX(filter_box), cutoff_label, FALSE, FALSE, 0);
    
    cutoff_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20.0, 20000.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(cutoff_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(cutoff_scale), 1000.0);
    g_signal_connect(cutoff_scale, "value-changed", G_CALLBACK(SynthComponent::onCutoffChanged), this);
    gtk_box_pack_start(GTK_BOX(filter_box), cutoff_scale, FALSE, FALSE, 0);

    // Text entry for cutoff frequency
    cutoff_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(cutoff_entry), "1000");
    gtk_box_pack_start(GTK_BOX(filter_box), cutoff_entry, FALSE, FALSE, 0);
    g_signal_connect(cutoff_entry, "activate", G_CALLBACK(SynthComponent::onCutoffEntryActivated), this);
    
    // Resonance slider
    res_label = gtk_label_new("Resonance: 0.71");
    gtk_box_pack_start(GTK_BOX(filter_box), res_label, FALSE, FALSE, 0);
    
    res_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.1, 10.0, 0.1);
    gtk_scale_set_value_pos(GTK_SCALE(res_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(res_scale), 0.707);
    g_signal_connect(res_scale, "value-changed", G_CALLBACK(SynthComponent::onResChanged), this);
    gtk_box_pack_start(GTK_BOX(filter_box), res_scale, FALSE, FALSE, 0);

    // Text entry for resonance
    res_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(res_entry), "0.71");
    gtk_box_pack_start(GTK_BOX(filter_box), res_entry, FALSE, FALSE, 0);
    g_signal_connect(res_entry, "activate", G_CALLBACK(SynthComponent::onResEntryActivated), this);
    
    // Filter type combo
    filter_type_label = gtk_label_new("Filter Type:");
    gtk_box_pack_start(GTK_BOX(filter_box), filter_type_label, FALSE, FALSE, 0);
    
    filter_type_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_type_combo), NULL, "Lowpass");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_type_combo), NULL, "Highpass");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(filter_type_combo), NULL, "Bandpass");
    gtk_combo_box_set_active(GTK_COMBO_BOX(filter_type_combo), 0);
    g_signal_connect(filter_type_combo, "changed", G_CALLBACK(SynthComponent::onFilterTypeChanged), this);
    gtk_box_pack_start(GTK_BOX(filter_box), filter_type_combo, FALSE, FALSE, 0);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), filter_box, gtk_label_new("Filter"));
}

// Create ADSR envelope controls and bind callbacks.
void SynthComponent::setupEnvelopeSection() {
    GtkWidget *env_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(env_box), 10);
    
    // Attack slider
    attack_label = gtk_label_new("Attack (s): 0.01");
    gtk_box_pack_start(GTK_BOX(env_box), attack_label, FALSE, FALSE, 0);
    
    attack_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.001, 5.0, 0.001);
    gtk_scale_set_value_pos(GTK_SCALE(attack_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(attack_scale), 0.01);
    g_signal_connect(attack_scale, "value-changed", G_CALLBACK(SynthComponent::onAttackChanged), this);
    gtk_box_pack_start(GTK_BOX(env_box), attack_scale, FALSE, FALSE, 0);

    // Text entry for attack time
    attack_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(attack_entry), "0.01");
    gtk_box_pack_start(GTK_BOX(env_box), attack_entry, FALSE, FALSE, 0);
    g_signal_connect(attack_entry, "activate", G_CALLBACK(SynthComponent::onAttackEntryActivated), this);
    
    // Decay slider
    decay_label = gtk_label_new("Decay (s): 0.1");
    gtk_box_pack_start(GTK_BOX(env_box), decay_label, FALSE, FALSE, 0);
    
    decay_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.001, 5.0, 0.001);
    gtk_scale_set_value_pos(GTK_SCALE(decay_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(decay_scale), 0.1);
    g_signal_connect(decay_scale, "value-changed", G_CALLBACK(SynthComponent::onDecayChanged), this);
    gtk_box_pack_start(GTK_BOX(env_box), decay_scale, FALSE, FALSE, 0);

    // Text entry for decay time
    decay_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(decay_entry), "0.1");
    gtk_box_pack_start(GTK_BOX(env_box), decay_entry, FALSE, FALSE, 0);
    g_signal_connect(decay_entry, "activate", G_CALLBACK(SynthComponent::onDecayEntryActivated), this);
    
    // Sustain slider
    sustain_label = gtk_label_new("Sustain: 0.7");
    gtk_box_pack_start(GTK_BOX(env_box), sustain_label, FALSE, FALSE, 0);
    
    sustain_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.0, 1.0, 0.01);
    gtk_scale_set_value_pos(GTK_SCALE(sustain_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(sustain_scale), 0.7);
    g_signal_connect(sustain_scale, "value-changed", G_CALLBACK(SynthComponent::onSustainChanged), this);
    gtk_box_pack_start(GTK_BOX(env_box), sustain_scale, FALSE, FALSE, 0);

    // Text entry for sustain level
    sustain_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(sustain_entry), "0.7");
    gtk_box_pack_start(GTK_BOX(env_box), sustain_entry, FALSE, FALSE, 0);
    g_signal_connect(sustain_entry, "activate", G_CALLBACK(SynthComponent::onSustainEntryActivated), this);
    
    // Release slider
    release_label = gtk_label_new("Release (s): 0.3");
    gtk_box_pack_start(GTK_BOX(env_box), release_label, FALSE, FALSE, 0);
    
    release_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 0.001, 5.0, 0.001);
    gtk_scale_set_value_pos(GTK_SCALE(release_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(release_scale), 0.3);
    g_signal_connect(release_scale, "value-changed", G_CALLBACK(SynthComponent::onReleaseChanged), this);
    gtk_box_pack_start(GTK_BOX(env_box), release_scale, FALSE, FALSE, 0);

    // Text entry for release time
    release_entry = gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(release_entry), "0.3");
    gtk_box_pack_start(GTK_BOX(env_box), release_entry, FALSE, FALSE, 0);
    g_signal_connect(release_entry, "activate", G_CALLBACK(SynthComponent::onReleaseEntryActivated), this);
    
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), env_box, gtk_label_new("Envelope"));
}

// Initialize synth engine, module graph, and MIDI callbacks.
void SynthComponent::initializeSynth() {
    try {
        // Create and initialize wavetable manager
        wavetableManager = new WavetableManager();
        
        // Generate standard wavetables with larger size for better quality
// generate higher‑resolution tables so the sine tone is smoother
    const size_t highRes = 8192;
    wavetableManager->generateSineWave("sine", highRes);
    wavetableManager->generateSquareWave("square", highRes);
    wavetableManager->generateSawtoothWave("sawtooth", highRes);
    wavetableManager->generateTriangleWave("triangle", highRes);
        
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

        // prepare audio output but do not start it until armed
        audioOutput = new AudioOutput(pipeline);

        // hook up MIDI handler so that sequencer (or external MIDI) can drive the pipeline
        midiHandler.setNoteOnCallback([this](MidiNote note){ handleMidiNoteOn(note); });
        midiHandler.setNoteOffCallback([this](MidiNote note){ handleMidiNoteOff(note); });
        
        // Update status with initialization details
        char statusMsg[512];
        snprintf(statusMsg, sizeof(statusMsg), 
                 "Synthesizer initialized successfully!\n"
                 "Wavetables: %zu (sine, square, sawtooth, triangle)\n"
                 "Sample Rate: %.1f kHz\n"
                 "Audio Buffer: 64 samples\n"
                 "Oscillator: Sine @ 440 Hz\n"
                 "Filter: Lowpass enabled\n"
                 "Armed: %s",
                 wavetableManager->getNumWavetables(),
                 pipeline->getSampleRate() / 1000.0f,
                 isArmed ? "yes" : "no");
        updateStatus(statusMsg);
#ifdef USE_PORTAUDIO
        std::cout << "SynthComponent initialized with real-time PortAudio output" << std::endl;
#else
        std::cout << "SynthComponent initialized with wavetable file output" << std::endl;
#endif
    } catch (const std::exception& e) {
        updateStatus("Error initializing synth");
        std::cerr << "Initialization error: " << e.what() << std::endl;
    }
}

// Destroy runtime synth objects in safe dependency order.
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

// Show top-level window and run GTK main loop.
void SynthComponent::show() {
    gtk_widget_show_all(main_window);
    gtk_main();
}

// Getter for top-level GTK window.
GtkWidget* SynthComponent::getMainWindow() {
    return main_window;
}

// Update status text panel with latest message.
void SynthComponent::updateStatus(const char* message) {
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(status_text));
    gtk_text_buffer_set_text(buffer, message, -1);
}

// oscillator callbacks
// Handle frequency slider movement and propagate to engine/UI.
void SynthComponent::onFreqChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Frequency (Hz): %.1f", val);
    gtk_label_set_text(GTK_LABEL(self->freq_label), buf);
    // also update entry if present
    if (self->freq_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.1f", val);
        gtk_entry_set_text(GTK_ENTRY(self->freq_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setOscillatorFrequency(0, static_cast<float>(val));
        std::cout << "Frequency set to " << val << " Hz" << std::endl;
    }
}

// Parse and clamp typed frequency value, then update slider.
void SynthComponent::onFreqEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 20.0) val = 20.0;
        if (val > 5000.0) val = 5000.0;
        gtk_range_set_value(GTK_RANGE(self->freq_scale), val);
        // onFreqChanged will handle updating label/pipeline
    } catch (...) {
        // invalid input; ignore or reset to current slider value
        double cur = gtk_range_get_value(GTK_RANGE(self->freq_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Handle detune slider movement and propagate to engine/UI.
void SynthComponent::onDetuneChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Detune (cents): %.0f", val);
    gtk_label_set_text(GTK_LABEL(self->detune_label), buf);
    // mirror value into entry field if available
    if (self->detune_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.0f", val);
        gtk_entry_set_text(GTK_ENTRY(self->detune_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setOscillatorDetune(0, static_cast<float>(val));
        std::cout << "Detune set to " << val << " cents" << std::endl;
    }
}

// Parse and clamp typed detune value, then update slider.
void SynthComponent::onDetuneEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < -1200.0) val = -1200.0;
        if (val > 1200.0) val = 1200.0;
        gtk_range_set_value(GTK_RANGE(self->detune_scale), val);
        // onDetuneChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->detune_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.0f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Handle cutoff slider movement and propagate to engine/UI.
void SynthComponent::onCutoffChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Cutoff (Hz): %.1f", val);
    gtk_label_set_text(GTK_LABEL(self->cutoff_label), buf);
    // mirror value into entry field if available
    if (self->cutoff_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.1f", val);
        gtk_entry_set_text(GTK_ENTRY(self->cutoff_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setFilterCutoff(0, static_cast<float>(val));
        std::cout << "Cutoff set to " << val << " Hz" << std::endl;
    }
}

// Parse and clamp typed cutoff value, then update slider.
void SynthComponent::onCutoffEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 20.0) val = 20.0;
        if (val > 20000.0) val = 20000.0;
        gtk_range_set_value(GTK_RANGE(self->cutoff_scale), val);
        // onCutoffChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->cutoff_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.1f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Handle resonance slider movement and propagate to engine/UI.
void SynthComponent::onResChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Resonance: %.2f", val);
    gtk_label_set_text(GTK_LABEL(self->res_label), buf);
    // mirror value into entry field if available
    if (self->res_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.2f", val);
        gtk_entry_set_text(GTK_ENTRY(self->res_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setFilterResonance(0, static_cast<float>(val));
        std::cout << "Resonance set to " << val << std::endl;
    }
}

// Parse and clamp typed resonance value, then update slider.
void SynthComponent::onResEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 0.1) val = 0.1;
        if (val > 10.0) val = 10.0;
        gtk_range_set_value(GTK_RANGE(self->res_scale), val);
        // onResChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->res_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Map filter-type combobox selection into filter enum.
void SynthComponent::onFilterTypeChanged(GtkComboBox* combo, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    int idx = gtk_combo_box_get_active(combo);
    if (self->pipeline) {
        // Map index to FilterType
        FilterType type;
        switch (idx) {
            case 0: type = FilterType::LOWPASS; break;
            case 1: type = FilterType::HIGHPASS; break;
            case 2: type = FilterType::BANDPASS; break;
            default: type = FilterType::LOWPASS; break;
        }
        self->pipeline->setFilterType(0, type);
        std::cout << "Filter type set to " << idx << std::endl;
    }
}

// Handle attack slider movement and propagate to engine/UI.
void SynthComponent::onAttackChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Attack (s): %.3f", val);
    gtk_label_set_text(GTK_LABEL(self->attack_label), buf);
    // mirror value into entry field if available
    if (self->attack_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.3f", val);
        gtk_entry_set_text(GTK_ENTRY(self->attack_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setEnvelopeAttack(0, static_cast<float>(val));
        std::cout << "Attack set to " << val << " s" << std::endl;
    }
}

// Handle decay slider movement and propagate to engine/UI.
void SynthComponent::onDecayChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Decay (s): %.3f", val);
    gtk_label_set_text(GTK_LABEL(self->decay_label), buf);
    // mirror value into entry field if available
    if (self->decay_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.3f", val);
        gtk_entry_set_text(GTK_ENTRY(self->decay_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setEnvelopeDecay(0, static_cast<float>(val));
        std::cout << "Decay set to " << val << " s" << std::endl;
    }
}

// Handle sustain slider movement and propagate to engine/UI.
void SynthComponent::onSustainChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Sustain: %.2f", val);
    gtk_label_set_text(GTK_LABEL(self->sustain_label), buf);
    // mirror value into entry field if available
    if (self->sustain_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.2f", val);
        gtk_entry_set_text(GTK_ENTRY(self->sustain_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setEnvelopeSustain(0, static_cast<float>(val));
        std::cout << "Sustain set to " << val << std::endl;
    }
}

// Handle release slider movement and propagate to engine/UI.
void SynthComponent::onReleaseChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Release (s): %.3f", val);
    gtk_label_set_text(GTK_LABEL(self->release_label), buf);
    // mirror value into entry field if available
    if (self->release_entry) {
        char numbuf[32];
        snprintf(numbuf, sizeof(numbuf), "%.3f", val);
        gtk_entry_set_text(GTK_ENTRY(self->release_entry), numbuf);
    }
    if (self->pipeline) {
        self->pipeline->setEnvelopeRelease(0, static_cast<float>(val));
        std::cout << "Release set to " << val << " s" << std::endl;
    }
}

// Parse and clamp typed attack value, then update slider.
void SynthComponent::onAttackEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 0.001) val = 0.001;
        if (val > 5.0) val = 5.0;
        gtk_range_set_value(GTK_RANGE(self->attack_scale), val);
        // onAttackChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->attack_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Parse and clamp typed decay value, then update slider.
void SynthComponent::onDecayEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 0.001) val = 0.001;
        if (val > 5.0) val = 5.0;
        gtk_range_set_value(GTK_RANGE(self->decay_scale), val);
        // onDecayChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->decay_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Parse and clamp typed sustain value, then update slider.
void SynthComponent::onSustainEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 0.0) val = 0.0;
        if (val > 1.0) val = 1.0;
        gtk_range_set_value(GTK_RANGE(self->sustain_scale), val);
        // onSustainChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->sustain_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.2f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Parse and clamp typed release value, then update slider.
void SynthComponent::onReleaseEntryActivated(GtkEntry* entry, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    const char* text = gtk_entry_get_text(entry);
    try {
        double val = std::stod(text);
        if (val < 0.001) val = 0.001;
        if (val > 5.0) val = 5.0;
        gtk_range_set_value(GTK_RANGE(self->release_scale), val);
        // onReleaseChanged will propagate to label/pipeline
    } catch (...) {
        double cur = gtk_range_get_value(GTK_RANGE(self->release_scale));
        char buf[32];
        snprintf(buf, sizeof(buf), "%.3f", cur);
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
    }
}

// Switch active oscillator wavetable from waveform selection.
void SynthComponent::onWaveformChanged(GtkComboBox* combo, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    int idx = gtk_combo_box_get_active(combo);
    const char* text = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    if (text) {
        gtk_label_set_text(GTK_LABEL(self->waveform_label), text);
    }
    if (self->pipeline) {
        // map index to wavetable name
        static const char* names[] = {"sine","square","sawtooth","triangle"};
        if (idx >= 0 && idx < 4) {
            self->pipeline->setOscillatorWavetable(0, names[idx]);
            std::cout << "Waveform changed to " << names[idx] << std::endl;
        }
    }
}

// arm control setup and callbacks
// Create arm/disarm toggle and attach it into main layout.
void SynthComponent::setupArmControl() {
    // add an "arm" toggle at the top of the main window
    GtkWidget *arm_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    arm_button = gtk_toggle_button_new_with_label("Arm");
    g_signal_connect(arm_button, "clicked", G_CALLBACK(SynthComponent::onArmToggled), this);

    // insert into the main vbox before the notebook
    GtkWidget *parent = gtk_widget_get_parent(notebook);
    if (GTK_IS_BOX(parent)) {
        gtk_box_pack_start(GTK_BOX(parent), arm_box, FALSE, FALSE, 0);
        gtk_box_pack_start(GTK_BOX(arm_box), arm_button, FALSE, FALSE, 0);
    }

    isArmed = false;
}

// Forward arm-toggle widget state to component state handler.
void SynthComponent::onArmToggled(GtkToggleButton* toggle, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    bool active = gtk_toggle_button_get_active(toggle);
    self->setArmed(active);
}

// Arm or disarm audio output and reset playing state on disarm.
void SynthComponent::setArmed(bool armed) {
    if (armed == isArmed) return;
    isArmed = armed;
    if (armed) {
        if (audioOutput && !audioOutput->isRunning()) {
            audioOutput->start();
        }
        updateStatus("System armed");
    } else {
        if (audioOutput && audioOutput->isRunning()) {
            audioOutput->stop();
        }
        // ensure any playing notes stop
        if (pipeline) pipeline->noteOff();
        activeMidiNote = -1;
        updateStatus("System disarmed");
    }
}


/********************************************************
 * Sequencer + MIDI helper implementations
 ********************************************************/

/********************************************************
 * Sequencer + MIDI helper implementations
 ********************************************************/

// Timer callback that advances sequencer one step per tick.
gboolean SynthComponent::sequencerTimeoutCallback(gpointer data) {
    SynthComponent* self = static_cast<SynthComponent*>(data);
    self->advanceSequencer();
    return G_SOURCE_CONTINUE;
}

// Advance sequencer, emit note events, and rotate step index.
void SynthComponent::advanceSequencer() {
    // stop any notes that were active on the previous step
    for (int note : prevActiveNotes) {
        MidiNote off(note, 0.0f, false, 0);
        handleMidiNoteOff(off);
    }
    prevActiveNotes.clear();

    if (seqState.empty() || seqState[0].empty())
        return;

    int rows = seqState.size();
    int cols = seqState[0].size();

    // for every row in the current column, fire a note on if active
    for (int r = 0; r < rows; ++r) {
        if (seqState[r][currentStep]) {
            int midiNote = 60 + r; // C4 = 60 base
            MidiNote onMsg(midiNote, 1.0f, true, 0);
            handleMidiNoteOn(onMsg);
            prevActiveNotes.push_back(midiNote);
        }
    }

    // increment step
    currentStep = (currentStep + 1) % cols;
}

// Start sequencer timing loop when synth is armed.
void SynthComponent::startSequencer() {
    if (sequencerRunning) return;
    if (!isArmed) {
        updateStatus("Cannot start sequencer: synth is unarmed");
        return;
    }
    sequencerRunning = true;
    currentStep = 0;
    guint interval = calculateSequencerIntervalMs();
    sequencerTimeoutId = g_timeout_add(interval, sequencerTimeoutCallback, this);
    updateStatus("Sequencer started");
}

// Stop sequencer timing loop and release any held notes.
void SynthComponent::stopSequencer() {
    if (!sequencerRunning) return;
    sequencerRunning = false;
    if (sequencerTimeoutId) {
        g_source_remove(sequencerTimeoutId);
        sequencerTimeoutId = 0;
    }
    // turn off any lingering notes
    for (int note : prevActiveNotes) {
        MidiNote off(note, 0.0f, false, 0);
        handleMidiNoteOff(off);
    }
    prevActiveNotes.clear();
    updateStatus("Sequencer stopped");
}

// static callbacks that forward to instance methods
// Forward start-button click to sequencer start method.
void SynthComponent::onStartButtonClicked(GtkWidget* widget, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->startSequencer();
}

// Forward stop-button click to sequencer stop method.
void SynthComponent::onStopButtonClicked(GtkWidget* widget, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->stopSequencer();
}

// Update sequencer BPM and restart timer if already running.
void SynthComponent::onTempoChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->sequencerBPM = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Tempo: %.1f BPM", self->sequencerBPM);
    self->updateStatus(buf);
    self->restartSequencerTimerIfRunning();
}

// Update sequencer column count and rebuild the editable grid.
void SynthComponent::onColumnsChanged(GtkSpinButton* spin, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    int newCols = gtk_spin_button_get_value_as_int(spin);
    self->rebuildSequencerGrid(newCols);
}

// Update time-signature numerator and refresh running timer.
void SynthComponent::onTimeSignatureNumeratorChanged(GtkSpinButton* spin, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->timeSignatureNumerator = gtk_spin_button_get_value_as_int(spin);
    self->restartSequencerTimerIfRunning();
}

// Update time-signature denominator and refresh running timer.
void SynthComponent::onTimeSignatureDenominatorChanged(GtkComboBox* combo, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    int idx = gtk_combo_box_get_active(combo);
    switch (idx) {
        case 0: self->timeSignatureDenominator = 2; break;
        case 1: self->timeSignatureDenominator = 4; break;
        case 2: self->timeSignatureDenominator = 8; break;
        case 3: self->timeSignatureDenominator = 16; break;
        default: self->timeSignatureDenominator = 4; break;
    }
    self->applySequencerSegments();
    self->restartSequencerTimerIfRunning();
}

// Persist grid toggle state in sequencer step matrix.
void SynthComponent::onGridToggle(GtkToggleButton* toggle, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    int row = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toggle), "row"));
    int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toggle), "col"));
    bool active = gtk_toggle_button_get_active(toggle);
    self->seqState[row][col] = active;
}

// Convert MIDI note-on into pipeline trigger and track active note.
void SynthComponent::handleMidiNoteOn(const MidiNote& note) {
    if (!isArmed) return;
    float freq = midiNoteToFrequency(note.noteNumber);
    if (pipeline) {
        pipeline->noteOn(freq, note.noteNumber);
        activeMidiNote = note.noteNumber;
    }
}

// Convert matching MIDI note-off into pipeline release trigger.
void SynthComponent::handleMidiNoteOff(const MidiNote& note) {
    if (!isArmed) return;
    if (pipeline && (activeMidiNote < 0 || note.noteNumber == activeMidiNote)) {
        pipeline->noteOff(note.noteNumber);
        activeMidiNote = -1;
    }
}

// Convert MIDI note number to equal-temperament frequency in Hz.
float SynthComponent::midiNoteToFrequency(int noteNumber) const {
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

// Build step-sequencer UI and initialize sequencer state buffers.
void SynthComponent::setupSequencerSection() {
    // make sure arm control exists before sequencer start
    // initialize state
    const int rows = 12;
    const int cols = sequencerColumns;
    sequencerBPM = 120.0f;
    currentStep = 0;
    sequencerRunning = false;
    seqState.assign(rows, std::vector<bool>(cols, false));
    seqButtons.assign(rows, std::vector<GtkWidget*>(cols, nullptr));

    GtkWidget *seq_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(seq_box), 10);

    // tempo control
    tempo_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 30.0, 300.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(tempo_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(tempo_scale), sequencerBPM);
    g_signal_connect(tempo_scale, "value-changed", G_CALLBACK(SynthComponent::onTempoChanged), this);
    gtk_box_pack_start(GTK_BOX(seq_box), tempo_scale, FALSE, FALSE, 0);

    // column and time-signature controls
    GtkWidget *settings_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    GtkWidget *cols_label = gtk_label_new("Columns:");
    columns_spin = gtk_spin_button_new_with_range(4.0, 64.0, 1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(columns_spin), sequencerColumns);
    g_signal_connect(columns_spin, "value-changed", G_CALLBACK(SynthComponent::onColumnsChanged), this);

    GtkWidget *time_sig_label = gtk_label_new("Time Sig:");
    time_sig_num_spin = gtk_spin_button_new_with_range(1.0, 12.0, 1.0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(time_sig_num_spin), timeSignatureNumerator);
    g_signal_connect(time_sig_num_spin, "value-changed", G_CALLBACK(SynthComponent::onTimeSignatureNumeratorChanged), this);

    GtkWidget *slash_label = gtk_label_new("/");
    time_sig_den_combo = gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(time_sig_den_combo), nullptr, "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(time_sig_den_combo), nullptr, "4");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(time_sig_den_combo), nullptr, "8");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(time_sig_den_combo), nullptr, "16");
    gtk_combo_box_set_active(GTK_COMBO_BOX(time_sig_den_combo), 1);
    g_signal_connect(time_sig_den_combo, "changed", G_CALLBACK(SynthComponent::onTimeSignatureDenominatorChanged), this);

    gtk_box_pack_start(GTK_BOX(settings_box), cols_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), columns_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), time_sig_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), time_sig_num_spin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), slash_label, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(settings_box), time_sig_den_combo, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(seq_box), settings_box, FALSE, FALSE, 0);

    // start/stop buttons
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    start_button = gtk_button_new_with_label("Start");
    stop_button = gtk_button_new_with_label("Stop");
    g_signal_connect(start_button, "clicked", G_CALLBACK(SynthComponent::onStartButtonClicked), this);
    g_signal_connect(stop_button, "clicked", G_CALLBACK(SynthComponent::onStopButtonClicked), this);
    gtk_box_pack_start(GTK_BOX(button_box), start_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), stop_button, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(seq_box), button_box, FALSE, FALSE, 0);

    // grid of toggle buttons
    sequencer_grid = gtk_grid_new();
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            GtkWidget *toggle = gtk_toggle_button_new();
            gtk_grid_attach(GTK_GRID(sequencer_grid), toggle, c, r, 1, 1);
            seqButtons[r][c] = toggle;
            g_object_set_data(G_OBJECT(toggle), "row", GINT_TO_POINTER(r));
            g_object_set_data(G_OBJECT(toggle), "col", GINT_TO_POINTER(c));
            g_signal_connect(toggle, "toggled", G_CALLBACK(SynthComponent::onGridToggle), this);
        }
    }
    applySequencerSegments();
    gtk_box_pack_start(GTK_BOX(seq_box), sequencer_grid, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), seq_box, gtk_label_new("Sequencer"));
}

// Rebuild sequencer grid while preserving any existing step state.
void SynthComponent::rebuildSequencerGrid(int newColumnCount) {
    if (newColumnCount < 1 || !sequencer_grid || seqState.empty()) {
        return;
    }

    const int rows = static_cast<int>(seqState.size());
    std::vector<std::vector<bool>> oldState = seqState;
    const int oldCols = static_cast<int>(oldState[0].size());
    sequencerColumns = newColumnCount;

    seqState.assign(rows, std::vector<bool>(sequencerColumns, false));
    seqButtons.assign(rows, std::vector<GtkWidget*>(sequencerColumns, nullptr));

    const int copyCols = std::min(oldCols, sequencerColumns);
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < copyCols; ++c) {
            seqState[r][c] = oldState[r][c];
        }
    }

    GList* children = gtk_container_get_children(GTK_CONTAINER(sequencer_grid));
    for (GList* child = children; child != nullptr; child = child->next) {
        gtk_widget_destroy(GTK_WIDGET(child->data));
    }
    g_list_free(children);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < sequencerColumns; ++c) {
            GtkWidget *toggle = gtk_toggle_button_new();
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(toggle), seqState[r][c]);
            gtk_grid_attach(GTK_GRID(sequencer_grid), toggle, c, r, 1, 1);
            seqButtons[r][c] = toggle;
            g_object_set_data(G_OBJECT(toggle), "row", GINT_TO_POINTER(r));
            g_object_set_data(G_OBJECT(toggle), "col", GINT_TO_POINTER(c));
            g_signal_connect(toggle, "toggled", G_CALLBACK(SynthComponent::onGridToggle), this);
        }
    }
    applySequencerSegments();
    gtk_widget_show_all(sequencer_grid);

    currentStep = (sequencerColumns > 0) ? (currentStep % sequencerColumns) : 0;
    restartSequencerTimerIfRunning();
}

// Add visible spacing segments across columns from time-signature denominator.
void SynthComponent::applySequencerSegments() {
    if (seqButtons.empty() || seqButtons[0].empty()) {
        return;
    }

    const int rows = static_cast<int>(seqButtons.size());
    const int cols = static_cast<int>(seqButtons[0].size());
    const int segmentSize = std::max(1, cols / std::max(1, timeSignatureDenominator));

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            GtkWidget* toggle = seqButtons[r][c];
            if (!toggle) {
                continue;
            }

            const bool boundary = (c > 0) && (c % segmentSize == 0);
            gtk_widget_set_margin_start(toggle, boundary ? 8 : 1);
            gtk_widget_set_margin_end(toggle, 1);
            gtk_widget_set_margin_top(toggle, 1);
            gtk_widget_set_margin_bottom(toggle, 1);
        }
    }
}

// Calculate per-step timer interval from BPM, time signature, and column count.
guint SynthComponent::calculateSequencerIntervalMs() const {
    if (sequencerColumns <= 0 || sequencerBPM <= 0.0f) {
        return 125;
    }

    const float msPerBeat = 60000.0f / sequencerBPM;
    const float beatsPerBar = static_cast<float>(timeSignatureNumerator) *
                              (4.0f / static_cast<float>(timeSignatureDenominator));
    const float msPerBar = msPerBeat * beatsPerBar;
    const float interval = msPerBar / static_cast<float>(sequencerColumns);
    return static_cast<guint>(std::max(10.0f, interval));
}

// Recreate timer with updated interval if sequencer is already running.
void SynthComponent::restartSequencerTimerIfRunning() {
    if (!sequencerRunning) {
        return;
    }
    if (sequencerTimeoutId) {
        g_source_remove(sequencerTimeoutId);
    }
    sequencerTimeoutId = g_timeout_add(calculateSequencerIntervalMs(), sequencerTimeoutCallback, this);
}
