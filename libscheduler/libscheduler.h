/** @file libscheduler.h
 */
#include "../libpriqueue/libpriqueue.h"
#ifndef LIBSCHEDULER_H_
#define LIBSCHEDULER_H_

/**
  Stores information making up a job to be scheduled including any statistics.
  You may need to define some global variables or a struct to store your job queue elements.
*/
typedef struct _job_t
{
  int pid;                // unique job identifier
  int priority;           // job priority. 0 is highest.

  int arrival_time;       //time job was received.
  int remaining_time;     //will be initialized to job length, and will be possibly be updated when preemptive schemes are used.

  int start_time;         //time job started executing
  int finish_time;        //time job finished executing

  int waiting_time;       //how long the job spends waiting in the queue until execution
  int response_time;      //amount of time it takes from when a job was first submitted until the scheduler produces first response
} job_t;


/*
* scheduler struct
*/
typedef struct _scheduler_t {
    scheme_t scheme;        // scheme to be used (will decided comparator function)
    priqueue_t queue;       // queue to hold jobs waiting
    int cores;              // number of cores for the simlator
    job_t** activeCores;    // job_t pointer array, with size == the number of cores specified.
}

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
int   checkForPremption                (job_t* current, job_t* new, scheme_t scheme);
void  displayJobStats                  (job_t* job, int time);


#endif /* LIBSCHEDULER_H_ */
