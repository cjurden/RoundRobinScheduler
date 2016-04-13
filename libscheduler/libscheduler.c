/** @file libscheduler.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libscheduler.h"
#include "../libpriqueue/libpriqueue.h"

/*
* Global variables to handle wait, response and turnaround time.
*/
float avgWaitTime, avgResponseTime, avgTurnaroundTie = 0.0;
int completed_jobs = 0;
int last_time = 0;
int no_cores_active = 0;
int wait_time, response_time, turnaround_time = 0;

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
    priqueue_t *queue;       // queue to hold jobs waiting
    int cores;              // number of cores for the simlator
    job_t** activeCores;    // job_t pointer array, with size == the number of cores specified.
}

/*
* Scheme, queue and struct to be used within all scheduling functions.
*/
scheme_t scheme = 0;
priqueue_t *q;
scheduler_t *s;

/*
* TODO Time helper function
*
* updates reaminign time of every item in the core array
* keeps track of last time time was updated
*/
void set_time(int time){
  int diff = time - last_time;
  for(int i = 0; i < no_cores_active; i ++) {
    s->activeCores[i]->remaining_time = s->activeCores[i]->remaining_time - diff;
    if(s->activeCores[i]->remaining_time <= 0) {
      s->activeCores[i]->finish_time = time + s->activeCores[i]->remaining_time;
    }
  }
  last_time = time;
}
/*
* Comparator function definitions.
*/

/*
* GLOBAL COMPARER TO PASS TO QUEUE INIT BASED ON parameters
*/

int compareFCFS(const void *elem1, const void *elem2){
  job_t* j1 = (job_t *)elem1;
  job_t* j2 = (job_t *)elem2;

  return j1->arrival_time - j2->arrival_time;
}

int compareSJF(const void *elem1, const void *elem2){
    job_t* j1 = (job_t *)elem1;
    job_t* j2 = (job_t *)elem2;

    /*
    * SJF scheme compares the remaining execution times
    * of the two jobs. If they equal, we defer the comparison
    * to the arrival times of each job.
    */

    //TODO make sure to check if job is running (if needed!!!)

    int val = j1->running_time - j2->running_time;
    if(val != 0){
        return val;
    }
    return j1->arrival_time - j2->arrival_time;
}

int comparePriority(const void *elem1, const void *elem2){
    job_t* j1 = (job_t *)elem1;
    job_t* j2 = (job_t *)elem2;

    /*
    * PRI scheme compares the priorities of the two jobs.
    * If they equal, we defer the comparison to the arrival
    * times of each job.
    */

    int val = j1->priority - j2->priority;
    if(val != 0){
        return val;
    }
    return j1->arrival_time - j2->arrival_time;
}

/**
  Initalizes the scheduler.

  Assumptions:
    - You may assume this will be the first scheduler function called.
    - You may assume this function will be called once once.
    - You may assume that cores is a positive, non-zero number.
    - You may assume that scheme is a valid scheduling scheme.

  @param cores the number of cores that is available by the scheduler. These cores will be known as core(id=0), core(id=1), ..., core(id=cores-1).
  @param scheme  the scheduling scheme that should be used. This value will be one of the six enum values of scheme_t
*/
void scheduler_start_up(int cores, scheme_t scheme)
{
    /*
    * Allocate memory for structure and initialize all variables.
    */
    s = malloc(sizeof(scheduler_t));
    s->activeCores = malloc(sizeof(job_t*)*(s->cores));
    s->scheme = scheme;
    s->cores = cores;

  /*
  * Note here that FCFS and RR use same comparator, along within
  * SJF and PSJF, and PRI and PPRI. The preemptive schemes
  * will use an additional function on top of the comparator.
  */
  switch(s->scheme){
    case FCFS :
        priqueue_init(s->queue, compareFCFS);
    case SJF  :
        priqueue_init(s->queue, compareSJF);
    case PSJF :
        priqueue_init(s->queue, compareSJF);
    case PRI  :
        priqueue_init(s->queue, comparePriority);
    case PPRI :
        priqueue_init(s->queue, comparePriority);
    case RR   :
        priqueue_init(s->queue, compareFCFS);
  }//end switch

  /*
  * initialize activeCores to NULL (i.e. no job running on them).
  */
  int count;
  for(count = 0; count < s->cores; count++){
      s->activeCores[count] = NULL;
  }
}//end scheduler start up

