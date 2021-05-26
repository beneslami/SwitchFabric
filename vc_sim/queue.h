/*---------------------------------------------------------------------------*/
struct queue {
  struct queue *next ;
  struct queue *last ;
  char *data ;
} ;
typedef struct queue QUEUE ;
/*---------------------------------------------------------------------------*/
#define NULL_QUEUE ((QUEUE *)0)
/*---------------------------------------------------------------------------*/
extern void init_queues() ;
extern int queue_empty(QUEUE *queue) ;
extern void enqueue(QUEUE *queue, char *data) ;
extern char *dequeue(QUEUE *queue) ;
extern QUEUE *new_queue() ;
/*---------------------------------------------------------------------------*/
