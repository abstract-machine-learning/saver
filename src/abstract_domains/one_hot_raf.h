/**
 * One Hot Reduced Affine Forms (OHRAF).
 *
 * Defines abstract transfer functions for OHRAF.
 *
 * @file one_hot_raf.h
 * @author Abhinandan Pal <abhinandan.mike123@gmail.com>
 */
#ifndef ONE_HOT_RAF_H
#define ONE_HOT_RAF_H

#include <stdio.h>
#include <stdbool.h>

#include "raf.h"

/** OHRAF abstract value. */

/**
 * Structure of a OHRAF.
 *
 * One Hot Reduced Affine Forms (OHRAF) allows to marafain the constrains of 
 * one-hot encoding while using RAF abstraction in general. In one-hot encoding
 * in a tier only one of the values can be 1 and rest 0.
 * 
 * OHRAF stores the value origniating from zero and the addition value to be sumed to obtain
 * the value originating from one.
 * During the innerproduct (of poly kernal) or magnitude (of KBF kernal) step it obtains
 * an Raf for the tier by setting one of the features of the tier as 1 and rest as 0
 * so as to obtain the min and the max
 */


/*
 * As RAF and ohraf are structurally same. The current version doesnt Convert RAF to 
 * ohraf. [To Do: use an union type for it]
 */

void Raf_sanityCheck(const bool* isOneHot,short* origin,const Raf *y, const unsigned int space_size);




/**
 * @param[out] r Result
 * @param[in]  x One hot Raf
 * @param[in]  t Translation length
 */
void ohraf_translate(Raf *r, const Raf x, const Real t);

/**
 * @param[out] r Result
 * @param[in]  x One hot Raf
 * @param[in]  s Scale magnitude
 */
void ohraf_scale(Raf *r, const Raf x, const Real s);

/**
 * @param[out] r Result
 * @param[in]  x One hot Raf
 */
void ohraf_exponent(Raf *r, const Raf x);

/**
 * @param[out] r Result
 * @param[in]  x One hot Raf
 * @param[in]  degree power degree
 */
void ohraf_pow(Raf *r, const Raf x, const unsigned int degree);

/**
 * @param[out] r Result
 * @param[in]  x One hot Raf
 * @param[in]  tier_size No of features in current tier
 * Obtain An Raf by merging all the one hot Raf in current tier.
 */
void ohraf_Rafize(Raf *r,const Raf* x,const short* origin,const unsigned int tier_size);

#endif