/**
  Called when we need to check for preemption on the active cores.

  If the scheme is PSJF or PPRI, we need to check if the newly received job can
  preempt any job currently executing on the active cores.

  @param scheme the scheme_t for the scheduler
  @param current the job currently running on the core
  @param new the job that can potentially preempt the current

  @return TRUE if we should preempt the current job
  @return FALSE if we should NOT preempt the current job
 */
 bool checkForPreemption(scheme_t scheme, job_t* current, job_t* new){
     if(scheme = PPRI){
         int prio = current->priority - new->priority;
         if(prio > 0){ //this means that the new job has higher prio
             return true;
         }
         if(prio < 0){ //this means that the current job has higher prio, should NOT preempt
             return false;
         }
         //if we get here, we know the jobs have equal prios. so we defer to arrival times
         int arrive = current->arrival_time - new->arrival_time;
         if(arrive > 0){ //this mean the new job arrived before current
             return true;
         }
         if(arrive <= 0){ //current job has earlier or equal arrival time, do NOT preempt
             return false;
         }
     }
     if(scheme = PSJF){
         int rem_time = current->remaining_time - new->remaining_time;
         if(rem_time > 0){ //this means that the new job has shorter remaining_time, preempt
             return true;
         }
         if(rem_time < 0){ //this means that the current job has shorter remaining time, should NOT preempt
             return false;
         }
         //if we get here, we know the jobs have equal prios. so we defer to arrival times
         int arrive = current->arrival_time - new->arrival_time;
         if(arrive > 0){ //this mean the new job arrived before current
             return true;
         }
         if(arrive <= 0){ //current job has earlier or equal arrival time, do NOT preempt
             return false;
         }
     }
 }

/**
  Called when a new job arrives.

  If multiple cores are idle, the job should be assigned to the core with the
  lowest id.
  If the job arriving should be scheduled to run during the next
  time cycle, return the zero-based index of the core the job should be
  scheduled on. If another job is already running on the core specified,
  this will preempt the currently running job.
  Assumptions:
    - You may assume that every job wil have a unique arrival time.

  @param job_number a globally unique identification number of the job arriving.
  @param time the current time of the simulator.
  @param running_time the total number of time units this job will run before it will be finished.
  @param priority the priority of the job. (The lower the value, the higher the priority.)
  @return index of core job should be scheduled on
  @return -1 if no scheduling changes should be made.

 */
int scheduler_new_job(int job_number, int time, int running_time, int priority)
{
  set_time(time);
  job_t *job = malloc(sizeof(job_t));
  job->pid = job_number;
  job->arrival_time = time;
  job->remaining_time = running_time;
  job->priority = priority;

    int count;
    for(count = 0; count < s->cores; count++){
        /*
        * iterate through cores and find the first available.
        */
        if(s->activeCores[count] == NULL){
            s->activeCores[count] = job;
            job->start_time = time;
            no_cores_active++;
            return count;
        }
    }
        /*
        * All cores are busy. Check if we can/should preempt a job on a core
        * given the scheme.
        */
        if(s->scheme == PSJF || s->scheme == PPRI){
            /*
            * we have a preemptive scheme. so we loop through the cores to
            */
            int count;
            for(count = 0; count < s->cores; count++){
                if(checkForPreemption(s->scheme, s->activeCores[count],job)==TRUE){
                    /*
                    * so we need to preempt. remove current job and put it on
                    * the queue, replace with new.
                    */
                    priqueue_offer(s->queue,s->activeCores[count]);
                    //NEED TO DO SOME WORK WITH RESPONSE AND WAITING TIME here
                    s->activeCores[count] = job;
                    job->response_time = time; //Note that this might be weird
                    return count;
                }
            }
        }//end preemptive if check
    /*
    * If we get here, we either have nothing to preempt or all cores are busy.
    * So, place the job on the queue.
    */
    priqueue_offer(s->queue,job);
    return -1;
}


/**
  Called when a job has completed execution.

  The core_id, job_number and time parameters are provided for convenience. You may be able to calculate the values with your own data structure.
  If any job should be scheduled to run on the core free'd up by the
  finished job, return the job_number of the job that should be scheduled to
  run on core core_id.

  @param core_id the zero-based index of the core where the job was located.
  @param job_number a globally unique identification number of the job.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled to run on core core_id
  @return -1 if core should remain idle.
 */
int scheduler_job_finished(int core_id, int job_number, int time)
{

  //update global time variables
  //free memory of core array
  //schedule next job
	return -1;
}


/**
  When the scheme is set to RR, called when the quantum timer has expired
  on a core.

  If any job should be scheduled to run on the core free'd up by
  the quantum expiration, return the job_number of the job that should be
  scheduled to run on core core_id.

  @param core_id the zero-based index of the core where the quantum has expired.
  @param time the current time of the simulator.
  @return job_number of the job that should be scheduled on core cord_id
  @return -1 if core should remain idle
 */
int scheduler_quantum_expired(int core_id, int time)
{
  //if no job waiting, return the current job id
  //if job waiting,
	return -1;
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
	return 0.0;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
	return 0.0;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
	return 0.0;
}


/**
  Free any memory associated with your scheduler.

  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{

}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled. Furthermore, we have also listed the current state of the job (either running on a given core or idle). For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority, and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{

}
