/** @file libscheduler.h
 */
#include "../libpriqueue/libpriqueue.h"
#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_


/*
* Constants which represent the different scheduling algorithms
*/
typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;

/*
* Comparator functions for the different schemes.
*/
int   compareFCFS                      (const void *elem1, const void *elem2);
int   compareSJF                       (const void* elem1, const void* elem2);
int   comparePriority                  (const void* elem1, const void* elem2);

/*
* Pre-defined functions.
*/
void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int job_number, int time);
int   scheduler_quantum_expired        (int core_id, int time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();

void  scheduler_show_queue             ();

/*
* Additional helper functions.
*/
void  checkCores                       ();
bool  checkForPreemption                (scheme_t scheme, job_t* current, job_t* new);
void  displayJobStats                  (job_t* job, int time);


#endif /* LIBSCHEDULER_H_ */
