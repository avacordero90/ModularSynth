#ifndef SYNTH_COMPONENT_H
#define SYNTH_COMPONENT_H

#include <gtk/gtk.h>
#include "../core/audio_pipeline.h"
#include "../core/wavetable_manager.h"
#include "../modules/oscillator.h"
#include "../modules/filter.h"
#include "../modules/envelope.h"

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
    
    // Filter section
    GtkWidget *filter_frame;
    GtkWidget *cutoff_scale;
    GtkWidget *res_scale;
    GtkWidget *filter_type_combo;
    GtkWidget *cutoff_label;
    GtkWidget *res_label;
    GtkWidget *filter_type_label;
    
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
    
    // Status
    GtkWidget *status_text;
    
    // Synth engine
    WavetableManager* wavetableManager;
    AudioPipeline* pipeline;
    std::unique_ptr<Oscillator> oscillator;
    std::unique_ptr<Filter> filter;
    std::unique_ptr<Envelope> envelope;

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
};

#endif // SYNTH_COMPONENT_H
