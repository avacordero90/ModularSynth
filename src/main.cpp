#include <gtk/gtk.h>
#include "plugin/synth_component.h"

int main(int argc, char *argv[]) {
    gtk_init(&argc, &argv);
    
    // Create the synth component
    SynthComponent synth;
    
    // Initialize synthesizer
    synth.initializeSynth();
    
    // Show window and run GTK main loop
    synth.show();
    
    return 0;
}