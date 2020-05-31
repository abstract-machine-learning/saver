/**
 * SVM Abstract Verifier.
 * 
 * Verifies property of SVM classifiers using abstract interpretation.
 * 
 * @file saver.c
 * @author Marco Zanella <marco.zanella.1991@gmail.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "kernel.h"
#include "classifier.h"
#include "perturbation.h"
#include "stopwatch.h"
#include "options.h"
#include "dataset.h"
#include "abstract_domains/abstract_domain.h"
#include "abstract_domains/interval.h"
#include "abstract_classifiers/abstract_classifier.h"
#include "counterexamples/counterexample_seeker.h"


/**
 * Prints detailed debug information about analysis of a single sample.
 * 
 * @param[in] classifier          Concrete classifier
 * @param[in] abstract_classifier Abstract classifier
 * @param[in] sample              Concrete sample
 * @param[in] adversarial_region  Adversarial region
 * @note This is a debug function.
 */
void check_soundness(
    const Classifier classifier,
    const AbstractClassifier abstract_classifier,
    const Real *sample,
    const AdversarialRegion adversarial_region 
) {
    Real *scores = classifier_score(classifier, sample);
    Interval *abstract_scores = abstract_classifier_score(abstract_classifier, adversarial_region);
    unsigned int N = classifier_get_n_classes(classifier), i, j;

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            printf(
                "%u - %u:\t%f\t[%f, %f]\t%s\n",
                i,
                j,
                scores[index],
                abstract_scores[index].l,
                abstract_scores[index].u,
                (abstract_scores[index].l <= scores[index] && scores[index] <= abstract_scores[index].u) ? "sound" : "unsound"
            );
        }
    }
}


/**
 * Prints a help message.
 *
 * @param[in] argc ARGument Counter
 * @param[in] argv ARGument Vector
 */
static void display_help(const int argc, const char *argv[]) {
    (void) argc;

    fprintf(stderr, "Usage: %s <path to classifier> <path to dataset> [abstraction] [perturbation] [perturbation parameters] [--counterexamples_file <path>]\n", argv[0]);
    fprintf(stderr, "Detailed parameter description:\n");
    fprintf(stderr, "\t- <path to classifier>: Relative or absolute path to classifier file\n");
    fprintf(stderr, "\t- <path to dataset>: Relative or absolute path to dataset file (CSV)\n");
    fprintf(stderr, "\t- <abstraction>: Abstraction to use [interval|raf|hybrid] (default: %s)\n", DEFAULT_ABSTRACTION);
    fprintf(stderr, "\t\t* interval: Runs analysis using the interval abstract domain\n");
    fprintf(stderr, "\t\t* raf: Runs analysis using the Reduced Affine Forms (RAF)  abstract domain\n");
    fprintf(stderr, "\t\t* hybrid: Runs analysis using a mix of interval and RAF\n");
    fprintf(stderr, "\t- <perturbation>: Perturbation to use [l_inf|frame|from_file] (default: %s)\n", DEFAULT_PERTURBATION);
    fprintf(stderr, "\t- <perturbation parameters>: Additional parameters for the perturbation, depend on the perturbation:\n");
    fprintf(stderr, "\t\t* l_inf\n");
    fprintf(stderr, "\t\t\t- <epsilon>: Magnitude of the perturbation (default: %f)\n", DEFAULT_EPSILON);
    fprintf(stderr, "\t\t* frame\n");
    fprintf(stderr, "\t\t\t- <epsilon>: Magnitude of the perturbation (default: %f)\n", DEFAULT_EPSILON);
    fprintf(stderr, "\t\t\t- <width>: Image width\n");
    fprintf(stderr, "\t\t\t- <height>: Image height\n");
    fprintf(stderr, "\t\t\t- <frame width>: Width of the frame\n");
    fprintf(stderr, "\t\t\t- <frame height>: Width of the frame\n");
    fprintf(stderr, "\t\t* from_file\n");
    fprintf(stderr, "\t\t\t- <file path>: Perturbation file path\n");
    fprintf(stderr, "\t- --counterexamples_file <path>: If specified, searches counterexamples of non-robust cases and stores them in <path>\n");
    fprintf(stderr, "\t- --debug_output: Enables verbose, debug output\n");
}




/**
 * Prints a list of classes to standard output.
 * 
 * @param[in] classes Classes to print
 * @param[in] n       Number of classes to print
 */
static void print_classes(char **classes, const unsigned int n) {
    unsigned int i;

    for (i = 0; i < n - 1; ++i) {
        printf("%s,", classes[i]);
    }
    printf("%s", classes[i]);
}



/**
 * Main.
 * 
 * @param[in] argc ARGument Counter
 * @param[in] argv ARGument Vector
 * @return EXIT_SUCCESS in case of success, EXIT_FAILURE otherwise
 */
