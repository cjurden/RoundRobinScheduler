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
float avgWaitTime, avgResponseTime, avgTurnaroundTime = 0.0;
int completed_jobs = 0;

int last_time = 0;
int no_cores_active = 0;
int wait_time, response_time, turnaround_time = 0;


/*
* Scheme, queue and struct to be used within all scheduling functions.
*/
scheme_t scheme = 0;
scheduler_t *s;

/*
* updates reaminign time of every item in the core array
* keeps track of last time time was updated
*/
void set_time(int time){
  for(int i = 0; i < s->cores; i ++) {
    if(s->activeCores[i] != NULL){
      s->activeCores[i]->remaining_time -= (time - s->activeCores[i]->update_time);
      s->activeCores[i]->update_time = time;
      printf("\n\nRemaining time for Job %d: %d\n", s->activeCores[i]->pid, s->activeCores[i]->remaining_time);
    }
  }
}

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
    int val = j1->remaining_time - j2->remaining_time;
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

int compareRR(const void *elem1, const void *elem2){
    void *ptr = &elem1;
    ptr = &elem2;
  return 0;
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
        priqueue_init(&s->queue, compareFCFS);
    case SJF  :
        priqueue_init(&s->queue, compareSJF);
    case PSJF :
        priqueue_init(&s->queue, compareSJF);
    case PRI  :
        priqueue_init(&s->queue, comparePriority);
    case PPRI :
        priqueue_init(&s->queue, comparePriority);
    case RR   :
        priqueue_init(&s->queue, compareRR);
  }//end switch

  /*
  * initialize activeCores to NULL (i.e. no job running on them).
  */
  int count;
  for(count = 0; count < s->cores; count++){
      s->activeCores[count] = NULL;
  }

  printf("SUCCESS SCHEDULE STARTUP");
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
     if(scheme == PPRI){
         int prio = current->priority - new->priority;
         if(prio > 0){ //this means that the new job has higher prio
             return TRUE;
         }
         if(prio < 0){ //this means that the current job has higher prio, should NOT preempt
             return FALSE;
         }
         //if we get here, we know the jobs have equal prios. so we defer to arrival times
         int arrive = current->arrival_time - new->arrival_time;
         if(arrive > 0){ //this mean the new job arrived before current
             return TRUE;
         }
         if(arrive <= 0){ //current job has earlier or equal arrival time, do NOT preempt
             return FALSE;
         }
     }
     if(scheme == PSJF){
         int rem_time = current->remaining_time - new->remaining_time;
         if(rem_time > 0){ //this means that the new job has shorter remaining_time, preempt
             return TRUE;
         }
         if(rem_time < 0){ //this means that the current job has shorter remaining time, should NOT preempt
             return FALSE;
         }
         //if we get here, we know the jobs have equal prios. so we defer to arrival times
         int arrive = current->arrival_time - new->arrival_time;
         if(arrive > 0){ //this mean the new job arrived before current
             return TRUE;
         }
         if(arrive <= 0){ //current job has earlier or equal arrival time, do NOT preempt
             return TRUE;
         }
     }
     return FALSE;
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
  printf("\n\nscheduling new job! number: %d\n\n", job_number);
    job_t *job = malloc(sizeof(job_t));
    job->pid = job_number;
    job->arrival_time = time;
    job->remaining_time = running_time;
    job->priority = priority;
    set_time(time);
      int count;
      for(count = 0; count < s->cores; count++){
          /*
          * iterate through cores and find the first available.
          */
          if(s->activeCores[count] == NULL){
              s->activeCores[count] = job;
              job->start_time = time;
              no_cores_active += 1;
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
              if(checkForPreemption(s->scheme, s->activeCores[count],job) == TRUE){
                  /*
                  * so we need to preempt. remove current job and put it on
                  * the queue, replace with new.
                  */
                  priqueue_offer(&s->queue,s->activeCores[count]);
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
      set_time(time);
      priqueue_offer(&s->queue,job);
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
  /*
  * update global time variables, free memory of core array, schedule next job
  */
  printf("\n\nSCHEDULER FINISHED. JOB_ID: %d\n\n", job_number);
  set_time(time);
  completed_jobs++;
  int t_time = (time - s->activeCores[core_id]->arrival_time);
  &turnaround_time = t_time;
  printf("\n\n[scheduler_job_finished] s->activeCores[core_id]->waiting_time: %d\n\n", s->activeCores[core_id]->waiting_time);
  int w_time = time - s->activeCores[core_id]->waiting_time;
  printf("\n\n[scheduler_job_finished] wait time: %d\n\n", w_time);
  &wait_time +=
  free(s->activeCores[core_id]);

  //check to see if another job is ready to hop on the core
  if(s->queue.head != NULL){
      s->activeCores[core_id] = priqueue_poll(&s->queue);
      s->activeCores[core_id]->waiting_time += (time - s->activeCores[core_id]->update_time);
      s->activeCores[core_id]->update_time = time;
      return s->activeCores[core_id]->pid;
  }

  //queue is empty, core should remain idle.
  s->activeCores[core_id]=NULL;
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
    /*
    * update the remaining time for the job currently running on the core
    */
    s->activeCores[core_id]->remaining_time -= time - s->activeCores[core_id]->update_time;
    s->activeCores[core_id]->update_time = time;

    /*
    * See if that job has completed running. If so, free it
    */
    if(s->activeCores[core_id]->remaining_time <= 0){
        //set_time(time); here
        free(s->activeCores[core_id]);
        s->activeCores[core_id] = NULL;
    }
    else{
        priqueue_offer(&s->queue, s->activeCores[core_id]);
        s->activeCores[core_id] = NULL;
    }

    //if no job waiting, return the current job id
    //if job waiting,
    if(priqueue_size(&s->queue) != 0){
        job_t *new = priqueue_poll(&s->queue);
        new->waiting_time += time - new->update_time;

        //do some work with time_responded to here

        return new->pid;
    }
    return -1;

    /*
        //set_time(time);
  if(s->queue.head == NULL) {
    return((int)s->activeCores[core_id]->pid);
  }
  if(s->queue.head != NULL) {
    priqueue_offer(&s->queue, (void *)&s->activeCores[core_id]);
    job_t *tmp = (job_t *)(&s->queue.head->item);
    wait_time += (int)tmp->waiting_time;
    s->activeCores[core_id] = (job_t *)priqueue_poll(&s->queue);
    return (int)s->activeCores[core_id]->pid;
  }
	return -1;

    */
}


/**
  Returns the average waiting time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average waiting time of all jobs scheduled.
 */
float scheduler_average_waiting_time()
{
  printf("wait time: %d", wait_time);
  avgWaitTime = (float) (wait_time / completed_jobs);
  return avgWaitTime;
}


/**
  Returns the average turnaround time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average turnaround time of all jobs scheduled.
 */
float scheduler_average_turnaround_time()
{
    return avgTurnaroundTime;
  //avgTurnaroundTie = (float)(turnaround_time / completed_jobs);
	//return 0.0;
}


/**
  Returns the average response time of all jobs scheduled by your scheduler.

  Assumptions:
    - This function will only be called after all scheduling is complete (all jobs that have arrived will have finished and no new jobs will arrive).
  @return the average response time of all jobs scheduled.
 */
float scheduler_average_response_time()
{
  avgResponseTime = (float)(response_time / completed_jobs);
	return avgResponseTime;
}


/**
  Free any memory associated with your scheduler.
  Assumptions:
    - This function will be the last function called in your library.
*/
void scheduler_clean_up()
{
    priqueue_destroy(&s->queue);
    free(s->activeCores);
    free(s);

    /*
  if(s->queue.head != NULL){
    qnode_t *tmp = (qnode_t *)(&s->queue.head);
    qnode_t *temp = tmp->next;
    while(tmp->next != NULL) {
      free(&s->queue.head->item);
      s->queue.head->item = temp;
      qnode_t *temp = tmp->next;
    }
    free(tmp);
    free(temp);
  }
  int i = 0;
  while(s->activeCores[i] != NULL){
    free(s->activeCores[i]);
    i++;
  }
  free(s);
  */
}


/**
  This function may print out any debugging information you choose. This
  function will be called by the simulator after every call the simulator
  makes to your scheduler.
  In our provided output, we have implemented this function to list the jobs in the order they are to be scheduled.
  Furthermore, we have also listed the current state of the job (either running on a given core or idle).
  For example, if we have a non-preemptive algorithm and job(id=4) has began running, job(id=2) arrives with a higher priority,
  and job(id=1) arrives with a lower priority, the output in our sample output will be:

    2(-1) 4(0) 1(-1)

  This function is not required and will not be graded. You may leave it
  blank if you do not find it useful.
 */
void scheduler_show_queue()
{
    qnode_t *current = s->queue.head;
    int pid;
    int prio;
    int time_rem;

    /*
    * Iterate through the queue printing either Prio or Time remaining
    */
    int i = 0;
    for(i; i<s->queue.size; i++){
        pid = ((job_t *)current->item)->pid;
        if(s->scheme == SJF || s->scheme == PSJF){
            time_rem = ((job_t *)current->item)->remaining_time;
            printf("PID: %d | TIME_REM: %d ",pid,time_rem);
        }
        else{
            prio = ((job_t *)current->item)->priority;
            printf("PID: %d | PRIO: %d ",pid,prio);
        }
        current = current->next;
    }
}
