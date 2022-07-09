#include "one_hot_raf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../report_error.h"
#define eps 0.0000001
#define MIN_PART 6.0

/**
 * Assert that input RAF is 0 , 1 or [0,1] and flag them accordingly.  
 */
void Raf_sanityCheck(const bool* isOneHot,short* origin,const Raf *y, const unsigned int space_size)
{
	for(unsigned int i = 0; i<space_size;i++)
    {
        if(isOneHot[i])
        {
        	if(y[i].index < 0)
        	{
        		printf("Initial one hot encoded RAF (%d) has multiple noise",i);
                exit(0);
        	}
            if( fabs(y[i].noise[0] - 0.0) < eps && fabs(y[i].c - 0.0)< eps)
                origin[i] = 0; 
            else if(fabs(y[i].noise[0] - 0.0) < eps && fabs(y[i].c - 1.0)< eps)
                origin[i] = 1;
            else if(fabs(y[i].noise[0] - 0.5) < eps && fabs(y[i].c - 0.5) < eps)
                origin[i] = 2;
            else{
            	printf("\nA Initial RAF %d : [%.30f + %.30fe] doesn't satisfy one hot constrain\n",i,y[i].c,y[i].noise[0]);
                exit(0);
            }        
        }
        else
            origin[i] = 2;
    }
}



void ohraf_translate(Raf *r, const Raf x, const Real t)
{
	/*one hot raf can only have one noise term*/
	r->c = x.c + t;
    r->noise[0] =  x.noise[0];
    r->index = x.index;
}


void ohraf_scale(Raf *r, const Raf x, const Real s)
{
	/*one hot raf can only have one noise term*/
	r->c = s * x.c;
    r->noise[0] =  s *x.noise[0];
    r->index = x.index;
}


void ohraf_exponent(Raf *r, const Raf x)
{
	Real zero = x.c - x.noise[0];
	Real one = x.c + x.noise[0];
	zero = exp(zero);
	one = exp(one);
	r->c = 0.5 * (one + zero);
	r->noise[0] = 0.5*(one - zero);
	r->index = x.index;
}


void ohraf_pow(Raf *r, const Raf x, const unsigned int degree)
{

	Real zero = x.c - x.noise[0];
	Real one = x.c + x.noise[0];
	zero = pow(zero,degree);
	one = pow(one,degree);
	r->c = 0.5 * (one + zero);
	r->noise[0] = 0.5*(one - zero);
	r->index = x.index;
}


/* Obtain a single RAF for a tier using OH in RAF form for all its features*/
void ohraf_Rafize(Raf *r,const Raf* x,const short* origin,const unsigned int tier_size)
{
	Real min = 0.0, max = 0.0;
    bool isFirst = true;
    for(unsigned int i = 0; i < tier_size; i++){
        if(origin[i] == 0)
            continue;
        Real val = x[i].c + x[i].noise[0];	//value originating from 1
        bool quit = false;
        
        for(unsigned int j = 0; j < tier_size; j++){
            if(i == j)
                continue;
            if(origin[j] == 1){  
                quit = true;
                break;
            }
            val += x[j].c - x[j].noise[0];		//value originating from 0
        }
        if(quit)
            continue;
        if(isFirst){
            isFirst = false;
            min = val;
            max = val;
        }
        else if(val > max)
            max = val;
        else if(val < min)
            min = val;
    }
    r->c = 0.5 * (min + max);
    r->noise[0] = 0.5 * (max - min);
    r->index = x[0].index;
}

/*Obtain the flags to be use to obtain the counter example*/
void tierize_raf(Raf *r,const bool *isOneHot,const Tier tier,const short* origins,const unsigned int space_size,bool* maxExample,bool* minExample)
{

    
    for(unsigned int i = 0;i < space_size; i++){
        minExample[i] = false;
        maxExample[i] = false;
    }
    for(unsigned int i = 0;i < space_size; i++){
        if(isOneHot[i]){
            unsigned int j;
            for(j = i;j <space_size;j++){
                if(tier.tiers[i] != tier.tiers[j])
                    break;
            }
            if(j == i + 1)
            {
                if(r->noise[i] > 0)
                    maxExample[i] = true;
                else
                    minExample[i] = true;
            }
            else{
                unsigned int* pos = malloc(sizeof(int)*2);
                tierize_raf_helper(r,i,j,origins,pos);
                for(unsigned int k = i;k < j;k++){
                    if(k == pos[0])
                        minExample[k] = true;
                    if(k == pos[1])
                        maxExample[k] = true;
                i = j-1;
                
                }
                free(pos);
            }   
            
        }
        else
        {
            if(r->noise[i] > 0)
                maxExample[i] = true;
            else
                minExample[i] = true;
        }
    }
    /*printf("\nMin Example: ");
    for(unsigned int i = 0;i < space_size; i++){
        printf("%d ",minExample[i]);
    }
    printf("\nMax Example: ");
    for(unsigned int i = 0;i < space_size; i++){
        printf("%d ",maxExample[i]);
    }
    printf("\n");*/
}

