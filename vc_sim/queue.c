/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator - Message Queue Routines
 |    (C) Bill Dally, July 2, 1989
 */
/*---------------------------------------------------------------------------*/
#include "defs.h"
#include "queue.h"
/*---------------------------------------------------------------------------*/
QUEUE *free_queues ;
/*---------------------------------------------------------------------------*/
void init_queues()
{
  free_queues = NULL_QUEUE ;
}
/*---------------------------------------------------------------------------*/
void free_queue(QUEUE *queue)
{
  queue->next = free_queues ;
  free_queues = queue ;
}
/*---------------------------------------------------------------------------*/
/* returns cells en-masse */
void clear_queue(QUEUE *queue)
{
  if(queue->last) {
    queue->last->next = free_queues ;
    free_queues = queue->next ;
    queue->last = queue->next = NULL_QUEUE ;
  }
}
/*---------------------------------------------------------------------------*/
int queue_empty(QUEUE *queue)
{
  return(!(queue->last)) ;
}
/*---------------------------------------------------------------------------*/
void enqueue(QUEUE *queue, char *data)
{
  QUEUE *cell ;
  cell = new_queue() ;
  cell->data = data ;
  if(queue->last) { /* not empty */
    cell->last = queue->last ;
    queue->last->next = cell ;
    queue->last = cell ;
  }
  else {
    queue->last = queue->next = cell ;
  }
}
/*---------------------------------------------------------------------------*/
char *dequeue(QUEUE *queue)
{
  char *data ;
  QUEUE *cell ;
  if(queue->next) { /* not empty */
    cell = queue->next ;	/* unlink */
    queue->next = cell->next ;
    if(queue->next) { /* not last element */
      queue->next->last = NULL_QUEUE ;
    }
    else { /* last element */
      queue->last = NULL_QUEUE ;
    }
    data = cell->data ;
    free_queue(cell) ;
    return(data) ;
  }
  else {
    printf("Attempt to dequeue from empty queue\n") ; exit(6) ; 
  }
}
/*---------------------------------------------------------------------------*/
QUEUE *new_queue()
{
  QUEUE *q ;
  if(free_queues) {
    q = free_queues ;
    free_queues = free_queues->next ; 
  }
  else {
    q = NEW_STRUCT(QUEUE) ;
  }
  q->next = q->last = NULL_QUEUE ; q->data = (char *)0 ;
  return(q) ;
}
/*---------------------------------------------------------------------------*/

