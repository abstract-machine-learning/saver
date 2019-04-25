/**
 * Dataset.
 * 
 * Defines some utility function to manipulate a dataset.
 * 
 * @file dataset.h
 */
#ifndef DATASET_H
#define DATASET_H

#include <stdio.h>
#include "type.h"

/**
 * Type of a dataset.
 */
typedef struct dataset *Dataset;


/**
 * Reads a dataset from a source.
 * 
 * @param[in,out] stream Source to read from
 * @return Dataset read from source
 */
Dataset dataset_read(FILE *stream);


/**
 * Deletes a dataset.
 * 
 * @param[out] dataset Pointer to dataset to delete
 */
void dataset_delete(Dataset *dataset);


/**
 * Returns number of entries in given dataset.
 * 
 * @param[in] dataset Dataset
 * @return Number of entries
 */
unsigned int dataset_get_size(const Dataset dataset);


/**
 * Returns number of features in given dataset.
 * 
 * @param[in] dataset Dataset
 * @return Number of features
 */
unsigned int dataset_get_space_size(const Dataset dataset);


/**
 * Returns data in i-esim entry of given dataset.
 * 
 * @param[in] dataset Dataset
 * @param[in] i       Index of entry to read
 * @return Pointer to data of i-esim entry
 */
Real *dataset_get_row(const Dataset dataset, const unsigned int i);


/**
 * Returns label of i-esim entry of given dataset.
 * 
 * @param[in] dataset Dataset
 * @param[in] i       Index of entry to read
 * @return Label of i-esim entry
 */
char *dataset_get_label(const Dataset dataset, const unsigned int i);

#endif
