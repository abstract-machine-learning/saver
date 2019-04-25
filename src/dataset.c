#include "dataset.h"

#include <stdio.h>
#include <malloc.h>

#include "report_error.h"

#define LABEL_BUFFER_SIZE 32


/**
 * Structure of a dataset.
 */
struct dataset {
    unsigned int size;        /**< Number of samples. */
    unsigned int space_size;  /**< Size of the feature space. */
    Real *data;               /**< Features (row major matrix). */
    char *labels;             /**< Labels. */
};



Dataset dataset_read(FILE *stream) {
    unsigned int n_rows, n_cols, i, j;
    Real *data;
    char *labels;
    int result;
    Dataset dataset;

    result = fscanf(stream, "# %u %u", &n_rows, &n_cols);

    labels = (char *) malloc(LABEL_BUFFER_SIZE * n_rows * sizeof(char));
    data = (Real *) malloc(n_rows * n_cols * sizeof(Real));

    for (i = 0; i < n_rows; ++i) {
        double buffer;
        result = fscanf(stream, "\n%[^,],", labels + (i * LABEL_BUFFER_SIZE));
        for (j = 0; j < n_cols - 1; ++j) {
            result = fscanf(stream, "%lf,", &buffer);
            data[i * n_cols + j] = buffer;
        }

        result = fscanf(stream, "%lf", &buffer);
        data[i * n_cols + j] = buffer;
    }

    dataset = (Dataset) malloc(sizeof(struct dataset));
    if (!dataset) {
        report_error("Cannot allocate memory.");
    }

    dataset->size = n_rows;
    dataset->space_size = n_cols;
    dataset->data = data;
    dataset->labels = labels;

    (void) result;
    return dataset;
}



void dataset_delete(Dataset *dataset) {
    if (dataset == NULL || *dataset == NULL) {
        return;
    }

    free((*dataset)->data);
    free((*dataset)->labels);
    free(*dataset);
    *dataset = NULL;
}


unsigned int dataset_get_size(const Dataset dataset) {
    return dataset->size;
}


unsigned int dataset_get_space_size(const Dataset dataset) {
    return dataset->space_size;
}


Real *dataset_get_row(const Dataset dataset, const unsigned int i) {
    return dataset->data + i * dataset->space_size;
}


char *dataset_get_label(const Dataset dataset, const unsigned int i) {
    return dataset->labels + i * LABEL_BUFFER_SIZE;
}
