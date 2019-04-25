/**
 * Stopwatch to measure time.
 * 
 * Defines a stopwatch utility which can be used to measure time between
 * two events.
 * 
 * @file stopwatch.h
 */
#ifndef STOPWATCH_H
#define STOPWATCH_H

/**
 * Type of a stopwatch.
 */
typedef struct stopwatch *Stopwatch;


/**
 * Creates a stopwatch.
 * 
 * @return New stopwatch
 */
Stopwatch stopwatch_create();


/**
 * Deletes a stopwatch.
 * 
 * @param[out] stopwatch Pointer to stopwatch
 */
void stopwatch_delete(Stopwatch *stopwatch);


/**
 * Resgisters start of an event.
 * 
 * @param[out] stopwatch Stopwatch
 * @return The stopwatch itself
 */
Stopwatch stopwatch_start(Stopwatch stopwatch);


/**
 * Registers end of an event.
 * 
 * @param[out] stopwatch Stopwatch
 * @return The stopwatch itself
 */
Stopwatch stopwatch_stop(Stopwatch stopwatch);


/**
 * Returns milliseconds between start and end event.
 * 
 * @param[in] stopwatch Stopwatch
 * @return Milliseconds between start and end event
 * @note Returns a negative value is stopwatch_start has been called
 *       after stopwatch_stop on given stopwatch.
 * @warning Returns an undefined result if either stopwatch_start or
 *          stopwatch_stop have not been previously called on given
 *          stopwatch.
 */
double stopwatch_get_elapsed_milliseconds(const Stopwatch stopwatch);

#endif
