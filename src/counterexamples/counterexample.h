/**
 * A counterexample.
 *
 * A counterexample is a set of samples belonging to the same adversarial
 * region.
 *
 * @file counterexample.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef COUNTEREXAMPLE_H
#define COUNTEREXAMPLE_H

#include <stdio.h>

#include "../type.h"


/** Type of a counterexample. */
typedef struct counterexample *Counterexample;


/**
 * Creates a counterexample.
 *
 * Allocates memory for the pair of samples. Both samples initially
 * contain the origin vector.
 *
 * @param[out] counterexample Pointer of counterexample to create
 * @param[in] space_size Size of the space
 * @param[in] samples_number Number of samples
 * @note #counterexample_delete should be called to ensure proper memory deallocation.
 */
void counterexample_create(
    Counterexample *counterexample,
    const unsigned int space_size,
    const unsigned int samples_number
);


/**
 * Deletes a counterexample.
 *
 * Deallocates memory for the pair of samples.
 *
 * @param[out] counterexample Pointer to counterexample
 * @warning Memory is deallocated and content of the samples is lost.
 */
void counterexample_delete(Counterexample *counterexample);


/**
 * Returns n-th sample.
 *
 * @param[in] counterexample Counterexample
 * @param[in] n Number of the sample
 * @return n-th sample
 */
Real *counterexample_get_nth_sample(const Counterexample counterexample, const unsigned int n);


/**
 * Returns space size.
 *
 * @param[in] counterexample Counterexample
 * @return Size of the space
 */
unsigned int counterexample_get_space_size(const Counterexample counterexample);


/**
 * Returns number of samples in a counterexample.
 *
 * @param[in] counterexample Counterexample
 * @return Number of samples
 */
unsigned int counterexample_get_samples_number(const Counterexample counterexample);


/**
 * Prints a counterexample.
 *
 * @param[in] counterexample Counterexample
 * @param[in] stream Standard I/O stream to write to
 */
void counterexample_print(const Counterexample counterexample, FILE *stream);

#endif
