/**
 * RAF abstract classifier.
 *
 * Performs a RAF analysis on a classifier.
 *
 * @file raf_classifier.h
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#ifndef RAF_CLASSIFIER_H
#define RAF_CLASSIFIER_H

#include "abstract_classifier.h"
#include "../abstract_domains/raf.h"
#include "../classifier.h"

/** Type of a RAF classifier. */
typedef struct raf_classifier *RafClassifier;


/**
 * Creates a RAF classifier.
 *
 * @param[in] classifier Concrete classifier
 * @return RAF classifier
 * @note #raf_classifier_delete must be called to ensure proper memory
 *       deallocation
 */
RafClassifier raf_classifier_create(const Classifier classifier);


/**
 * Deletes a RAF classifier.
 *
 * @param[out] raf_classifier RAF classifier
 */
void raf_classifier_delete(RafClassifier *raf_classifier);

Interval *raf_classifier_ovo_score_helper(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    bool* fair_opt,
    unsigned int* has_counterexample,
    Raf *abstract_sample,
    float percent,
    float* RegSize
);

/**
 * Computes scores on an adversarial_region.
 *
 * @param[in] raf_classifier RAF classifier
 * @param[in] adversarial_region Adversarial region
 * @return Array of scores, each one being an #Interval
 */
Interval *raf_classifier_score(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region
);


/**
 * Computes possible classes of an adversarial region.
 *
 * @param[in] raf_classifier RAF classifier
 * @param[in] adversarial_region Adversarial region
 * @param[out] classes Array of class names
 * @return Number of possible classes
 */
unsigned int raf_classifier_classify(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    char **classes,
    bool* fair_opt,
    unsigned int* has_counterexample
);

#endif
