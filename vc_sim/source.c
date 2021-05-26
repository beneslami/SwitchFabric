/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator
 |    Routines for message sources
 |    (C) Bill Dally, July 2, 1989 
 */
/*---------------------------------------------------------------------------*/
#include "defs.h"
#include "netsim.h"
#include "message.h"
#include "event.h"
#include "stats.h"
#include "source.h"
#include "queue.h"
/*---------------------------------------------------------------------------*/
/* compute destination */
int source_dest(SOURCE *source)
{
  int dest ;
  dest = random() % nr_sources ;
  return(dest) ;
}
/*---------------------------------------------------------------------------*/
MESSAGE *new_source_message(SOURCE *source)
{
  int dest ; 
  MESSAGE *message ;

  dest = source_dest(source) ;
  message = new_message(dest, source->length) ;
  /* for voice/data sources with spct high priority voice messages */
  if((stype == S_VOICE_DATA ) && (spct <= (random() % 100))) {
    message->priority = LOW_PRIORITY ;
  }
  if(debug & DEBUG_SOURCING) {
    printf("NEW MESSAGE P = %d\n", message->priority) ;
  }
  return(message) ;
}
/*---------------------------------------------------------------------------*/
int source_message_pending(SOURCE *source)
{
  return((source->sat) || (!queue_empty(source->message_queue))) ;
}
/*---------------------------------------------------------------------------*/
void dequeue_source_to_lane(SOURCE *source, LANE *lane)
{
  MESSAGE *message ;

  /* if drain switch on do nothing */
  if(drain_messages) return ;

  if(source->sat) {
    message = new_source_message(source) ;
  }
  else { 
    message = (MESSAGE *)dequeue(source->message_queue) ;
  }
  assign_initial_lane_to_message(lane, message) ;
}
/*---------------------------------------------------------------------------*/
void defer_source_message(SOURCE *source, MESSAGE *message)
{
  enqueue(source->message_queue, (char *)message) ;
}
/*---------------------------------------------------------------------------*/
void schedule_source_event(SOURCE *source)
{
  int time ;
  double prob ;
  if(expsrc) { /* exp dist */
    prob = drand48() ;
    time = cur_time + 1 + (int)(-log(prob)/rate) ;
  }
  else {
    time = cur_time + 1 + (random() % (2*source->interval)) ;
  }
  schedule_source_event_at_time(source, time) ;
}
/*---------------------------------------------------------------------------*/
void schedule_source_event_at_time(SOURCE *source, int time)
{
  EVENT *event ;
  if(debug & DEBUG_SOURCING) {
    printf("Scheduling source %d.%d at time %d\n",
	   SOURCE_NODE_NR(source), SOURCE_SLOT(source), time) ;
  }
  event = new_event(source_event_funct, (char *)source) ;
  schedule_event(time, event) ;
}
/*---------------------------------------------------------------------------*/
void source_event_funct(char *src) /* typing for generic event */
{
  SOURCE *source = (SOURCE *)src ;
  EVENT *event  ;
  CHANNEL *channel ;
  MESSAGE *message ;
  LANE *lane ;
  int time, lane_nr, dest ;

  channel = source->dst ;
  if(debug & DEBUG_SOURCING) {
    printf("Source Event on %d.%d\n",
	   SOURCE_NODE_NR(source), SOURCE_SLOT(source)) ;
  }

  /* first, schedule next injection */
  schedule_source_event(source) ;

  /* if drain switch on do nothing */
  if(drain_messages) return ;


  /* find first non-busy lane */
  lane_nr = first_free_lane_nr(channel) ;
  if(lane_nr < nr_lanes) {
    lane = &(channel->lanes[lane_nr]) ;
    message = new_source_message(source) ;
    assign_initial_lane_to_message(lane, message) ;
  }
  else {
    /* if saturation source, do not defer */
    if(!source->sat) {
      message = new_source_message(source) ;
      defer_source_message(source, message) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void schedule_initial_source_events()
{
  int i ;

  for(i=0;i<nr_sources;i++) {
    schedule_source_event(&(sources[i])) ;
  }
}
/*---------------------------------------------------------------------------*/
void free_messages_in_queue(SOURCE *source)
{
  MESSAGE *message ;
  while(!queue_empty(source->message_queue)) {
    message = (MESSAGE *)dequeue(source->message_queue) ;
    free_message(message) ;
  }
}
/*---------------------------------------------------------------------------*/

