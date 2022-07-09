#include "raf_classifier.h"

#include <malloc.h>
#include "../report_error.h"
#include "../tier.h"
#include "../abstract_domains/one_hot_raf.h"

/* Use RAF with OH abstraction */
//#define ONE_HOT_ON
/* Use alternative methord for OH abstraction to facitilate counterexamples */
//#define OH_AT_LAST
/* Partition and rerun to find additional counterexample and counter sub-regions */
//#define RUN_PARTITION false
/**
 * Structure of a RAF classifier.
 */
struct raf_classifier {
    Classifier classifier;  /**< Concrete classifier. */
    Interval *buffer;       /**< Internal buffer. */
};

/**
 * Obtain a single sum RAF for a set of RAF one for each tier.
 * Apply OH gamma and find min and max to obtain RAF of tier given
 * OH in RAF form for each feature in the tier.
 */
static void tier_aware_sum(
    Raf *r,
    const bool *isOneHot,
    const Tier tier,
    const Raf *feature,
    const short* origins,
    const unsigned int space_size
)
{
    Raf *tierRaf = (Raf *)malloc(sizeof(Raf));
    raf_create(tierRaf, feature[0].size);
    for(unsigned int i = 0;i < space_size; i++){

        if(isOneHot[i]){
            unsigned int j;
            for(j = i;j <space_size;j++){
                if(tier.tiers[i] != tier.tiers[j])
                    break;
            }
            if(j==i+1){
                raf_fma_in_place(r, 1, feature[i]);
            }
            else{
                ohraf_Rafize(tierRaf,&feature[i] ,&origins[i], (j-i));
                raf_fma_in_place(r, 1, *tierRaf);
                i = j-1;
            }
        }
        else
        {
            raf_fma_in_place(r, 1, feature[i]);
        }
    }
    raf_delete(tierRaf);
    free(tierRaf);

}

/*Apply kernel without OH abstraction*/
static void raf_kernel(
    Raf *r,
    const Kernel kernel,
    const Real *x,
    const Raf *y,
    const unsigned int space_size
) {
    if (kernel_get_type(kernel) == KERNEL_RBF) {
        Raf exponent, product;
        unsigned int i;

        raf_create(&exponent, space_size);
        raf_create(&product, y[0].size);

        for (i = 0; i < space_size; ++i) {
            raf_translate(&product, y[i], -x[i]);
            raf_sqr(&product, product);
            raf_add_sparse_in_place(&exponent, product);
        }
        raf_scale(&exponent, exponent, -kernel_get_gamma(kernel));
        raf_exp(r, exponent);
        raf_delete(&exponent);
        raf_delete(&product);
    }

    else if (kernel_get_type(kernel) == KERNEL_POLYNOMIAL) {
        unsigned int i;
        Raf product;

        raf_create(&product, space_size);
        raf_translate_in_place(&product, kernel_get_c(kernel));

        for (i = 0; i < space_size; ++i) {
            raf_fma_in_place(&product, x[i], y[i]);
        }
    /*
    printf("SUM) -> (%f + ",product.c);
    for(unsigned int i = 0; i< product.size;i++)
        printf("%fe +",product.noise[i]);
    printf(")\n");
    */
        raf_pow(r, product, kernel_get_degree(kernel));
        raf_delete(&product);
    }

    else {
        report_error("Unsupported kernel type.");
    }
}