/*
 * Given an RAF modify it so as to reduce noises for features to a single noise for the tier.
 * And store the feature setting which to one leads to the min and max.
 */
void tierize_raf_helper(Raf *r,unsigned int f1,unsigned int fn,const short* origin,unsigned int* pos)
{
    Real min = 0.0, max = 0.0;
    unsigned int min_id = 0,max_id = 0;
    bool isFirst = true;
    for(unsigned int i = f1; i < fn; i++){
        if(origin[i] == 0)
            continue;
        Real val = r->noise[i];  //value originating from 1
        bool quit = false;
        
        for(unsigned int j = f1; j < fn; j++){
            if(i == j)
                continue;
            if(origin[j] == 1){  
                quit = true;
                break;
            }
            val -= r->noise[j];      //value originating from 0
        }
        if(quit)
            continue;
        if(isFirst){
            isFirst = false;
            min = val;
            max = val;
            min_id = i;
            max_id = i;
        }
        else if(val > max)
        {   max = val;
            max_id = i;
        }
        else if(val < min)
        {   min = val;
            min_id = i;
        }
    }
    Real range = 0.0;
    for(unsigned int i = f1; i < fn; i++){
        range += r->noise[i];
    }
    r->c += 0.5 * (min + max);
    r->noise[f1] = 0.5 * (max - min);
    for(unsigned int i = f1+1; i < fn; i++){
        r->noise[i] = 0.0;
    }
    pos[0] = min_id;
    pos[1] = max_id;
}

/*
 * Use the Flag to obtain potential counterexample and predit their label using the classifer being analised.
 * To check if thye are actually counter examples.
*/
int rafOH_has_counterexample(const Classifier classifier, const Raf* abstract_sample,bool* maxExample,bool* minExample,const unsigned int space_size)
{
    Real* maxSample = malloc(sizeof(Real)*space_size);
    Real* minSample = malloc(sizeof(Real)*space_size);
    for (unsigned int i = 0; i < space_size; ++i) {
        Real l, u;
        l = abstract_sample[i].c - abstract_sample[i].noise[0];
        u = abstract_sample[i].c + abstract_sample[i].noise[0];
        
        if(maxExample[i])
            maxSample[i] = u;
        else
            maxSample[i] = l;

        if(minExample[i])
            minSample[i] = u;
        else
            minSample[i] = l;
    }

    /*printf("\nMin Sample: ");
    for(unsigned int i = 0;i < space_size; i++){
        printf("%f ",minSample[i]);
    }
    printf("\nMax Sample: ");
    for(unsigned int i = 0;i < space_size; i++){
        printf("%f ",maxSample[i]);
    }
    printf("\n<----\n");*/

    char **classes = (char **) malloc(classifier_get_n_classes(classifier) * sizeof(char *));
    classifier_classify(classifier, maxSample, classes);
    char maxSampleClass = classes[0];
    classifier_classify(classifier, minSample, classes);
    char minSampleClass = classes[0];

    //printf("--->\nmax-> %c ; min -> %c ---> %d\n",maxSampleClass,minSampleClass,maxSampleClass != minSampleClass);
    free(classes);
    free(maxSample);
    free(minSample);
    return (maxSampleClass != minSampleClass);
}

