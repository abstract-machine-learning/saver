/**
 * A simple heuristic for finding robustness counterexamples.
 *
 * @file robustness_vertex_heuristic.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef ROBSTNESS_VERTEX_HEURISTIC_H
#define ROBSTNESS_VERTEX_HEURISTIC_H

#include "counterexample.h"
#include "../classifier.h"
#include "../adversarial_region.h"

/**
 * Searches for a robustness counterexample by applying a simple vertex heuristic.
 *
 * @param[out] counterexample Counterexample, if found
 * @param[in] classifier Classifier
 * @param[in] adversarial_region Adversial region
 * @return 1 if counterexample is found, 0 otherwise
 */
unsigned int robustness_vertex_heuristic(
    Counterexample counterexample,
    const Classifier classifier,
    const AdversarialRegion adversarial_region
);

#endif
