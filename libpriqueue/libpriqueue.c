/** @file libpriqueue.c
 */

#include <stdlib.h>
#include <stdio.h>

#include "libpriqueue.h"


/**
  Initializes the priqueue_t data structure.

  Assumtions
    - You may assume this function will only be called once per instance of priqueue_t
    - You may assume this function will be the first function called using an instance of priqueue_t.
  @param q a pointer to an instance of the priqueue_t data structure
  @param comparer a function pointer that compares two elements.
  See also @ref comparer-page
 */
void priqueue_init(priqueue_t *q, int(*comparer)(const void *, const void *))
{
    q->size=0;
    q->comp = comparer;
    q->head=NULL;
}


/**
  Inserts the specified element into this priority queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr a pointer to the data to be inserted into the priority queue
  @return The zero-based index where ptr is stored in the priority queue, where 0 indicates that ptr was stored at the front of the priority queue.
 */
int priqueue_offer(priqueue_t *q, void *ptr)
{
    /*
    * Create new qnode_t item and populate
    */
    qnode_t *node = malloc(sizeof(qnode_t));
    node->item = ptr;
    node->next = NULL;

    /*
    * Base Case: Queue is empty
    */
    if(q->head==NULL){
        q->head= node;
        q->size++;
        return 0;
    }

    int count = 0;
    qnode_t *cur = q->head;
    qnode_t *prev = NULL;
    while(cur != NULL){

        /*
        * Use the comp function to see if node is of higher priority than the current item.
        * If so, insert in between the prev and curr.
        */
        if(q->comp(cur->item, node->item) > 0){
            if(prev != NULL){
                prev->next = node;
            }
            if(count == 0){
                q->head = node;
            }
            return count;
        }

        /*
        * If node is not of higher priority, keep traversing the queue.
        * Advance prev, cur, increase count.
        */
        count++;
        prev = cur;
        cur = cur->next;
    }

    /*
    * If this point is reached, node is to be inserted at the end of the queue.
    */
    prev->next = node;
    q->size++;
    return q->size-1;
}


/**
  Retrieves, but does not remove, the head of this queue, returning NULL if
  this queue is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return pointer to element at the head of the queue
  @return NULL if the queue is empty
 */
void *priqueue_peek(priqueue_t *q)
{
    if(q->size == 0){
        return NULL;
    }
    return q->head->item;
}


/**
  Retrieves and removes the head of this queue, or NULL if this queue
  is empty.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the head of this queue
  @return NULL if this queue is empty
 */
void *priqueue_poll(priqueue_t *q)
{
    if(q->size == 0){
        return NULL;
    }

    /*
    * Save the job item from the head of the queue, and save the head's next item.
    * dealloc the memory and set head to equal next item
    */
    void *job = q->head->item;
    qnode_t *temp = q->head->next;
    free(q->head);
    q->head = temp;

    q->size--;
    return job;
}


/**
  Returns the element at the specified position in this list, or NULL if
  the queue does not contain an index'th element.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of retrieved element
  @return the index'th element in the queue
  @return NULL if the queue does not contain the index'th element
 */
void *priqueue_at(priqueue_t *q, int index)
{
    /*
    * Base case, queue is empty or index is out of range
    */
	if(q->size == 0 || q->size < index+1){
        return NULL;
    }

    int counter = 0;
    qnode_t *cur = q->head;

    while(counter != index){
        cur = cur->next;
        counter++;
    }

    void *node = cur->item;
}


/**
  Removes all instances of ptr from the queue.

  This function should not use the comparer function, but check if the data contained in each element of the queue is equal (==) to ptr.

  @param q a pointer to an instance of the priqueue_t data structure
  @param ptr address of element to be removed
  @return the number of entries removed
 */
int priqueue_remove(priqueue_t *q, void *ptr)
{
    if(q->size == 0){
        return 0;
    }

    int num_removed = 0;

    qnode_t *cur = q->head;
    qnode_t *prev = NULL;

    while(cur != NULL){
        //check to see if data matches
        //if so, free the memory and adjust queue
    }
}


/**
  Removes the specified index from the queue, moving later elements up
  a spot in the queue to fill the gap.

  @param q a pointer to an instance of the priqueue_t data structure
  @param index position of element to be removed
  @return the element removed from the queue
  @return NULL if the specified index does not exist
 */
void *priqueue_remove_at(priqueue_t *q, int index)
{
	return 0;
}


/**
  Returns the number of elements in the queue.

  @param q a pointer to an instance of the priqueue_t data structure
  @return the number of elements in the queue
 */
int priqueue_size(priqueue_t *q)
{
	return q->size;
}


/**
  Destroys and frees all the memory associated with q.

  @param q a pointer to an instance of the priqueue_t data structure
 */
void priqueue_destroy(priqueue_t *q)
{

}