/*Apply Alternative methord of kernel with OH abstraction to allow counterexample*/
static void raf_kernelOHLast(
    Raf *r,
    Tier tier,
    const Kernel kernel,
    const Real *x,
    const Raf *y,
    const unsigned int space_size
) {
    bool* isOneHot = (bool*)malloc(space_size*sizeof(bool));
    fill_isOneHot(isOneHot,tier);
    if (kernel_get_type(kernel) == KERNEL_RBF) {
        Raf exponent, product;
        unsigned int i;

        raf_create(&exponent, space_size);
        raf_create(&product, y[0].size);

        for (i = 0; i < space_size; ++i) {
            if(isOneHot[i])
            {
                ohraf_translate(&product, y[i], -x[i]);
                ohraf_pow(&product, product, 2);
            }
            else
            {
                raf_translate(&product, y[i], -x[i]);
                raf_sqr(&product, product);
            }
            raf_add_sparse_in_place(&exponent, product);
        }
        raf_scale(&exponent, exponent, -kernel_get_gamma(kernel));
        raf_exp(r, exponent);
        raf_delete(&exponent);
        raf_delete(&product);
    }

    else if (kernel_get_type(kernel) == KERNEL_POLYNOMIAL) {
        unsigned int i;
        Raf product;
        Raf scaled;

        raf_create(&product, space_size);
        raf_translate_in_place(&product, kernel_get_c(kernel));
        raf_create(&scaled, space_size);

        for (i = 0; i < space_size; ++i) {
            if(isOneHot[i])
                ohraf_scale(&scaled,y[i],x[i]);
            else
                raf_scale(&scaled,y[i],x[i]);
            
            raf_fma_in_place(&product, 1, scaled);
        }
    /*
    printf("SUM) -> (%f + ",product.c);
    for(unsigned int i = 0; i< product.size;i++)
        printf("%fe +",product.noise[i]);
    printf(")\n");
    */
        raf_pow(r, product, kernel_get_degree(kernel));
        raf_delete(&product);
        raf_delete(&scaled);
    }

    else {
        report_error("Unsupported kernel type.");
    }
    free(isOneHot);
}

/*Apply kernel with OH abstraction*/
static void raf_kernelOH(
    Raf *r,
    Tier tier,
    const Kernel kernel,
    const Real *x,
    const Raf *y,
    const unsigned int space_size
) {
    bool* isOneHot = (bool*)malloc(space_size*sizeof(bool));
    short* origins = (short*)calloc(space_size,sizeof(short));
    Raf* featureRaf = (Raf*) calloc(space_size, sizeof(Raf));
    
    for (unsigned int i = 0; i < space_size; ++i) {
        raf_create(&featureRaf[i], y[0].size);
    }

    fill_isOneHot(isOneHot,tier);
    Raf_sanityCheck(isOneHot,origins,y,space_size);

    if (kernel_get_type(kernel) == KERNEL_RBF) {
        Raf exponent;
        //Raf product;
        unsigned int i;

        raf_create(&exponent, space_size);
        //raf_create(&product, y[0].size);
        for (i = 0; i < space_size; ++i) {
            if(isOneHot[i])
            {
                ohraf_translate(&featureRaf[i], y[i], -x[i]);
                ohraf_pow(&featureRaf[i], featureRaf[i], 2);
            }
            else
            {
                raf_translate(&featureRaf[i], y[i], -x[i]);
                raf_sqr(&featureRaf[i], featureRaf[i]);
            }

        }

        tier_aware_sum(&exponent,isOneHot,tier,featureRaf,origins,space_size);
        raf_scale(&exponent, exponent, -kernel_get_gamma(kernel));
        raf_exp(r, exponent);
        raf_delete(&exponent);

    }

    else if (kernel_get_type(kernel) == KERNEL_POLYNOMIAL) {
        unsigned int i;
        Raf product;

        raf_create(&product, space_size);
        raf_translate_in_place(&product, kernel_get_c(kernel));

        for (i = 0; i < space_size; ++i) {
            if(isOneHot[i])
                ohraf_scale(&featureRaf[i],y[i],x[i]);
            else
                raf_scale(&featureRaf[i],y[i],x[i]);

        }
        tier_aware_sum(&product,isOneHot,tier,featureRaf,origins,space_size);
        raf_pow(r, product, kernel_get_degree(kernel));
        raf_delete(&product);
    }

    else {
        report_error("Unsupported kernel type.");
    }
    for (unsigned int i = 0; i < space_size; ++i) {
        raf_delete(&featureRaf[i]);
    }
    free(featureRaf);
    free(origins);
    free(isOneHot);
}

