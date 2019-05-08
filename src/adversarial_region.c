#include "adversarial_region.h"

#include <stdlib.h>

#include "report_error.h"


/***********************************************************************
 * Support functions.
 **********************************************************************/
unsigned int adversarial_region_contains_l_inf(
    const AdversarialRegion adversarial_region,
    const Real *point
) {
    const Real *original_sample = adversarial_region.sample;
    const Real magnitude = perturbation_get_magnitude(adversarial_region.perturbation);
    const unsigned int space_size = perturbation_get_space_size(adversarial_region.perturbation);
    unsigned int i;

    for (i = 0; i < space_size; ++i) {
        const Real min = original_sample[i] - magnitude >= 0.0 ? original_sample[i] - magnitude : 0.0,
                   max = original_sample[i] + magnitude <= 1.0 ? original_sample[i] + magnitude : 1.0;

        if (point[i] < min || point[i] > max) {
            return 0;
        }
    }

    return 1;
}



unsigned int adversarial_region_contains_hyper_rectangle(
    const AdversarialRegion adversarial_region,
    const Real *point
) {
    const Real *original_sample = adversarial_region.sample;
    const Perturbation perturbation = adversarial_region.perturbation;
    const unsigned int space_size = perturbation_get_space_size(perturbation);
    const Real *epsilon_l = perturbation_get_epsilon_lowerbounds(perturbation),
               *epsilon_u = perturbation_get_epsilon_upperbounds(perturbation);
    unsigned int i;

    for (i = 0; i < space_size; ++i) {
        const Real min = original_sample[i] - epsilon_l[i],
                   max = original_sample[i] + epsilon_u[i];

        if (point[i] < min || point[i] > max) {
            return 0;
        }
    }

    return 1;
}



static void adversarial_region_sample_l_inf(
    Real *sample,
    const AdversarialRegion adversarial_region
) {
    const Real *original_sample = adversarial_region.sample;
    const Real magnitude = perturbation_get_magnitude(adversarial_region.perturbation);
    const unsigned int space_size = perturbation_get_space_size(adversarial_region.perturbation);
    unsigned int i;

    for (i = 0; i < space_size; ++i) {
        const Real min = original_sample[i] - magnitude >= 0.0 ? original_sample[i] - magnitude : 0.0,
                   max = original_sample[i] + magnitude <= 1.0 ? original_sample[i] + magnitude : 1.0;

        sample[i] = ((Real) rand() / (Real) RAND_MAX) * (max - min) + min;
    }
}



static void adversarial_region_sample_hyper_rectangle(
    Real *sample,
    const AdversarialRegion adversarial_region
) {
    const Real *original_sample = adversarial_region.sample;
    const Perturbation perturbation = adversarial_region.perturbation;
    const unsigned int space_size = perturbation_get_space_size(perturbation);
    const Real *epsilon_l = perturbation_get_epsilon_lowerbounds(perturbation),
               *epsilon_u = perturbation_get_epsilon_upperbounds(perturbation);
    unsigned int i;

    for (i = 0; i < space_size; ++i) {
        const Real min = original_sample[i] - epsilon_l[i],
                   max = original_sample[i] + epsilon_u[i];

        sample[i] = ((Real) rand() / (Real) RAND_MAX) * (max - min) + min;
    }
}



/***********************************************************************
 * Public adversarial region functions.
 **********************************************************************/

unsigned int adversarial_region_contains(
    const AdversarialRegion adversarial_region,
    const Real *point
) {
    switch (perturbation_get_type(adversarial_region.perturbation)) {
        case PERTURBATION_L_INF:
            return adversarial_region_contains_l_inf(adversarial_region, point);

        case PERTURBATION_HYPER_RECTANGLE:
            return adversarial_region_contains_hyper_rectangle(adversarial_region, point);

        default:
            report_error("Unsupported type of adversarial region.");
    }
}



void adversarial_region_get_sample(
    Real *sample,
    const AdversarialRegion adversarial_region
) {
    switch (perturbation_get_type(adversarial_region.perturbation)) {
        case PERTURBATION_L_INF:
            adversarial_region_sample_l_inf(sample, adversarial_region);
            break;

        case PERTURBATION_HYPER_RECTANGLE:
            adversarial_region_sample_hyper_rectangle(sample, adversarial_region);
            break;

        default:
            report_error("Unsupported type of adversarial region.");
    }
}



void adversarial_region_print(
    const AdversarialRegion adversarial_region,
    FILE *stream
) {
    const unsigned int  space_size = perturbation_get_space_size(adversarial_region.perturbation);
    unsigned int i;

    if (space_size == 0) {
        fprintf(stream, "Adversarial region in a 0-dimensional space.\n");
        return;
    }

    fprintf(stream, "Adversarial region centered in:\n\t(");
    for (i = 0; i < space_size - 1; ++i) {
        fprintf(stream, "%f, ", adversarial_region.sample[i]);
    }
    fprintf(stream, "%f)\n",  adversarial_region.sample[space_size - 1]);
    fprintf(stream, "perturbed by:\n\t");
    perturbation_print(adversarial_region.perturbation, stream);
}