/*
*  Partition over the feature with highest weight in the score RAF.
*  And recursively analyse these partitions to find counter examples
*  and counter sub-regions 
*/
void partitionRerun(Raf score, Raf * abstract_sample, float percent, 
    const RafClassifier raf_classifier,
    const AdversarialRegion adversarial_region,
    bool* fair_opt,
    unsigned int* has_counterexample,float* RegSize, Tier tiers)
{
    Interval* intScr = malloc(sizeof(Interval));
    raf_to_interval(intScr, score);
    //printf("%f -> [%f,%f]\n",percent,intScr->l,intScr->u);
    
    /*for(unsigned int i = 0; i< score.size;i++)
    {
        printf("+ %f e_%d ",score.noise[i],i);
    }
    printf("\n");*/

    if(intScr->l >= 0 && intScr->u >= 0)
    {
        RegSize[1] += percent;
    }
    else if(intScr->l < 0 && intScr->u < 0)
    {
        RegSize[0] += percent; 
    }
    else if(percent < MIN_PART ){} //|| has_counterexample[0] == 1
    else
    {
        unsigned int pos = 0;
        Real maxW = fabs(score.noise[0]);
        for (unsigned int l = 0; l<score.size; l++)
        {
            if(fabs(score.noise[l]) > maxW)
            {
                if(tiers.tiers[tiers.size+l] == 1)
                {
                    continue;
                }
                pos = l;
                maxW = fabs(score.noise[l]);
            }
        }
        //printf("POS: {%d} -> {%d}\n",pos,tiers.tiers[pos]);
        Interval* intInput = malloc(sizeof(Interval));
        raf_to_interval(intInput, abstract_sample[pos]);
        Real store_c = abstract_sample[pos].c;
        Real store_noise = abstract_sample[pos].noise[0];
        
        Real mid = (intInput->l + intInput->u)/2;
        abstract_sample[pos].c = (intInput->l + mid)/2;
        abstract_sample[pos].noise[0] = (mid - intInput->l)/2;
        

        unsigned int dummy1 = 0;
        raf_classifier_ovo_score_helper(raf_classifier,adversarial_region,fair_opt,&dummy1,abstract_sample,percent/2,RegSize);

        mid = (intInput->l + intInput->u)/2;
        abstract_sample[pos].c = (intInput->u + mid)/2;
        abstract_sample[pos].noise[0] = (intInput->u - mid)/2;
        
        unsigned int dummy2 = 0;
        raf_classifier_ovo_score_helper(raf_classifier,adversarial_region,fair_opt,&dummy2,abstract_sample,percent/2,RegSize);

        if(dummy2 == 1 || dummy1 == 1)
            has_counterexample[0] = 1;
        abstract_sample[pos].c = store_c;
        abstract_sample[pos].noise[0] = store_noise;

        free(intInput);
    }
    free(intScr);

}

/*
* Faster alternative to partition where instead of reruning we restrict the values noise can take.
* While being significtly faster its a worse over approximation than reruning on partition.
*/

void partitionRaf(Raf score)
{
    float* regionSize = malloc(2*sizeof(float));
    labelSize(score,regionSize,100.0);
    //printf("LR: %f RR: %f",regionSize[0],regionSize[1]);
    free(regionSize);
}

void labelSize(Raf score, float * regionSize, float percent)
{
    Interval* intScr = malloc(sizeof(Interval));
    raf_to_interval(intScr, score);
    //printf("%f -> [%f,%f]",percent,intScr->l,intScr->u);
    if(intScr->l >= 0 && intScr->u >= 0)
    {
        regionSize[1] += percent; 
    }
    else if(intScr->l < 0 && intScr->u < 0)
    {
        regionSize[0] += percent; 
    }
    else if(percent < 6.0f){}
    else
    {
        unsigned int pos = 0;
        Real maxW = score.noise[0];
        for (unsigned int l = 0; l<score.size; l++)
        {
            if(score.noise[l] > maxW)
            {
                pos = l;
                maxW = score.noise[l];
            }
        }
        Raf *lowerRaf = (Raf *)malloc(sizeof(Raf));
        raf_create(lowerRaf, score.size);
        Raf *upperRaf = (Raf *)malloc(sizeof(Raf));
        raf_create(upperRaf, score.size);
        raf_copy(lowerRaf,score);
        raf_copy(upperRaf,score);
        lowerRaf->c -= maxW/2;
        lowerRaf->noise[pos] = maxW/2;
        upperRaf->c += maxW/2;
        upperRaf->noise[pos] = maxW/2;

        /*Interval* intScr1 = malloc(sizeof(Interval));
        raf_to_interval(intScr1, *lowerRaf);
        printf("-> [%f,%f]",intScr1->l,intScr1->u);

        Interval* intScr2 = malloc(sizeof(Interval));
        raf_to_interval(intScr2, *upperRaf);
        printf("; [%f,%f]\n",intScr2->l,intScr2->u);*/

        labelSize(*lowerRaf,regionSize, percent/2);
        labelSize(*upperRaf,regionSize, percent/2);
        free(lowerRaf);
        free(upperRaf);
    }
    free(intScr);
    return;
}