static void overapproximate(
    Raf *abstract_sample,
    const AdversarialRegion adversarial_region,
    const unsigned int space_size
) {
    const Real *sample = adversarial_region.sample;
    const Perturbation perturbation = adversarial_region.perturbation;
    const Real magnitude = perturbation_get_magnitude(perturbation);
    const Real *epsilon_l = perturbation_get_epsilon_lowerbounds(perturbation),
               *epsilon_u = perturbation_get_epsilon_upperbounds(perturbation);
    const unsigned int image_width = perturbation_get_image_width(perturbation),
                       image_height = perturbation_get_image_height(perturbation),
                       frame_width = perturbation_get_frame_width(perturbation),
                       frame_height = perturbation_get_frame_height(perturbation);
    FILE *stream = perturbation_get_file_stream(perturbation);
    unsigned int i, j;

    switch (perturbation_get_type(perturbation)) {
        case PERTURBATION_L_INF:
            for (i = 0; i < space_size; ++i) {
                const Real l = (sample[i] - magnitude >= 0.0) ? sample[i] - magnitude : 0.0,
                           u = (sample[i] + magnitude <= 1.0) ? sample[i] + magnitude : 1.0;
                raf_create(abstract_sample + i, 1);
                abstract_sample[i].c = 0.5 * (l + u);
                abstract_sample[i].noise[0] = 0.5 * (u - l);
                abstract_sample[i].index = i;
            }
            break;

        case PERTURBATION_HYPER_RECTANGLE:
            for (i = 0; i < space_size; ++i) {
                const Real l = sample[i] - epsilon_l[i],
                           u = sample[i] + epsilon_u[i];
                raf_create(abstract_sample + i, 1);
                abstract_sample[i].c = 0.5 * (l + u);
                abstract_sample[i].noise[0] = 0.5 * (u - l);
                abstract_sample[i].index = i;
            }
            break;

        case PERTURBATION_FRAME:
            for (i = 0; i < image_height; ++i) {
                for (j = 0; j < image_width; ++j) {
                    const unsigned int index = i * image_width + j;
                    Real l, u;
                    raf_create(abstract_sample + index, 1);
                    if (i < frame_height || i > image_height - frame_height
                     || j < frame_width || j > image_width - frame_width) {
                        l = (sample[index] - magnitude >= 0.0) ? sample[index] - magnitude : 0.0,
                        u = (sample[index] + magnitude <= 1.0) ? sample[index] + magnitude : 1.0;
                    }                            
                    else {
                        l = sample[index];
                        u = sample[index];
                    }
                    abstract_sample[index].c = 0.5 * (l + u);
                    abstract_sample[index].noise[0] = 0.5 * (u - l);
                    abstract_sample[index].index = index;
                }
            }
            break;

        case PERTURBATION_FROM_FILE:
            for (i = 0; i < space_size; ++i) {
                double l, u;
                fscanf(stream, "[%lf;%lf] ", &l, &u);
                raf_create(abstract_sample + i, 1);
                abstract_sample[i].c = 0.5 * (l + u);
                abstract_sample[i].noise[0] = 0.5 * (u - l);
                abstract_sample[i].index = i;
            }
            fscanf(stream,"\n");
            break;

        default:
            report_error("Unrecognized type of adversarial region.");
    }
}




