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
  int pid;
  int priority;
  int run_time;
  int arrival_time;
  int started;
  int finished;
  int running_core;
  int remaining_time;     //used in preemption - keep track of when it stops

  int waiting_time;       //how long the job spends waiting in the queue until execution
  int response_time;      //amount of time it takes from when a job was first submitted until the scheduler produces first response
  int tunraround_time;    //amount of time to execute a particular job
} job_t;


/*
* scheduler struct
*/
typedef struct _scheduler_t {
    scheme_t scheme;
    priqueue_t queue;
    int cores;                   // number of cores for the simlator
    bool *cores_availabile;      // bool array to remember which cores are available for new jobs
}

/*
* Constants which represent the different scheduling algorithms
*/
typedef enum {FCFS = 0, SJF, PSJF, PRI, PPRI, RR} scheme_t;

int   compareArrival                   (const void* elem1, const void* elem2);
int   comparePriority                  (const void* elem1, const void* elem2);
int   compareSJF                       (const void* elem1, const void* elem2);
int   comparePSJF                      (const void* elem1, const void* elem2);
int   comparePPriority                 (const void* elem1, const void* elem2);
int   compareRR                        (const void* elem1, const void* elem2);

void  scheduler_start_up               (int cores, scheme_t scheme);
int   scheduler_new_job                (int job_number, int time, int running_time, int priority);
int   scheduler_job_finished           (int core_id, int job_number, int time);
int   scheduler_quantum_expired        (int core_id, int time);
float scheduler_average_turnaround_time();
float scheduler_average_waiting_time   ();
float scheduler_average_response_time  ();
void  scheduler_clean_up               ();

void  scheduler_show_queue             ();

#endif /* LIBSCHEDULER_H_ */
