#include "counterexample_seeker.h"

#include <malloc.h>
#include <string.h>

#include "robustness_vertex_heuristic.h"
#include "robustness_genetic_algorithm.h"
#include "robustness_divide_et_impera.h"
#include "../report_error.h"
#include "../type.h"


/**
 * Structure of a counterexample seeker.
 */
struct counterexample_seeker {
    AbstractClassifier abstract_classifier;  /**< Abstract classifier. */
    CounterExampleType type;                 /**< Type of counterexamples. */
};



/***********************************************************************
 * Internal support functions.
 **********************************************************************/

unsigned int robustness_seeker(
    Counterexample counterexample,
    const CounterexampleSeeker counterexample_seeker,
    const AdversarialRegion adversarial_region
) {
    const AbstractClassifier abstract_classifier = counterexample_seeker->abstract_classifier;
    const Classifier classifier = abstract_classifier_get_classifier(abstract_classifier);
    unsigned int counterexample_found = 0;

    //counterexample_found = robustness_vertex_heuristic(counterexample, classifier, adversarial_region);
    //counterexample_found = robustness_genetic_algorithm(counterexample, classifier, adversarial_region);


(void) classifier;
    counterexample_found = robustness_divide_et_impera(counterexample, abstract_classifier, adversarial_region);

    if (counterexample_found) {
        return 1;
    }

    return 0;
}



/***********************************************************************
 * Public functions.
 **********************************************************************/

void counterexample_seeker_create(
    CounterexampleSeeker *counterexample_seeker,
    const AbstractClassifier abstract_classifier,
    const CounterExampleType type
) {
    if (!counterexample_seeker) {
        report_error("Null pointer found.");
    }

    *counterexample_seeker = (CounterexampleSeeker) malloc(sizeof(struct counterexample_seeker));
    if (!*counterexample_seeker) {
        report_error("Cannot allocate memory.");
    }

    (*counterexample_seeker)->abstract_classifier = abstract_classifier;
    (*counterexample_seeker)->type = type;
}



void counterexample_seeker_delete(CounterexampleSeeker *counterexample_seeker) {
    if (!counterexample_seeker || !*counterexample_seeker) {
        return;
    }

    free(*counterexample_seeker);
    *counterexample_seeker = NULL;
}



unsigned int counterexample_seeker_search(
    Counterexample counterexample,
    const CounterexampleSeeker counterexample_seeker,
    const AdversarialRegion adversarial_region
) {
    (void) adversarial_region;

    switch (counterexample_seeker->type) {
        case COUNTEREXAMPLE_ROBUSTNESS:
            return robustness_seeker(counterexample, counterexample_seeker, adversarial_region);

        default:
            report_error("Unsupported type of counterexample seeker.");
    }

    return 0;
}