Interval *raf_classifier_ovo_score_helper(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    bool* fair_opt,
    unsigned int* has_counterexample,
    Raf *abstract_sample,
    float percent,
    float* RegSize
) {
    const Classifier classifier = raf_classifier->classifier;
    const Real *alpha = classifier_get_alpha(classifier);
    const Real *bias = classifier_get_bias(classifier);
    const Real *support_vectors = classifier_get_support_vectors(classifier);
    const unsigned int space_size = classifier_get_space_size(classifier);
    const unsigned int N = classifier_get_n_classes(classifier);
    const unsigned int *n_support_vectors = classifier_get_n_support_vectors(classifier);
    const Kernel kernel = classifier_get_kernel(classifier);
    unsigned int i, j, k, support_vectors_i_offset = 0, total_support_vectors = 0;
    Raf *K;
    FILE *featureRawFile;
    /* Overapproximates adversarial region with a RAF */
    

    /* Computes total number of support vectors */
    for (i = 0; i < N; ++i) {
        total_support_vectors += n_support_vectors[i];
    }
    if(fair_opt[0]){
        featureRawFile = fopen("feature_score_raw.txt", "a");
    }
    bool* isOneHot = (bool*)malloc(space_size*sizeof(bool));
    short* origins = (short*)calloc(space_size,sizeof(short));

    if(fair_opt[1])
    {
        /* Computes a complete version if kernel is linear */
        
        fill_isOneHot(isOneHot,adversarial_region.tier);
        Raf_sanityCheck(isOneHot,origins,abstract_sample,space_size);

        if (kernel_get_type(kernel) == KERNEL_LINEAR) {
            const Real *coefficients = classifier_get_coefficients(raf_classifier->classifier);
            
            Raf* featureRaf = (Raf*) calloc(space_size, sizeof(Raf));
            for (unsigned int i = 0; i < space_size; ++i) {
                raf_create(&featureRaf[i], abstract_sample[0].size);
            }

            Raf sum;
            raf_create(&sum, space_size);
            for (i = 0; i < N; ++i) {
                for (j = i + 1; j < N; ++j) {
                    const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
                    raf_singleton(&sum, bias[index]);
                    for (k = 0; k < space_size; ++k) {
                        if(isOneHot[k])
                            ohraf_scale(&featureRaf[k], abstract_sample[k], coefficients[index * space_size + k]);
                        else
                            raf_scale(&featureRaf[k], abstract_sample[k], coefficients[index * space_size + k]);
                    }
                    tier_aware_sum(&sum,isOneHot,adversarial_region.tier,featureRaf,origins,space_size);
                    if(percent == 100.0f)
                        raf_to_interval(raf_classifier->buffer + index, sum);
                    //printf("BUFFER : [%f,%f]\n",(raf_classifier->buffer + index)->l,(raf_classifier->buffer + index)->u);
                    if(fair_opt[0])   // Store the score for feature ranking
                    {
                        //printf("SVM (%d,%d) -> %f",i,j,sum.c);
                        fprintf(featureRawFile,"%f ",sum.c);
                        for (unsigned int l = 0; l<sum.size; l++)
                        {
                            //printf("+ %f e%d ",sum.noise[l], l);
                            fprintf(featureRawFile,"%f ",sum.noise[l]);
                        }
                        fprintf(featureRawFile,"%f\n");
                    }
                    }
            }
            bool* maxExample = malloc(sizeof(bool)*space_size);
            bool* minExample = malloc(sizeof(bool)*space_size);
            tierize_raf(&sum,isOneHot,adversarial_region.tier,origins,space_size,maxExample,minExample);
            if(fair_opt[2]){
                has_counterexample[0] = rafOH_has_counterexample(classifier,abstract_sample,maxExample,minExample,space_size);
            }
            if( fair_opt[3] && !fair_opt[0])
                partitionRerun(sum, abstract_sample, percent, raf_classifier, adversarial_region,fair_opt,has_counterexample,RegSize,adversarial_region.tier);
            //printf("%f -> CE: %d\n",percent,*has_counterexample);
            raf_delete(&sum);
            free(maxExample);
            free(minExample);
            
            free(isOneHot);
            free(origins);
            for (unsigned int i = 0; i < space_size; ++i) {
                raf_delete(&featureRaf[i]);
            }
            free(featureRaf);
            return raf_classifier->buffer;
        }

        /* Precomputes kernel matrix. */
        K = (Raf *) malloc(total_support_vectors * sizeof(Raf));
        for (i = 0; i < total_support_vectors; ++i) {
            raf_create(K + i, space_size);
            if(fair_opt[2]){
                raf_kernelOHLast(
                    K + i,
                    adversarial_region.tier,
                    kernel,
                    support_vectors + i * space_size,
                    abstract_sample,
                    space_size
                );
            }else{
                raf_kernelOH(
                    K + i,
                    adversarial_region.tier,
                    kernel,
                    support_vectors + i * space_size,
                    abstract_sample,
                    space_size
                );

            }
        }
    }
    else
    {
        /* Computes a complete version if kernel is linear */
        if (kernel_get_type(kernel) == KERNEL_LINEAR) {
            const Real *coefficients = classifier_get_coefficients(raf_classifier->classifier);
            Raf sum;
            raf_create(&sum, space_size);
            for (i = 0; i < N; ++i) {
                for (j = i + 1; j < N; ++j) {
                    const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
                    raf_singleton(&sum, bias[index]);
                    for (k = 0; k < space_size; ++k) {

                        raf_fma_in_place(&sum, coefficients[index * space_size + k], abstract_sample[k]);

                    } 
                    if(percent == 100.0f)
                        raf_to_interval(raf_classifier->buffer + index, sum);
                    if(fair_opt[0])
                    {
                        //printf("SVM (%d,%d) -> %f",i,j,sum.c);
                        fprintf(featureRawFile,"%f ",sum.c);
                        for (unsigned int l = 0; l<sum.size; l++)
                        {
                            //printf("+ %f e%d ",sum.noise[l], l);
                            fprintf(featureRawFile,"%f ",sum.noise[l]);
                        }
                        fprintf(featureRawFile,"\n");
                    }
                }
            }
            raf_delete(&sum);
            return raf_classifier->buffer;
        }
        /* Precomputes kernel matrix. */
        K = (Raf *) malloc(total_support_vectors * sizeof(Raf));
        for (i = 0; i < total_support_vectors; ++i) {
            raf_create(K + i, space_size);
            raf_kernel(
                K + i,
                kernel,
                support_vectors + i * space_size,
                abstract_sample,
                space_size
            );
        }

    }
    
    

    /* Computes scores */
    for (i = 0; i < N; ++i) {
        unsigned int support_vectors_j_offset;
        if (i > 0) {
            support_vectors_i_offset += n_support_vectors[i - 1];
        }
        support_vectors_j_offset = support_vectors_i_offset;
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;

            Raf sum;
            raf_create(&sum, space_size);
            sum.c = bias[index];
            support_vectors_j_offset += n_support_vectors[j - 1];

            for (k = 0; k < n_support_vectors[i]; ++k) {
                raf_fma_in_place(
                    &sum,
                    alpha[(j - 1) * total_support_vectors + support_vectors_i_offset + k],
                    K[support_vectors_i_offset + k]
                );
            }

            for (k = 0; k < n_support_vectors[j]; ++k) {
                raf_fma_in_place(
                    &sum,
                    alpha[i * total_support_vectors + support_vectors_j_offset + k],
                    K[support_vectors_j_offset + k]
                );
            }            
            if(fair_opt[1]){
                bool* maxExample = malloc(sizeof(bool)*space_size);
                bool* minExample = malloc(sizeof(bool)*space_size);
                tierize_raf(&sum,isOneHot,adversarial_region.tier,origins,space_size,maxExample,minExample);
                if(fair_opt[2]){
                    has_counterexample[0] = rafOH_has_counterexample(classifier,abstract_sample,maxExample,minExample,space_size);
                }
                if( fair_opt[3] && !fair_opt[0])
                    partitionRerun(sum, abstract_sample, percent, raf_classifier, adversarial_region,fair_opt,has_counterexample,RegSize,adversarial_region.tier);
                //printf("%f -> CE: %d\n",percent,*has_counterexample);
            }
            if(fair_opt[0])
            {
                //printf("SVM (%d,%d) -> %f",i,j,sum.c);
                fprintf(featureRawFile,"%f ",sum.c);
                for (unsigned int l = 0; l<sum.size; l++)
                {
                    //printf("+ %f e%d ",sum.noise[l], l);
                    fprintf(featureRawFile,"%f ",sum.noise[l]);
                }
                //printf("+ %f delta ",sum.delta);
                fprintf(featureRawFile,"\n");
            }
            if(percent == 100.0f)
                raf_to_interval(raf_classifier->buffer + index, sum);
            raf_delete(&sum);
        }
    }
    for (i = 0; i < total_support_vectors; ++i) {
        raf_delete(K + i);
    }
    free(K);
    free(isOneHot);
    free(origins);


    return raf_classifier->buffer;
}

