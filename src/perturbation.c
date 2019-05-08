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
        /** Structure of a hyper rectangle perturbation. */
        struct {
            Real *epsilon_l;  /**< Array of lowerbound epsilons. */
            Real *epsilon_u;  /**< Array of upperbound epsilons. */
        } hyper_rectangle;
        /** Structure of a frame perturbation. */
        struct {
            unsigned int image_width;   /**< Width of the image. */
            unsigned int image_height;  /**< Height of the image. */
            unsigned int frame_width;   /**< Width of the frame. */
            unsigned int frame_height;  /**< Height of the frame. */
        } frame;
    } perturbation_data;
};



void perturbation_create_hyper_rectangle(
    Perturbation *perturbation,
    const unsigned int space_size
) {
    unsigned int i;

    *perturbation = (Perturbation) malloc(sizeof(struct perturbation));
    (*perturbation)->type = PERTURBATION_HYPER_RECTANGLE;
    (*perturbation)->magnitude = 0.0;
    (*perturbation)->space_size = space_size;
    (*perturbation)->perturbation_data.hyper_rectangle.epsilon_l = (Real *) malloc(space_size * sizeof(Real));
    (*perturbation)->perturbation_data.hyper_rectangle.epsilon_u = (Real *) malloc(space_size * sizeof(Real));

    for (i = 0; i < space_size; ++i) {
        (*perturbation)->perturbation_data.hyper_rectangle.epsilon_l[i] = 0.0;
        (*perturbation)->perturbation_data.hyper_rectangle.epsilon_u[i] = 0.0;
    }
}



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
    if ((*perturbation)->type == PERTURBATION_HYPER_RECTANGLE) {
        free((*perturbation)->perturbation_data.hyper_rectangle.epsilon_l);
        free((*perturbation)->perturbation_data.hyper_rectangle.epsilon_u);
    }

    free(*perturbation);
    *perturbation = NULL;
}



void perturbation_copy(Perturbation dst, const Perturbation src) {
    dst->type = src->type;
    dst->magnitude = src->magnitude;
    dst->space_size = src->space_size;

    if (src->type == PERTURBATION_HYPER_RECTANGLE) {
        memcpy(
            dst->perturbation_data.hyper_rectangle.epsilon_l,
            src->perturbation_data.hyper_rectangle.epsilon_l,
            src->space_size * sizeof(Real)
        );

        memcpy(
            dst->perturbation_data.hyper_rectangle.epsilon_u,
            src->perturbation_data.hyper_rectangle.epsilon_u,
            src->space_size * sizeof(Real)
        );
    }
}



PerturbationType perturbation_get_type(const Perturbation perturbation) {
    return perturbation->type;
}



Real perturbation_get_magnitude(const Perturbation perturbation) {
    return perturbation->magnitude;
}



unsigned int perturbation_get_space_size(const Perturbation perturbation) {
    return perturbation->space_size;
}



Real *perturbation_get_epsilon_lowerbounds(const Perturbation perturbation) {
    return perturbation->perturbation_data.hyper_rectangle.epsilon_l;
}



Real *perturbation_get_epsilon_upperbounds(const Perturbation perturbation) {
    return perturbation->perturbation_data.hyper_rectangle.epsilon_l;
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



void perturbation_set_magnitude(Perturbation perturbation, const Real magnitude) {
    perturbation->magnitude = magnitude;
}



void perturbation_print(const Perturbation perturbation, FILE *stream) {
    switch (perturbation->type) {
        case PERTURBATION_L_ONE:
            fprintf(stream, "Perturbation L-1\n");
            return;

        case PERTURBATION_L_INF:
            fprintf(stream, "Perturbation L-inf\n");
            return;

        case PERTURBATION_HYPER_RECTANGLE:
            fprintf(stream, "Hyper rectangle perturbation.\n");
            return;

        case PERTURBATION_FRAME:
            fprintf(stream, "Occlusive frame perturbation\n");
            return;

        default:
            fprintf(stream, "Unrecognized perturbation (code: %u)\n", perturbation->type);
    }
}
