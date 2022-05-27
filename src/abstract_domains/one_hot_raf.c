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
	zero = exp(x.c);
	one = exp(x.c);
	r->c = 0.5 * (one + zero);
	r->noise[0] = 0.5*(one - zero);
	r->index = x.index;
}


void ohraf_pow(Raf *r, const Raf x, const unsigned int degree)
{
	Real zero = x.c - x.noise[0];
	Real one = x.c + x.noise[0];
	zero = pow(x.c,degree);
	one = pow(x.c,degree);
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
    r->noise[0] = 0.5 * (min - max);
    r->index = x[0].index;
}