static Interval *raf_classifier_ovo_score(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    bool* fair_opt,
    unsigned int* has_counterexample
)
{
    const Classifier classifier = raf_classifier->classifier;
    const unsigned int space_size = classifier_get_space_size(classifier);
    Raf *abstract_sample;
    abstract_sample = (Raf *) malloc(space_size * sizeof(Raf));
    overapproximate(abstract_sample, adversarial_region, space_size);
    float* RegSize = (float *)malloc(2*sizeof(float));
    RegSize[0] = 0.0;
    RegSize[1] = 0.0;
    Interval* ans = raf_classifier_ovo_score_helper(raf_classifier,adversarial_region,fair_opt,has_counterexample,abstract_sample,100.0f,RegSize);
    //printf("-ve Region: %f, +ve region: %f\n",RegSize[0],RegSize[1]);
    if(RegSize[0]>0 && RegSize[1]>0)
        has_counterexample[1] = 1;
    for (unsigned i = 0; i < space_size; ++i) {
        raf_delete(abstract_sample + i);
    }
    free(abstract_sample);
    free(RegSize);
    return ans;
}


static unsigned int raf_classifier_ovo_classify(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    char **classes,
    bool* fair_opt,
    unsigned int* has_counterexample
) {
    const Interval *scores = raf_classifier_ovo_score(raf_classifier, adversarial_region, fair_opt,has_counterexample);
    //printf("SCORE: [%f,%f]\n", scores[0].l,scores[0].u);
    Interval *votes;
    unsigned int i, j, winning_classes = 0;
    const unsigned int N = classifier_get_n_classes(raf_classifier->classifier);

    votes = (Interval *) malloc(N * sizeof(Interval));
    for (i = 0; i < N; ++i) {
        votes[i].l = 0.0;
        votes[i].u = 0.0;
    }

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            if (scores[index].l >= 0.0) {
                votes[i].l += 1.0;
                votes[i].u += 1.0;
            }
            else if (scores[index].u <= 0.0) {
                votes[j].l += 1.0;
                votes[j].u += 1.0;
            }
            else {
                votes[i].u += 1.0;
                votes[j].u += 1.0;
            }
        }
    }


    for (i = 0; i < N; ++i) {
        unsigned int is_maximal = 1;
        for (j = 0; j < N; ++j) {
            if (i == j) {
                continue;
            }
            if (votes[j].l > votes[i].u) {
                is_maximal = 0;
                break;
            }
        }

        if (is_maximal) {
            classes[winning_classes] = classifier_get_classes(raf_classifier->classifier)[i];
            ++winning_classes;
        }
    }

    free(votes);

    return winning_classes;
