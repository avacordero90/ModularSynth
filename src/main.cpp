#include <gtk/gtk.h>
#include "app/synth_component.h"

// Application entry point: initialize GTK and run synth UI.
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