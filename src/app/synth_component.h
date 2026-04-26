#ifndef SYNTH_COMPONENT_H
#define SYNTH_COMPONENT_H

#include <gtk/gtk.h>
#include "midi_handler.h"
#include "../core/audio_pipeline.h"
#include "../core/wavetable_manager.h"
#include "../modules/oscillator.h"
#include "../modules/filter.h"
#include "../modules/envelope.h"

// forward declarations
class AudioOutput;

/**
 * Main synthesizer component for GTK GUI
 */
class SynthComponent {
private:
    // GTK widgets
    GtkWidget *main_window;
    GtkWidget *notebook;
    
    // Oscillator section
    GtkWidget *osc_frame;
    GtkWidget *freq_scale;
    GtkWidget *detune_scale;
    GtkWidget *waveform_combo;
    GtkWidget *freq_label;
    GtkWidget *detune_label;
    GtkWidget *waveform_label;
    GtkWidget *freq_entry;          // numeric entry for frequency
    GtkWidget *detune_entry;        // numeric entry for detune (cents)
    
    // Filter section
    GtkWidget *filter_frame;
    GtkWidget *cutoff_scale;
    GtkWidget *res_scale;
    GtkWidget *filter_type_combo;
    GtkWidget *cutoff_label;
    GtkWidget *res_label;
    GtkWidget *filter_type_label;
    GtkWidget *cutoff_entry;        // numeric entry for cutoff frequency
    GtkWidget *res_entry;           // numeric entry for resonance
    
    // Envelope section
    GtkWidget *env_frame;
    GtkWidget *attack_scale;
    GtkWidget *decay_scale;
    GtkWidget *sustain_scale;
    GtkWidget *release_scale;
    GtkWidget *attack_label;
    GtkWidget *decay_label;
    GtkWidget *sustain_label;
    GtkWidget *release_label;
    GtkWidget *attack_entry;        // numeric entry for attack time
    GtkWidget *decay_entry;         // numeric entry for decay time
    GtkWidget *sustain_entry;       // numeric entry for sustain level
    GtkWidget *release_entry;       // numeric entry for release time
    
    // Status
    GtkWidget *status_text;
    
    // Synth engine
    WavetableManager* wavetableManager;
    AudioPipeline* pipeline;
    std::unique_ptr<Oscillator> oscillator;
    std::unique_ptr<Filter> filter;
    std::unique_ptr<Envelope> envelope;

    // audio output subsystem (PortAudio)
    AudioOutput* audioOutput;

    // MIDI handling and sequencing
    MidiHandler midiHandler;

    // arming control (prevents sound until armed)
    GtkWidget *arm_button;
    bool isArmed;

    // Sequencer UI
    GtkWidget *sequencer_frame;
    GtkWidget *sequencer_grid;
    GtkWidget *start_button;
    GtkWidget *stop_button;
    GtkWidget *tempo_scale;
    GtkWidget *columns_spin;
    GtkWidget *time_sig_num_spin;
    GtkWidget *time_sig_den_combo;

    // sequencer state (rows = notes, columns = steps)
    std::vector<std::vector<GtkWidget*>> seqButtons;
    std::vector<std::vector<bool>> seqState;
    std::vector<int> prevActiveNotes;
    int activeMidiNote;
    int currentStep;
    bool sequencerRunning;
    guint sequencerTimeoutId;
    float sequencerBPM;
    int sequencerColumns;
    int timeSignatureNumerator;
    int timeSignatureDenominator;

public:
    SynthComponent();
    ~SynthComponent();

    void initializeSynth();
    void cleanupSynth();
    void show();
    GtkWidget* getMainWindow();
    
    void updateStatus(const char* message);

private:
    void setupOscillatorSection();
    void setupFilterSection();
    void setupEnvelopeSection();
    void setupSequencerSection();
    void setupArmControl();

    // oscillator control callbacks
    static void onFreqChanged(GtkRange* range, gpointer userData);
    static void onFreqEntryActivated(GtkEntry* entry, gpointer userData);
    static void onDetuneChanged(GtkRange* range, gpointer userData);
    static void onDetuneEntryActivated(GtkEntry* entry, gpointer userData);
    static void onWaveformChanged(GtkComboBox* combo, gpointer userData);

    // filter control callbacks
    static void onCutoffChanged(GtkRange* range, gpointer userData);
    static void onCutoffEntryActivated(GtkEntry* entry, gpointer userData);
    static void onResChanged(GtkRange* range, gpointer userData);
    static void onResEntryActivated(GtkEntry* entry, gpointer userData);
    static void onFilterTypeChanged(GtkComboBox* combo, gpointer userData);

    // envelope control callbacks
    static void onAttackChanged(GtkRange* range, gpointer userData);
    static void onAttackEntryActivated(GtkEntry* entry, gpointer userData);
    static void onDecayChanged(GtkRange* range, gpointer userData);
    static void onDecayEntryActivated(GtkEntry* entry, gpointer userData);
    static void onSustainChanged(GtkRange* range, gpointer userData);
    static void onSustainEntryActivated(GtkEntry* entry, gpointer userData);
    static void onReleaseChanged(GtkRange* range, gpointer userData);
    static void onReleaseEntryActivated(GtkEntry* entry, gpointer userData);

    // Sequencer control callbacks
    static gboolean sequencerTimeoutCallback(gpointer data);
    void advanceSequencer();
    static void onStartButtonClicked(GtkWidget* widget, gpointer userData);
    static void onStopButtonClicked(GtkWidget* widget, gpointer userData);
    static void onTempoChanged(GtkRange* range, gpointer userData);
    static void onColumnsChanged(GtkSpinButton* spin, gpointer userData);
    static void onTimeSignatureNumeratorChanged(GtkSpinButton* spin, gpointer userData);
    static void onTimeSignatureDenominatorChanged(GtkComboBox* combo, gpointer userData);
    static void onGridToggle(GtkToggleButton* toggle, gpointer userData);
    static void onArmToggled(GtkToggleButton* toggle, gpointer userData);

    // Sequencer control helpers
    void startSequencer();
    void stopSequencer();
    void rebuildSequencerGrid(int newColumnCount);
    guint calculateSequencerIntervalMs() const;
    void restartSequencerTimerIfRunning();

    // MIDI handling helpers
    void handleMidiNoteOn(const MidiNote& note);
    void handleMidiNoteOff(const MidiNote& note);
    float midiNoteToFrequency(int noteNumber) const;
    
    // arm state management
    void setArmed(bool armed);
};

#endif // SYNTH_COMPONENT_H
