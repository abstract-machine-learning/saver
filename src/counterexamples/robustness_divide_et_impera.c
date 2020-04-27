#include "robustness_divide_et_impera.h"

#include <stdio.h>
#include <string.h>

#include "../report_error.h"
#include "counterexample_seeker.h"
#include "robustness_vertex_heuristic.h"

/***********************************************************************
 * Internal support functions.
 **********************************************************************/

static unsigned int classes_equal(
    const char **classes_a,
    const unsigned int n_classes_a,
    const char **classes_b,
    const unsigned int n_classes_b
) {
    unsigned int i;

    if (n_classes_a != n_classes_b) {
        return 0;
    }

    for (i = 0; i < n_classes_a; ++i) {
        if (classes_a[i] != classes_b[i]) {
            return 0;
        }
    }

    return 1;
}




static void perturbation_to_hyper_rectangle(
    Perturbation dst,
    const Perturbation src
) {
    const unsigned int space_size = perturbation_get_space_size(src);
    unsigned int i;

    switch (perturbation_get_type(src)) {
        case PERTURBATION_L_INF:
            for (i = 0; i < space_size; ++i) {
                perturbation_get_epsilon_lowerbounds(dst)[i] = perturbation_get_magnitude(src);
                perturbation_get_epsilon_upperbounds(dst)[i] = perturbation_get_magnitude(src);
            }
            break;

        case PERTURBATION_HYPER_RECTANGLE:
            perturbation_copy(dst, src);
            break;

        default:
            report_error("Unsupported type of perturbation.");
    }
}



static void hyper_rectangle_scale(Perturbation perturbation, const Real s) {
    const unsigned int space_size = perturbation_get_space_size(perturbation);
    unsigned int i;

    for (i = 0; i < space_size; ++i) {
        perturbation_get_epsilon_lowerbounds(perturbation)[i] *= s;
        perturbation_get_epsilon_upperbounds(perturbation)[i] *= s;
    }
}


static void find_max_hyper_rectangle(
    AdversarialRegion *dst,
    const AbstractClassifier abstract_classifier,
    char **classes_buffer
) {
    const unsigned int n_classes = abstract_classifier_classify(abstract_classifier, *dst, classes_buffer);

    if (n_classes == 1) {
        return;
    }

    hyper_rectangle_scale(dst->perturbation, 0.5);
    find_max_hyper_rectangle(dst, abstract_classifier, classes_buffer);
}





static unsigned int divide_et_impera(
    Counterexample counterexample,
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion original_region
);







static CounterExampleSeekerOutput analyze_region(
    Counterexample counterexample,
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion region,
    const char **original_concrete_classes,
    const unsigned int n_original_concrete_classes
) {
    const Classifier classifier = abstract_classifier_get_classifier(abstract_classifier);
    const unsigned int n_classes = classifier_get_n_classes(classifier);

    unsigned int n_concrete_classes, n_abstract_classes;
    char **concrete_classes = (char **) malloc(n_classes * sizeof(char *)), 
         **abstract_classes = (char **) malloc(n_classes * sizeof(char *));
    CounterExampleSeekerOutput result;


    n_concrete_classes = classifier_classify(classifier, region.sample, concrete_classes);
    n_abstract_classes = abstract_classifier_classify(
        abstract_classifier,
        region,
        abstract_classes
    );

printf("\nSubregion analysis.\n");
    if (n_concrete_classes == n_abstract_classes) {
printf("\tsubregion is robust\n");
        if (classes_equal(original_concrete_classes, n_original_concrete_classes, (const char **) abstract_classes, n_abstract_classes)) {
printf("\t\tand has the same classes as inner region\n");
            result = COUNTEREXAMPLE_SEEKER_ROBUST;
        }
        else {
printf("\t\tand has NOT the same classes as inner region: COUNTEREXAMPLE\n");
            adversarial_region_get_sample(counterexample_get_nth_sample(counterexample, 1), region);
            result = COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE;
        }
    }
    else {

unsigned int found = robustness_vertex_heuristic(
                counterexample,
                classifier,
                region
            );
printf("Counterexample: %u\n", found);
result = found ? COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE : COUNTEREXAMPLE_SEEKER_DONT_KNOW;



/*
printf("\tSubregion is not robust\n");
        const CounterExampleSeekerOutput region_result = divide_et_impera(
            counterexample,
            abstract_classifier,
            region
        );
        if (region_result == COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE) {
printf("\t\tfound a counterexample\n");
            result = COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE;
        }
        else if (region_result == COUNTEREXAMPLE_SEEKER_ROBUST) {
printf("\t\tsubregion was robust, afer all\n");
            adversarial_region_get_sample(counterexample_get_nth_sample(counterexample, 1), region);
            n_concrete_classes = classifier_classify(classifier, counterexample_get_nth_sample(counterexample, 1), concrete_classes);
            if (classes_equal(original_concrete_classes, n_original_concrete_classes, (const char **) concrete_classes, n_concrete_classes)) {
printf("\t\t\tand with same classes of inner region\n");
                result = COUNTEREXAMPLE_SEEKER_ROBUST;
            }
            else {
printf("\t\t\tand with different classes wrt inner region: FOUND counterexample\n");
                result = COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE;
            }
        }
        else {
printf("\t\tfound a DONT KNOW\n");
            result = COUNTEREXAMPLE_SEEKER_DONT_KNOW;
        }
*/
    }

    free(concrete_classes);
    free(abstract_classes);

    return result;
}



