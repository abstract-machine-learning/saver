#include "counterexample.h"

#include <malloc.h>
#include <string.h>

#include "../report_error.h"


/**
 * Structure of a counterexample.
 */
struct counterexample {
    Real *samples;                /**< Flattened array of samples. */
    unsigned int space_size;      /**< Size of the space. */
    unsigned int samples_number;  /**< Number of samples. */
};



void counterexample_create(
    Counterexample *counterexample,
    const unsigned int space_size,
    const unsigned int samples_number
) {
    unsigned int i;

    if (!counterexample) {
        report_error("Found null pointer.");
    }


    *counterexample = (Counterexample) malloc(sizeof(struct counterexample));
    if (!*counterexample) {
        report_error("Cannot allocate memory.");
    }

    (*counterexample)->samples = (Real *) malloc(space_size * samples_number * sizeof(Real));
    if (!(*counterexample)->samples) {
        report_error("Cannot allocate memory.");
    }

    (*counterexample)->space_size = space_size;
    (*counterexample)->samples_number = samples_number;

    for (i = 0; i < space_size * samples_number; ++i) {
        (*counterexample)->samples[i] = (Real) 0.0;
    }
}



void counterexample_delete(Counterexample *counterexample) {
    if (!*counterexample || !counterexample) {
        return;
    }

    free((*counterexample)->samples);
    free(*counterexample);
    *counterexample = NULL;
}



Real *counterexample_get_nth_sample(const Counterexample counterexample, const unsigned int n) {
    return counterexample ? counterexample->samples + n * counterexample->space_size : NULL;
}



unsigned int counterexample_get_space_size(Counterexample counterexample) {
    return counterexample ? counterexample->space_size : 0;
}



unsigned int counterexample_get_samples_number(const Counterexample counterexample) {
    return counterexample ? counterexample->samples_number : 0;
}



void counterexample_print(const Counterexample counterexample, FILE *stream) {
    unsigned int i, j;

    if (!counterexample) {
        return;
    }

    for (i = 0; i < counterexample->samples_number; ++i) {
        fprintf(stream, "(");
        for (j = 0; j < counterexample->space_size; ++j) {
            fprintf(stream, "%f", counterexample->samples[i * counterexample->space_size + j]);
            if (j + 1 < counterexample->space_size) {
                fprintf(stream, ", ");
            }
        }
        fprintf(stream, ")\n");
    }
}
