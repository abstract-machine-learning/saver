#include "robustness_vertex_heuristic.h"

#include <string.h>

#include "../report_error.h"

static void get_boundaries(
    Real *lowerbound,
    Real *upperbound,
    const AdversarialRegion adversarial_region,
    const unsigned int i
) {
    const Real *sample = adversarial_region.sample;
    const Perturbation perturbation = adversarial_region.perturbation;
    const Real magnitude = perturbation_get_magnitude(perturbation);

    switch (perturbation_get_type(perturbation)) {
         case PERTURBATION_L_INF:
             *lowerbound = sample[i] - magnitude >= 0.0 ? sample[i] - magnitude : 0.0;
             *upperbound = sample[i] + magnitude <= 1.0 ? sample[i] + magnitude : 1.0;
             break;

        case PERTURBATION_HYPER_RECTANGLE:
            *lowerbound = sample[i] - perturbation_get_epsilon_lowerbounds(perturbation)[i];
            *upperbound = sample[i] + perturbation_get_epsilon_upperbounds(perturbation)[i];
            break;

        default:
            report_error("Unsupported type of adversarial region.");
    }
}



// Move to separate file?
static unsigned int is_counterexample(
    const Classifier classifier,
    const Real *sample_a,
    const Real *sample_b
) {
    unsigned int n_classes_a, n_classes_b, i;
    char **classes_a, **classes_b;

    classes_a = (char **) malloc(classifier_get_n_classes(classifier) * sizeof(char *));
    classes_b = (char **) malloc(classifier_get_n_classes(classifier) * sizeof(char *));

    n_classes_a = classifier_classify(classifier, sample_a, classes_a);
    n_classes_b = classifier_classify(classifier, sample_b, classes_b);


    if (n_classes_a != n_classes_b) {
        free(classes_a);
        free(classes_b);
        return 1;
    }

    for (i = 0; i < n_classes_a; ++i) {
        if (strcmp(classes_a[i], classes_b[i]) != 0) {
            free(classes_a);
            free(classes_b);
            return 1;
        }
    }

    free(classes_a);
    free(classes_b);
    return 0;
}



unsigned int robustness_vertex_heuristic(
    Counterexample counterexample,
    const Classifier classifier,
    const AdversarialRegion adversarial_region
) {
    unsigned int i, j, found = 0;
    const unsigned int space_size = classifier_get_space_size(classifier);
    const Real *sample = adversarial_region.sample;
    Real *sample_l = counterexample_get_nth_sample(counterexample, 0),
         *sample_u = counterexample_get_nth_sample(counterexample, 1);
    const unsigned int n_classes = classifier_get_n_classes(classifier);
    int *derivative_sign;

    derivative_sign = (int *) malloc(n_classes * (n_classes - 1) / 2 * space_size * sizeof(int));
    if (!derivative_sign) {
        report_error("Cannot allocate memory.");
    }

    /* Estimates marginal correlation of components for each binary classifier. */
    for (i = 0; i < space_size; ++i) {
        Real score_l, score_u;

        memcpy(sample_l, sample, space_size * sizeof(Real));
        memcpy(sample_u, sample, space_size * sizeof(Real));

        get_boundaries(sample_l + i, sample_u + i, adversarial_region, i);
        score_l = classifier_score(classifier, sample_l)[i];
        score_u = classifier_score(classifier, sample_u)[i];

        for (j = 0; j < n_classes * (n_classes - 1) / 2; ++j) {
            derivative_sign[j * space_size + i] = (score_l < score_u) ? 1 : -1;
        }
    }

    /* Searches for counterexamples in some the vertices. */
    for (j = 0; j < n_classes * (n_classes - 1) / 2; ++j) {
        for (i = 0; i < space_size; ++i) {
            if (derivative_sign[j * space_size + i] >= 0) {
                get_boundaries(sample_l + i, sample_u + i, adversarial_region, i);
            }
            else {
                get_boundaries(sample_l + i, sample_u + i, adversarial_region, i);
            }
        }

        if (is_counterexample(classifier, sample_l, sample_u)) {
            found = 1;
            break;
        }
    }

    free(derivative_sign);

    return found;
}