static unsigned int divide_et_impera(
    Counterexample counterexample,
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion original_region
) {
    const Classifier classifier = abstract_classifier_get_classifier(abstract_classifier);
    const unsigned int space_size = classifier_get_space_size(classifier),
                       n_classes = classifier_get_n_classes(classifier);
    char **concrete_classes = (char **) malloc(n_classes * sizeof(char *)),
         **abstract_classes = (char **) malloc(n_classes * sizeof(char *));
    unsigned int i, n_concrete_classes, n_abstract_classes;


printf("\n---recursion\n");
    // Terminates if manages to prove robustness
    n_concrete_classes = classifier_classify(classifier, original_region.sample, concrete_classes),
    n_abstract_classes = abstract_classifier_classify(abstract_classifier, original_region, abstract_classes);
    if (n_concrete_classes == n_abstract_classes) {
printf("robust!\n\n");
exit(0);
        free(concrete_classes);
        free(abstract_classes);
        return COUNTEREXAMPLE_SEEKER_ROBUST;
    }
printf("not robust!\n");


    
    // Finds maximal robust concentric subregion
    Perturbation max_perturbation;
    perturbation_create_hyper_rectangle(&max_perturbation, space_size);
    perturbation_copy(max_perturbation, original_region.perturbation);
    AdversarialRegion max_region = {original_region.sample, max_perturbation};
    find_max_hyper_rectangle(&max_region, abstract_classifier, abstract_classes);


    // Examines every outer region
    Real *outer_sample = (Real *) malloc(space_size * sizeof(Real));
    Perturbation outer_perturbation;
    perturbation_create_hyper_rectangle(&outer_perturbation, space_size);
    CounterExampleSeekerOutput result = COUNTEREXAMPLE_SEEKER_ROBUST;

    for (i = 0; i < space_size; ++i) {
        if (result == COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE) {
            break;
        }
printf("examining region %u\n", i);

        memcpy(outer_sample, original_region.sample, space_size * sizeof(Real));
        perturbation_copy(outer_perturbation, original_region.perturbation);

        const Real *epsilon_l = perturbation_get_epsilon_lowerbounds(original_region.perturbation),
                   *epsilon_u = perturbation_get_epsilon_upperbounds(original_region.perturbation),
                   *epsilon2_l = perturbation_get_epsilon_lowerbounds(max_region.perturbation),
                   *epsilon2_u = perturbation_get_epsilon_upperbounds(max_region.perturbation);
(void) epsilon_l;
(void) epsilon2_l;

        outer_sample[i] = original_region.sample[i] + 0.5 * (epsilon_u[i] + epsilon2_u[i]);
        perturbation_get_epsilon_lowerbounds(outer_perturbation)[i] = 0.5 * (epsilon_u[i] - epsilon2_u[i]);
        AdversarialRegion upper_region = {outer_sample, outer_perturbation};

        switch (analyze_region(counterexample, abstract_classifier, upper_region, (const char **) concrete_classes, n_concrete_classes)) {
            case COUNTEREXAMPLE_SEEKER_ROBUST:
printf("subregion robust\n");
                break;

            case COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE:
                result = COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE;
printf("counterexample\n");
                break;

            case COUNTEREXAMPLE_SEEKER_DONT_KNOW:
printf("don't know\n");
                result = COUNTEREXAMPLE_SEEKER_DONT_KNOW;
                break;
        }

/* TODO: specular to upper case
        AdversarialRegion lower_region;
        switch (analyze_region(counterexample, abstract_classifier, lower_region, (const char **) concrete_classes, n_concrete_classes)) {
            case COUNTEREXAMPLE_SEEKER_ROBUST:
                break;

            case COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE:
                return COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE;

            case COUNTEREXAMPLE_SEEKER_DONT_KNOW:
                possibly_robust = 0;
                break;
        }
*/
    }


    free(concrete_classes);
    free(abstract_classes);
    free(outer_sample);
    perturbation_delete(&max_perturbation);
    perturbation_delete(&outer_perturbation);

    return result;
}









/***********************************************************************
 * Public functions.
 **********************************************************************/

