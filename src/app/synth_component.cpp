#include "synth_component.h"
#include "../core/audio_output.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include <cmath>

SynthComponent::SynthComponent()
    : main_window(nullptr),
      wavetableManager(nullptr),
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
    
    // Frequency slider
    freq_label = gtk_label_new("Frequency (Hz): 440");
    gtk_box_pack_start(GTK_BOX(osc_box), freq_label, FALSE, FALSE, 0);
    
    freq_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, 20.0, 20000.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(freq_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(freq_scale), 440.0);
    g_signal_connect(freq_scale, "value-changed", G_CALLBACK(SynthComponent::onFreqChanged), this);
    gtk_box_pack_start(GTK_BOX(osc_box), freq_scale, FALSE, FALSE, 0);
    
    // Detune slider
    detune_label = gtk_label_new("Detune (cents): 0");
    gtk_box_pack_start(GTK_BOX(osc_box), detune_label, FALSE, FALSE, 0);
    
    detune_scale = gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL, -1200.0, 1200.0, 1.0);
    gtk_scale_set_value_pos(GTK_SCALE(detune_scale), GTK_POS_RIGHT);
    gtk_range_set_value(GTK_RANGE(detune_scale), 0.0);
    g_signal_connect(detune_scale, "value-changed", G_CALLBACK(SynthComponent::onDetuneChanged), this);
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

// oscillator callbacks
void SynthComponent::onFreqChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Frequency (Hz): %.1f", val);
    gtk_label_set_text(GTK_LABEL(self->freq_label), buf);
    if (self->pipeline) {
        self->pipeline->setOscillatorFrequency(0, static_cast<float>(val));
        std::cout << "Frequency set to " << val << " Hz" << std::endl;
    }
}

void SynthComponent::onDetuneChanged(GtkRange* range, gpointer userData) {
    SynthComponent* self = static_cast<SynthComponent*>(userData);
    double val = gtk_range_get_value(range);
    char buf[64];
    snprintf(buf, sizeof(buf), "Detune (cents): %.0f", val);
    gtk_label_set_text(GTK_LABEL(self->detune_label), buf);
    if (self->pipeline) {
        self->pipeline->setOscillatorDetune(0, static_cast<float>(val));
        std::cout << "Detune set to " << val << " cents" << std::endl;
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
