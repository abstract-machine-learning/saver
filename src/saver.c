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
#include <stdbool.h>

#include "kernel.h"
#include "classifier.h"
#include "perturbation.h"
#include "tier.h"
#include "stopwatch.h"
#include "options.h"
#include "dataset.h"
#include "abstract_domains/abstract_domain.h"
#include "abstract_domains/interval.h"
#include "abstract_classifiers/abstract_classifier.h"
#include "counterexamples/counterexample_seeker.h"

#define SAVER_VERSION "v1.0"


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
    const AdversarialRegion adversarial_region,
    bool* fair_opt,
    unsigned int* has_counterexample 
) {
    Real *scores = classifier_score(classifier, sample);
    Interval *abstract_scores = abstract_classifier_score(abstract_classifier, adversarial_region,fair_opt,has_counterexample);
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

    fprintf(stderr, "SAVer %s\n", SAVER_VERSION);
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

void print_classifier(const Classifier classifier,FILE *resultFile)
{
    char *row1[] = {"SVM PARAM", "CType", "KType", "Gamma", "Degree", "Coeff.","SpaceSize"};
    printf("------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"------------------------------------------------------------------------------------------------------------\n");

    printf("| %6s\t|| %6s\t| %6s\t| %6s\t| %6s\t| %6s\t| %8s|\n",row1[0],row1[1],row1[2],row1[3],row1[4],row1[5],row1[6]);
    fprintf(resultFile,"| %6s\t|| %6s\t| %6s\t| %6s\t| %6s\t| %6s\t| %8s|\n",row1[0],row1[1],row1[2],row1[3],row1[4],row1[5],row1[6]);
    printf("------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"\n");
    printf(
        "| %6s\t|| %6s\t| %6s\t| %6f\t| %6d\t| %6f\t| %8d|\n",
        row1[0],
        classifierTypeStr(classifier_get_type(classifier)),
        kernelTypeStr(kernel_get_type(classifier_get_kernel(classifier))),
        kernel_get_gamma(classifier_get_kernel(classifier)),
        kernel_get_degree(classifier_get_kernel(classifier)),
        kernel_get_c(classifier_get_kernel(classifier)),
        classifier_get_space_size(classifier)

    );
    fprintf(resultFile,
        "| %6s\t|| %6s\t| %6s\t| %6f\t| %6d\t| %6f\t| %8d|\n",
        row1[0],
        classifierTypeStr(classifier_get_type(classifier)),
        kernelTypeStr(kernel_get_type(classifier_get_kernel(classifier))),
        kernel_get_gamma(classifier_get_kernel(classifier)),
        kernel_get_degree(classifier_get_kernel(classifier)),
        kernel_get_c(classifier_get_kernel(classifier)),
        classifier_get_space_size(classifier)
    );
    printf("------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"------------------------------------------------------------------------------------------------------------\n");

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
    unsigned int i, robust_cases = 0, correct_cases = 0, conditionally_robust_cases = 0, counterexamples_found = 0, counterRegion_found = 0;
    Real epsilon = 0.01;
    char **classes, **abstract_classes;
    Perturbation perturbation;
    Tier tier;
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

    /* Reads Tier */
    //tier_create(&tier, dataset_get_space_size(dataset));
    tier_read(&tier, argv[6], dataset_get_space_size(dataset));

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

    /* if binary classier then calculate confusion matrix */
    bool is_binary = (atoi(argv[7]) == 1);

    bool* fair_opt = (bool*)malloc(4*sizeof(bool));

    /*Top region for finding the weights for each feature*/
    fair_opt[0] = (atoi(argv[8]) == 1);    // is Top

    /*Use OH abstraction for OH encoded features*/
    fair_opt[1] = (atoi(argv[9]) == 1);    // if OH

    /*Alternative implementation of RAF OH to get counter example*/
    fair_opt[2] = (atoi(argv[10]) == 1);   // if get CE

    /*If to paritition input space to get additional counterexample and subregion*/
    fair_opt[3] = (atoi(argv[11]) == 1);       // if paratition

    if(fair_opt[0])
    {
        if(strcmp(argv[3], "raf") != 0)
        {
            printf("Feature analysis requires RAF\n");
            exit(0);
        }
        if(!is_binary)
        {
            printf("Non Implemented for Muticlass classifier, yet.\n");
            exit(0);
        }
        const Real *sample = dataset_get_row(dataset, i);
        const AdversarialRegion adversarial_region = {sample, perturbation,tier};
        unsigned int dummy = 0;
        abstract_classifier_classify(abstract_classifier, adversarial_region, abstract_classes,fair_opt,&dummy);
        return 0;
    }


    /* Prints heading */
    printf("Classifier\tDtaset\tID\tEpsilon\tLabel\tConcrete\tAbstract\n");
    if (options.counterexamples_file) {
        printf("\tCounterexample");
    }

    stopwatch = stopwatch_create();
    stopwatch_start(stopwatch);
  
    /*variables for confusion matrix*/
    int TP = 0,FP = 0,TN = 0,FN = 0;    
    int* labels = malloc(2*sizeof(int));
    if(is_binary)
        dataset_get_unique_labels(labels,dataset);
    //#pragma omp parallel for
    for (i = 0; i < dataset_get_size(dataset); ++i) {
        const Real *sample = dataset_get_row(dataset, i);
        const AdversarialRegion adversarial_region = {sample, perturbation,tier};
        unsigned int n_concrete_classes, n_abstract_classes, is_correct, is_robust;
        unsigned int *has_counterexample = (unsigned int *)calloc(2,sizeof(unsigned int));

        /* Runs concrete and abstract classifiers */
        n_concrete_classes = classifier_classify(classifier, sample, classes);
        n_abstract_classes = abstract_classifier_classify(abstract_classifier, adversarial_region, abstract_classes,fair_opt,has_counterexample);
        /* Compute metrics */
        is_correct = n_concrete_classes == 1 && strcmp(classes[0], dataset_get_label(dataset, i)) == 0;
        is_robust = n_abstract_classes <= n_concrete_classes;
        
        if(is_binary && n_concrete_classes == 1)
        {
            if(strcmp(dataset_get_label(dataset, labels[0]), classes[0]) == 0)
            {
                if(is_correct)
                    TP++;
                else
                    FP++;
            }
            else if(strcmp(dataset_get_label(dataset, labels[1]),classes[0]) == 0)
                {
                    if(is_correct)
                        TN++;
                    else
                        FN++;
                }

        }
        correct_cases += is_correct;
        robust_cases += is_robust;
        conditionally_robust_cases += is_correct && is_robust;

        /* Searches for counterexamples
        if (!is_robust && options.counterexamples_file) {
            has_counterexample = counterexample_seeker_search(counterexample, counterexample_seeker, adversarial_region);
            counterexamples_found += has_counterexample;
        } */

        counterexamples_found += has_counterexample[0];
        counterRegion_found += has_counterexample[1];
        if(is_robust && has_counterexample[0])
        {
            printf("\n\n\n\n There must never be a counterexample for robust perturbations \n\n\n\n");
            exit(0);
        }

        if (options.debug_output) {
            check_soundness(classifier, abstract_classifier, sample, adversarial_region,fair_opt,has_counterexample);
        }
    }
    printf("\n");
    printf("counterRegion: %d\n", counterRegion_found);
    double balanced_accuracy;
    if(is_binary)
    {balanced_accuracy = 50 * ( ( (TP*1.0)/(TP+FN) ) + ( (TN*1.0)/(TN+FP) )) ;}
    double accuracy = correct_cases*100.0/dataset_get_size(dataset);
    double robust_percent = robust_cases*100.0/dataset_get_size(dataset);

    stopwatch_stop(stopwatch);

    /* Append results to a output file*/
    FILE *resultFile;
    resultFile = fopen("result1.txt", "a");
    FILE *resultRawFile;
    resultRawFile = fopen("result_raw.txt", "a");
    fprintf(resultFile,"\n\n\t--------- Begin New Result --------\nArguments:\n");
    char *param_name[] = {"SVM Path","Data Path", "Abstraction", "Perturbation", "Perturbation Value/Path", "Tier Path", "Is binary"};
    /*Output the arguments*/

    for(int i = 1; i< argc-4;i++)
    {
        fprintf(resultFile,"%d) %s : %s\n",i,param_name[i-1],argv[i]);
    }
    fprintf(resultFile,"\n\n");

    fprintf(resultFile,"\n");
    printf("\n");

    print_classifier(classifier,resultFile);
    float ce = (dataset_get_size(dataset)*1.0 - counterexamples_found*1.0)/dataset_get_size(dataset)*100.0;

    /* Writes summary */
    printf("OH on: %d Bal. Acc.: %f; Acc: %f; Robust -> LB: %f UB: %f \n\n",fair_opt[1],balanced_accuracy,accuracy,robust_percent,ce);
    fprintf(resultFile,"OH on: %d WITH OH epsilon: 0.05; Bal. Acc.: %f; Acc: %f; Robust: %f CE: %f \n\n",fair_opt[1],balanced_accuracy,accuracy,robust_percent,ce);

    printf("------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"------------------------------------------------------------------------------------------------------------\n");
    
    char *row2[] = {"[SUMMARY]", "Size", "Epsilon", "Avg. Time (ms)", "Correct", "Robust","Cond. robust","Bal. Acc.","Counterexamples"};
    printf("| %6s\t|| %6s\t| %6s\t| %8s\t| %6s\t| %6s\t| %8s\t| ",row2[0],row2[1],row2[2],row2[3],row2[4],row2[5],row2[6]);
    fprintf(resultFile,"| %6s\t|| %6s\t| %6s\t| %8s\t| %6s\t| %6s\t| %8s\t| ",row2[0],row2[1],row2[2],row2[3],row2[4],row2[5],row2[6]);
    
    if (is_binary) {
        printf("%8s\t|",row2[7]);
        fprintf(resultFile,"%8s\t|",row2[7]);
    }
    if (true) {
        printf("%8s\t|",row2[8]);
        fprintf(resultFile,"%8s\t|",row2[8]);
    }
    printf("\n------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"\n------------------------------------------------------------------------------------------------------------\n");
    double time = stopwatch_get_elapsed_milliseconds(stopwatch) / dataset_get_size(dataset);
    printf(
        "| %6s\t| %6u\t|  %6g\t|  %8f\t|  %6u\t|  %6u\t|  %8u\t|  ",
        row2[0],
        dataset_get_size(dataset),
        epsilon,
        time,
        correct_cases,
        robust_cases,
        conditionally_robust_cases
    );
    fprintf(resultFile,"| %6s\t| %6u\t|  %6g\t|  %8f\t|  %6u\t|  %6u\t|  %8u\t|  ",
        row2[0],
        dataset_get_size(dataset),
        epsilon,
        time,
        correct_cases,
        robust_cases,
        conditionally_robust_cases
    );
    int dataset_size = dataset_get_size(dataset);
    
    fprintf(resultRawFile,"%f %f %f\n",(correct_cases*100.0/dataset_size),balanced_accuracy,(robust_cases*100.0/dataset_size));

    if (is_binary) {
        printf("%8f\t|",balanced_accuracy);
        fprintf(resultFile,"%8f\t|",balanced_accuracy);
    }
    if (true) {
        printf("%8u\t| ", counterexamples_found);
        fprintf(resultFile,"%8u\t| ", counterexamples_found);
    }
    printf("\n");
    fprintf(resultFile,"\n");
    printf("------------------------------------------------------------------------------------------------------------\n");
    fprintf(resultFile,"------------------------------------------------------------------------------------------------------------\n");



    /* Deallocates memory */
    classifier_delete(&classifier);
    perturbation_delete(&perturbation);
    dataset_delete(&dataset);
    stopwatch_delete(&stopwatch);
    abstract_classifier_delete(&abstract_classifier);
    free(classes);
    free(abstract_classes);
    free(labels);
    tier_delete(&tier);
    counterexample_seeker_delete(&counterexample_seeker);
    counterexample_delete(&counterexample);

    return EXIT_SUCCESS;
}
