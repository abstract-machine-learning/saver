#include "perturbation.h"

#include <stdlib.h>
#include <string.h>

/**
 * Structure of a perturbation.
 */
struct perturbation {
    PerturbationType type;    /**< Type of perturbation. */
    Real magnitude;           /**< Magnitude of the perturbation. */
    unsigned int space_size;  /**< Size of the space. */
    /** Data of different perturbations. */
    union {
        /** Structure of a frame perturbation. */
        struct {
            unsigned int image_width;   /**< Width of the image. */
            unsigned int image_height;  /**< Height of the image. */
            unsigned int frame_width;   /**< Width of the frame. */
            unsigned int frame_height;  /**< Height of the frame. */
        } frame;
    } perturbation_data;
};



void perturbation_read(
    Perturbation *perturbation,
    const int argc,
    const char *argv[],
    const unsigned int space_size
) {
    const char *perturbation_name = (argc > 0) ? argv[0] : DEFAULT_PERTURBATION;

    *perturbation = (Perturbation) malloc(sizeof(struct perturbation));
    (*perturbation)->magnitude = (argc > 1) ? atof(argv[1]) : DEFAULT_EPSILON;
    (*perturbation)->space_size = space_size;

    if (strcmp(perturbation_name, "l_one") == 0) {
        (*perturbation)->type = PERTURBATION_L_ONE;
    }

    else if (strcmp(perturbation_name, "l_inf") == 0) {
        (*perturbation)->type = PERTURBATION_L_INF;
    }

    else if (strcmp(perturbation_name, "frame") == 0 && argc > 5) {
        (*perturbation)->type = PERTURBATION_FRAME;
        (*perturbation)->perturbation_data.frame.image_width = (unsigned int) atoi(argv[2]);
        (*perturbation)->perturbation_data.frame.image_height = (unsigned int) atoi(argv[3]);
        (*perturbation)->perturbation_data.frame.frame_width = (unsigned int) atoi(argv[4]);
        (*perturbation)->perturbation_data.frame.frame_height = (unsigned int) atoi(argv[5]);
    }

    else {
        fprintf(stderr, "Unrecognized perturbation or wrong number of parameters.\n");
        exit(EXIT_FAILURE);
    }
}



void perturbation_delete(Perturbation *perturbation) {
    free(*perturbation);
    *perturbation = NULL;
}



PerturbationType perturbation_get_type(const Perturbation perturbation) {
    return perturbation->type;
}



Real perturbation_get_magnitude(const Perturbation perturbation) {
    return perturbation->magnitude;
}



Real perturbation_get_image_width(const Perturbation perturbation) {
    return perturbation->perturbation_data.frame.image_width;
}



Real perturbation_get_image_height(const Perturbation perturbation) {
    return perturbation->perturbation_data.frame.image_height;
}



Real perturbation_get_frame_width(const Perturbation perturbation) {
    return perturbation->perturbation_data.frame.frame_width;
}



Real perturbation_get_frame_height(const Perturbation perturbation) {
    return perturbation->perturbation_data.frame.frame_height;
}
