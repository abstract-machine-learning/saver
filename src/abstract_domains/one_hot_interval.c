#include "one_hot_interval.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "../report_error.h"


void interval_to_ohint(const bool* isOneHot,short* origin, const Interval *y, const unsigned int space_size)
{
    for(unsigned int i = 0; i<space_size;i++)
    {
        if(isOneHot[i])
        {
            if(y[i].l == 0 && y[i].u == 0)
                origin[i] = 0; 
            else if(y[i].l == 1 && y[i].u == 1)
                origin[i] = 1;
            else if(y[i].l == 0 && y[i].u == 1)
                origin[i] = 2;
            else
                {printf("Initial Interval %d : [%f,%f] doesn't satisfy one hot constrain",i,y[i].l,y[i].u);
                exit(0);}        
        }
        else
            origin[i] = 2;
    }
    
}



/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  t Translation length
 */
void ohint_translate(Interval *r, const Interval x, const Real t) {
    round_down;
    r->l = x.l + t;
    round_up;
    r->u = x.u + t;
}

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  s Scale magnitude
 */
void ohint_scale(Interval *r, const Interval x, const Real s) {
    round_down;
    r->l = x.l * s;
    round_up;
    r->u = x.u * s;
}

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 */
void ohint_exponent(Interval *r, const Interval x) {
    round_down;
    r->l = exp(x.l);
    round_up;
    r->u = exp(x.u);
}

/**
 * @param[out] r Result
 * @param[in]  x One hot Interval
 * @param[in]  d power degree
 */
void ohint_pow(Interval *r, const Interval x, const unsigned int degree) {
    round_down;
    r->l = pow(x.l,degree);
    round_up;
    r->u = pow(x.u,degree);
}

void ohint_intervalize(Interval *r,const Interval* x,const short* origin,const unsigned int tier_size)
{
    Real min = 0.0, max = 0.0;
    bool isFirst = true;
    for(unsigned int i = 0; i < tier_size; i++){
        if(origin[i] == 0)
            continue;
        Real val = x[i].u;
        bool quit = false;
        
        for(unsigned int j = 0; j < tier_size; j++){
            if(i == j)
                continue;
            if(origin[j] == 1){  
                quit = true;
                break;
            }
            val += x[j].l;
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
    r->l = min;
    r->u = max;
}