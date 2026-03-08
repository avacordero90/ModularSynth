#include "synth_component.h"
#include "../core/audio_output.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>

SynthComponent::SynthComponent()
    : main_window(nullptr),
      wavetableManager(nullptr),
      freq_entry(nullptr),
      pipeline(nullptr),
      isArmed(false) {
    
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
    setupSequencerSection();
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

SynthComponent::~SynthComponent() {
    stopSequencer();
    cleanupSynth();
}

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

// oscillator callbacks
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

void SynthComponent::onArmToggled(GtkToggleButton* toggle, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    bool active = gtk_toggle_button_get_active(toggle);
    self->setArmed(active);
}

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
        updateStatus("System disarmed");
    }
}


/********************************************************
 * Sequencer + MIDI helper implementations
 ********************************************************/

gboolean SynthComponent::sequencerTimeoutCallback(gpointer data) {
    SynthComponent* self = static_cast<SynthComponent*>(data);
    self->advanceSequencer();
    return G_SOURCE_CONTINUE;
}

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

void SynthComponent::startSequencer() {
    if (sequencerRunning) return;
    if (!isArmed) {
        updateStatus("Cannot start sequencer: synth is unarmed");
        return;
    }
    sequencerRunning = true;
    currentStep = 0;
    // convert BPM to interval for 16th-notes (4 per quarter note)
    guint interval = (guint)(60000.0f / (sequencerBPM * 4.0f));
    sequencerTimeoutId = g_timeout_add(interval, sequencerTimeoutCallback, this);
    updateStatus("Sequencer started");
}

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
void SynthComponent::onStartButtonClicked(GtkWidget* widget, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->startSequencer();
}

void SynthComponent::onStopButtonClicked(GtkWidget* widget, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->stopSequencer();
}

void SynthComponent::onTempoChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    self->sequencerBPM = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Tempo: %.1f BPM", self->sequencerBPM);
    self->updateStatus(buf);
    // if running, restart timer with new interval
    if (self->sequencerRunning) {
        self->stopSequencer();
        self->startSequencer();
    }
}

void SynthComponent::onGridToggle(GtkToggleButton* toggle, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    int row = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toggle), "row"));
    int col = GPOINTER_TO_INT(g_object_get_data(G_OBJECT(toggle), "col"));
    bool active = gtk_toggle_button_get_active(toggle);
    self->seqState[row][col] = active;
}

void SynthComponent::handleMidiNoteOn(const MidiNote& note) {
    if (!isArmed) return;
    float freq = midiNoteToFrequency(note.noteNumber);
    if (pipeline) {
        pipeline->noteOn(freq);
    }
}

void SynthComponent::handleMidiNoteOff(const MidiNote& note) {
    if (!isArmed) return;
    if (pipeline) {
        pipeline->noteOff();
    }
}

float SynthComponent::midiNoteToFrequency(int noteNumber) const {
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

void SynthComponent::setupSequencerSection() {
    // make sure arm control exists before sequencer start
    // initialize state
    const int rows = 12;
    const int cols = 16;
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
    gtk_box_pack_start(GTK_BOX(seq_box), sequencer_grid, TRUE, TRUE, 0);

    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), seq_box, gtk_label_new("Sequencer"));
}
