#include "options.h"

#include <stdio.h>
#include <string.h>

void read_options(Options *options, const int argc, const char *argv[]) {
    int i;
    options->counterexamples_file = NULL;
    options->debug_output = 0;

    for (i = 0; i < argc; ++i) {
        if (strcmp(argv[i], "--counterexamples-file") == 0 && argc > i + 1) {
            options->counterexamples_file = (char *) argv[i + 1];
            ++i;
        }
        else if (strcmp(argv[i], "--debug-output") == 0) {
            options->debug_output = 1;
        }
    }
}