/*
    const Interval *scores = raf_classifier_ovo_score(raf_classifier, adversarial_region);
    unsigned int *votes, i, j, max_class = 0, winning_classes = 0;
    const unsigned int N = classifier_get_n_classes(raf_classifier->classifier);

    votes = (unsigned int *) calloc(N * (N + 1) / 2, sizeof(unsigned int));
    if (votes == NULL) {
        report_error("Cannot allocate memory.");
    }

    for (i = 0; i < N; ++i) {
        for (j = i + 1; j < N; ++j) {
            const unsigned int index = i * (N - 1) - (i * (i + 1)) / 2 + j - 1;
            if (scores[index].l >= 0.0) {
                ++votes[i];
            }
            else if (scores[index].u <= 0) {
                ++votes[j];
            }
            else {
                ++votes[i];
                ++votes[j];
            }
        }
    }

    for (i = 0; i < N; ++i) {
        if (votes[i] > votes[max_class]) {
            winning_classes = 1;
            classes[0] = classifier_get_classes(raf_classifier->classifier)[i];
            max_class = i;
        }
        else if (votes[i] == votes[max_class]) {
            classes[winning_classes] = classifier_get_classes(raf_classifier->classifier)[i];
            ++winning_classes;
        }
    }

    free(votes);

    return winning_classes;
*/
}



RafClassifier raf_classifier_create(const Classifier classifier) {
    const unsigned int N = classifier_get_n_classes(classifier);
    RafClassifier raf_classifier = (RafClassifier) malloc(sizeof(struct raf_classifier));
    if (!raf_classifier) {
        report_error("Cannot allocate memory.");
    }

    raf_classifier->classifier = classifier;
    raf_classifier->buffer = (Interval *) malloc((N * (N - 1)) / 2 * sizeof(Interval));

    return raf_classifier;
}



void raf_classifier_delete(RafClassifier *raf_classifier) {
    if (raf_classifier == NULL || *raf_classifier == NULL) {
        return;
    }

    free((*raf_classifier)->buffer);
    free(*raf_classifier);
    *raf_classifier = NULL;
}



Interval *raf_classifier_score(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region
) {
    switch (classifier_get_type(raf_classifier->classifier)) {
        case CLASSIFIER_OVO:
            unsigned int dummy = 0;
            return raf_classifier_ovo_score(raf_classifier, adversarial_region,false, &dummy);
        default:
            report_error("Unsupported classifier type.");
    }
}



unsigned int raf_classifier_classify(
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    char **classes,
    bool* fair_opt,
    unsigned int* has_counterexample
) {
    switch (classifier_get_type(raf_classifier->classifier)) {
        case CLASSIFIER_OVO:
            return raf_classifier_ovo_classify(raf_classifier, adversarial_region, classes,fair_opt,has_counterexample);
        default:
            report_error("Unsupported classifier.");
    }
}