int main(const int argc, const char **argv) {
    FILE *classifier_file, *dataset_file;
    Classifier classifier;
    AbstractClassifier abstract_classifier;
    Dataset dataset;
    Stopwatch stopwatch;
    unsigned int i, robust_cases = 0, correct_cases = 0, conditionally_robust_cases = 0, counterexamples_found = 0;
    Real epsilon = 0.01;
    char **classes, **abstract_classes;
    Perturbation perturbation;
    Options options;
    CounterexampleSeeker counterexample_seeker;
    Counterexample counterexample;


    /* Input check */
    if (argc < 3) {
        display_help(argc, argv);
        exit(EXIT_FAILURE);
    }

    /* Reads classifier */
    classifier_file = fopen(argv[1], "r");
    classifier = classifier_read(classifier_file);
    fclose(classifier_file);

    /* Reads dataset */
    dataset_file = fopen(argv[2], "r");
    dataset = dataset_read(dataset_file);
    fclose(dataset_file);

    /* Reads perturbation */
    perturbation_read(&perturbation, argc - 4, argv + 4, dataset_get_space_size(dataset));
    epsilon = perturbation_get_magnitude(perturbation);

    /* Reads additional options. */
    read_options(&options, argc, argv);

    /* Builds abstract classifier */
    abstract_classifier = abstract_classifier_read(classifier, argc - 3, argv + 3);

    /* Builds counterexample seeker */
    counterexample_seeker_create(&counterexample_seeker, abstract_classifier, COUNTEREXAMPLE_ROBUSTNESS);
    counterexample_create(&counterexample, dataset_get_space_size(dataset), 2);

    /* Allocates memory */
    classes = (char **) malloc(classifier_get_n_classes(classifier) * sizeof(char *));
    abstract_classes = (char **) malloc(classifier_get_n_classes(classifier) * sizeof(char *));


    /* Prints heading */
    printf("Classifier\tDtaset\tID\tEpsilon\tLabel\tConcrete\tAbstract");
    if (options.counterexamples_file) {
        printf("\tCounterexample");
    }
    printf("\n");

    stopwatch = stopwatch_create();
    stopwatch_start(stopwatch);
    for (i = 0; i < dataset_get_size(dataset); ++i) {
        const Real *sample = dataset_get_row(dataset, i);
        const AdversarialRegion adversarial_region = {sample, perturbation};
        unsigned int n_concrete_classes, n_abstract_classes, is_correct, is_robust, has_counterexample = 0;

        /* Runs concrete and abstract classifiers */
        n_concrete_classes = classifier_classify(classifier, sample, classes);
        n_abstract_classes = abstract_classifier_classify(abstract_classifier, adversarial_region, abstract_classes);

        /* Compute metrics */
        is_correct = n_concrete_classes == 1 && strcmp(classes[0], dataset_get_label(dataset, i)) == 0;
        is_robust = n_abstract_classes <= n_concrete_classes;
        correct_cases += is_correct;
        robust_cases += is_robust;
        conditionally_robust_cases += is_correct && is_robust;

        /* Searches for counterexamples */
        if (!is_robust && options.counterexamples_file) {
            has_counterexample = counterexample_seeker_search(counterexample, counterexample_seeker, adversarial_region);
            counterexamples_found += has_counterexample;
        }

        /* Prints results */
        printf("%s\t%s\t%u\t%f\t%s\t", argv[1], argv[2], i, epsilon, dataset_get_label(dataset, i));
        print_classes(classes, n_concrete_classes);
        printf("\t");
        print_classes(abstract_classes, n_abstract_classes);
        if (options.counterexamples_file) {
            printf("\t%s", is_robust ? "NONE" : (has_counterexample ? "FOUND" : "NOT-FOUND"));
        }
        printf("\n");

        /* Debug information */
        if (options.debug_output) {
            check_soundness(classifier, abstract_classifier, sample, adversarial_region);
        }
    }
    stopwatch_stop(stopwatch);


    /* Writes summary */
    printf("[SUMMARY]\tSize\tEpsilon\tAvg. Time (ms)\tCorrect\tRobust\tCond. robust");
    if (options.counterexamples_file) {
        printf("\tCounterexamples");
    }
    printf("\n");
    printf(
        "[SUMMARY]\t%u\t %g\t %f\t %u\t %u\t %u",
        dataset_get_size(dataset),
        epsilon,
        stopwatch_get_elapsed_milliseconds(stopwatch) / dataset_get_size(dataset),
        correct_cases,
        robust_cases,
        conditionally_robust_cases
    );
    if (options.counterexamples_file) {
        printf("\t %u", counterexamples_found);
    }
    printf("\n");



    /* Deallocates memory */
    classifier_delete(&classifier);
    perturbation_delete(&perturbation);
    dataset_delete(&dataset);
    stopwatch_delete(&stopwatch);
    abstract_classifier_delete(&abstract_classifier);
    free(classes);
    free(abstract_classes);
    counterexample_seeker_delete(&counterexample_seeker);
    counterexample_delete(&counterexample);

    return EXIT_SUCCESS;
}
