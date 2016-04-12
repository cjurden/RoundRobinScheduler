/** @file libpriqueue.h
 */

#ifndef LIBPRIQUEUE_H_
#define LIBPRIQUEUE_H_

/*
* Priqueue Qnode Structure - an item in the queue.
*/
typedef struct _qnode_t
{
    void *item;                 //item containing data (in this case, will be a job struct).
    struct _qnode_t *next;       //next node in the queue.
} qnode_t;

/*
* Priqueue Data Structure
*/
typedef struct _priqueue_t
{
    int size;                                       //size of the queue.
    int(*comp)(const void*,const void*);            //generic comparer function (dependent upon scheme used).
    qnode_t *head;                                  //pointer to first element of the queue.
} priqueue_t;

void   priqueue_init     (priqueue_t *q, int(*comparer)(const void *, const void *));

int    priqueue_offer    (priqueue_t *q, void *ptr);
void * priqueue_peek     (priqueue_t *q);
void * priqueue_poll     (priqueue_t *q);
void * priqueue_at       (priqueue_t *q, int index);
int    priqueue_remove   (priqueue_t *q, void *ptr);
void * priqueue_remove_at(priqueue_t *q, int index);
int    priqueue_size     (priqueue_t *q);

void   priqueue_destroy  (priqueue_t *q);

#endif /* LIBPQUEUE_H_ */
