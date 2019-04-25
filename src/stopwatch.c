#include "stopwatch.h"

#include <malloc.h>
#include <sys/time.h>

#include "report_error.h"


/**
 * Structure of a stopwatch.
 */
struct stopwatch {
    struct timeval start_time;  /**< Start time. */
    struct timeval stop_time;   /**< End time. */
};


Stopwatch stopwatch_create() {
    Stopwatch stopwatch = (Stopwatch) malloc(sizeof(struct stopwatch));

    if (!stopwatch) {
        report_error("Cannot allocate memory.");
    }

    return stopwatch;
}


void stopwatch_delete(Stopwatch *stopwatch) {
    if (stopwatch == NULL || *stopwatch == NULL) {
        return;
    }

    free(*stopwatch);
    *stopwatch = NULL;
}



Stopwatch stopwatch_start(Stopwatch stopwatch) {
    gettimeofday(&stopwatch->start_time, NULL);

    return stopwatch;
}



Stopwatch stopwatch_stop(Stopwatch stopwatch) {
    gettimeofday(&stopwatch->stop_time, NULL);

    return stopwatch;
}



double stopwatch_get_elapsed_milliseconds(const Stopwatch stopwatch) {
    return (stopwatch->stop_time.tv_sec - stopwatch->start_time.tv_sec) * 1e3
         + (stopwatch->stop_time.tv_usec - stopwatch->start_time.tv_usec) * 1e-3;
}
