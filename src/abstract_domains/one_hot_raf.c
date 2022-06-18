#include "one_hot_raf.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../report_error.h"


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
            if(y[i].noise[0] == 0 && y[i].c == 0)
                origin[i] = 0; 
            else if(y[i].noise[0] == 0 && y[i].c == 1)
                origin[i] = 1;
            else if(y[i].noise[0] == 0.5 && y[i].c == 0.5)
                origin[i] = 2;
            else{
            	printf("Initial RAF %d : [%f + %fe] doesn't satisfy one hot constrain",i,y[i].c,y[i].noise[0]);
                exit(0);}        
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

void tierize_raf(Raf *r,const bool *isOneHot,const Tier tier,const short* origins,const unsigned int space_size,bool* maxExample,bool* minExample)
{
    //unsigned int* has_counterexample = 0;
    
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
            unsigned int* pos = malloc(sizeof(int)*2);
            tierize_raf_helper(r,i,j,origins,pos);
            for(unsigned int k = i;k < j;k++){
                if(k == pos[0])
                    minExample[k] = true;
                if(k == pos[1])
                    maxExample[k] = true;
            }
            i = j-1;
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

    //printf("--->\nmax-> %c ; min -> %c\n ---> %d",maxSampleClass,minSampleClass,maxSampleClass != minSampleClass);
    free(classes);
    return (maxSampleClass != minSampleClass);
}