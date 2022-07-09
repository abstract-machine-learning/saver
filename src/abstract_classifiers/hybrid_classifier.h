/**
 * RAF + Interval hybrid abstract classifier.
 *
 * Performs an analysis mixing results of interval and RAF analyzers to
 * improve precision.
 *
 * @file hybrid_classifier.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef HYBRID_CLASSIFIER_H
#define HYBRID_CLASSIFIER_H

#include "abstract_classifier.h"
#include "../abstract_domains/interval.h"
#include "../classifier.h"

/** Type of a hybrid classifier. */
typedef struct hybrid_classifier *HybridClassifier;


/**
 * Creates a hybrid classifier.
 *
 * @param[in] classifier Concrete classifier
 * @return Hybrid classifier
 * @note #hybrid_classifier_delete should be called to ensure proper memory
 *       deallocation.
 */
HybridClassifier hybrid_classifier_create(const Classifier classifier);


/**
 * Deletes a hybrid classifier.
 *
 * @param[out] hybrid_classifier Pointer to hybrid classifier
 */
void hybrid_classifier_delete(HybridClassifier *hybrid_classifier);


/**
 * Computes scores on an adversarial_region.
 *
 * @param[in] hybrid_classifier Hybrid classifier
 * @param[in] adversarial_region Adversarial region
 * @return Array of scores, each one being an #Interval
 */
Interval *hybrid_classifier_score(
    const HybridClassifier hybrid_classifier,
    const AdversarialRegion adversarial_region,
    bool* fair_opt,
    unsigned int* has_counterexample
);


/**
 * Computes possible classes of an adversarial region.
 *
 * @param[in] hybrid_classifier Hybrid classifier
 * @param[in] adversarial_region Adversarial region
 * @param[out] classes Array of class names
 * @return Number of possible classes
 */
unsigned int hybrid_classifier_classify(
    const HybridClassifier hybrid_classifier,
    const AdversarialRegion adversarial_region,
    char **classes,
    bool* fair_opt,
    unsigned int* has_counterexample
);

#endif