unsigned int robustness_divide_et_impera(
    Counterexample counterexample,
    const AbstractClassifier abstract_classifier,
    const AdversarialRegion adversarial_region
) {
    const unsigned int space_size = perturbation_get_space_size(adversarial_region.perturbation);
    Perturbation original_perturbation;


    // Converts perturbation to hyper rectangle
    perturbation_create_hyper_rectangle(&original_perturbation, space_size);
    perturbation_to_hyper_rectangle(original_perturbation, adversarial_region.perturbation);


    // Builds original adversarial region (as hyper rectangle)
    const AdversarialRegion original_region = {adversarial_region.sample, original_perturbation};


    // Starts the actual procedure
    const unsigned int result = divide_et_impera(
        counterexample,
        abstract_classifier,
        original_region
    );



    // Deallocates memory
    perturbation_delete(&original_perturbation);


return result == COUNTEREXAMPLE_SEEKER_COUNTEREXAMPLE;

/*
    const unsigned int space_size = perturbation_get_space_size(adversarial_region.perturbation),
                       n_classes = classifier_get_n_classes(abstract_classifier_get_classifier(abstract_classifier));
    const Real *sample = adversarial_region.sample;
    Perturbation perturbation;
    char **classes_buffer = (char **) malloc(n_classes * sizeof(char *));
    unsigned int i;

    Perturbation original_perturbation;
    perturbation_create_hyper_rectangle(&original_perturbation, space_size);
    perturbation_to_hyper_rectangle(original_perturbation, adversarial_region.perturbation);



    // Finds maximal robust adversarial region
    perturbation_create_hyper_rectangle(&perturbation, space_size);
    perturbation_to_hyper_rectangle(perturbation, adversarial_region.perturbation);
    AdversarialRegion max_region = {sample, perturbation};
    find_max_hyper_rectangle(&max_region, abstract_classifier, classes_buffer);
printf("Class label on max rectangle: %s\n", classes_buffer[0]);


    // Explores outer regions
    Real *outer_sample = (Real *) malloc(space_size * sizeof(Real));
    Perturbation outer_perturbation;
    perturbation_create_hyper_rectangle(&outer_perturbation, space_size);
    //for (i = 0; i < space_size; ++i) {
    for (i = 0; i < space_size; ++i) {
        memcpy(outer_sample, sample, space_size * sizeof(Real));
        perturbation_copy(outer_perturbation, adversarial_region.perturbation);

        const Real *epsilon_l = perturbation_get_epsilon_lowerbounds(original_perturbation),
                   *epsilon_u = perturbation_get_epsilon_upperbounds(original_perturbation),
                   *epsilon2_l = perturbation_get_epsilon_lowerbounds(max_region.perturbation),
                   *epsilon2_u = perturbation_get_epsilon_upperbounds(max_region.perturbation);
        unsigned int n_classes;

        // up!
        outer_sample[i] = sample[i] + 0.5 * (epsilon_u[i] + epsilon2_u[i]);
printf("outer_sample[%u]: %f\n", i, outer_sample[i]);
        perturbation_get_epsilon_lowerbounds(outer_perturbation)[i] = 0.5 * (epsilon_u[i] - epsilon2_u[i]);
        AdversarialRegion up = {outer_sample, outer_perturbation};

        n_classes = abstract_classifier_classify(abstract_classifier, up, classes_buffer);
printf("Running on outer region %u. n classes: %u:  ", i, n_classes);
unsigned int j;
for (j = 0; j < n_classes; ++j) {
    printf("%s  ", classes_buffer[j]);
}
printf("\n");





if (n_classes > 1) {
static unsigned int depth = 0;

printf("RECURSION   %u----------------------------------------------------\n", depth);
++depth;
    robustness_divide_et_impera(
        counterexample,
        abstract_classifier,
        up
    );
exit(0);
--depth;
}




        if (n_classes > 2) {
            unsigned int found = robustness_vertex_heuristic(
                counterexample,
                abstract_classifier_get_classifier(abstract_classifier),
                up 
            );
printf("Counterexample: %u\n", found);
if (found) {
    classifier_classify(abstract_classifier_get_classifier(abstract_classifier), counterexample_get_nth_sample(counterexample, 0), classes_buffer);
    printf("%s - ", classes_buffer[0]);
    classifier_classify(abstract_classifier_get_classifier(abstract_classifier), counterexample_get_nth_sample(counterexample, 1), classes_buffer);
    printf("%s\n\n", classes_buffer[0]);

printf("original in adv region? %u\n", adversarial_region_contains(adversarial_region, outer_sample));
printf("first counterexample in adv region? %u\n", adversarial_region_contains(adversarial_region, counterexample_get_nth_sample(counterexample, 0)));
printf("second counterexample in adv region? %u\n", adversarial_region_contains(adversarial_region, counterexample_get_nth_sample(counterexample, 1)));

return 1;
}
        }

*/
/*
        if (n_classes > 2) {
            const unsigned int n_classes_c = classifier_classify(
                abstract_classifier_get_classifier(abstract_classifier),
                outer_sample,
                classes_buffer
            );
printf("Running naive heuristic. n classes: %u: ", n_classes_c);
for (j = 0; j < n_classes_c; ++j) {
    printf("%s  ", classes_buffer[j]);
}
printf("\n\n");
        }
*/
/*
    }


    



    perturbation_delete(&perturbation);
    perturbation_delete(&outer_perturbation);
    free(classes_buffer);
    free(outer_sample);
    (void) counterexample;
    (void) abstract_classifier;
    (void) adversarial_region;
    return 0;
*/
}
