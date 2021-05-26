/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator
 |    Event List Routines
 |    (C) Bill Dally, 29-May-1989
 */
/*---------------------------------------------------------------------------*/
#include "defs.h"
#include "netdef.h"
#include "event.h"
/*---------------------------------------------------------------------------*/
/* for bookkeeping */
int nr_events ;
/*---------------------------------------------------------------------------*/
EVENT *new_event(void (*funct)(), SOURCE *source)
{
  EVENT *event ;

  if(free_events) {
    event = free_events ;
    free_events = free_events->next ;
  }
  else {
    event = NEW_STRUCT(EVENT) ;
    nr_events++ ; 
  }
  event->next = NULL_EVENT ;
  event->funct =  funct ;
  event->source = source ;
  return(event) ;
}
/*---------------------------------------------------------------------------*/
void free_event(EVENT *event)
{
  event->next = free_events ;
  free_events = event ;
}
/*---------------------------------------------------------------------------*/
void free_list_of_events(EVENT *events) 
{
  EVENT *event, *next_event ;  

  event = events ;
  while(event) {
    next_event = event->next ;
    free_event(event) ;
    event = next_event ;
  }
}
/*---------------------------------------------------------------------------*/
void init_event_list() 
{
  event_list = NEW_ARRAY(TIMESTEP, NR_TIMESTEPS) ;
  free_events = NULL_EVENT ;
  nr_events = 0 ;
  reset_event_list() ;
}
/*---------------------------------------------------------------------------*/
void check_events()
{
  EVENT *event ;
  int i ;

  /* check inventory of events */
  i = 0 ;
  printf("checking for %d events \n",nr_events) ; 
  FOREACH(event, free_events) {
    i++ ;
  }
  if(i != nr_events) {
    printf("EVENTS LOST: %d found %d expected\n",i,nr_events) ;
  }
}
/*---------------------------------------------------------------------------*/
void reset_event_list()
{
  EVENT *event, *next_event ;
  int i ;

  cur_time = 0 ;
  cur_index = 0 ;
  for(i=0;i<NR_TIMESTEPS;i++) {
    free_list_of_events(event_list[i]) ;
    event_list[i] = NULL_EVENT ;
  }
  check_events() ;
}
/*---------------------------------------------------------------------------*/
void schedule_event(int time, EVENT *event)
{
  int slot ;
  EVENT *e ;

  slot = time-cur_time ;
  if((slot < NR_TIMESTEPS) && (slot > 0)) {
    slot = time % NR_TIMESTEPS ;
    event->next = event_list[slot] ;
    event_list[slot] = event ;
    if(debug & DEBUG_EVENTS) {
      printf("SCHEDULING EVENT time = %d index = %d\n", time, slot);
      FOREACH(e, event_list[slot]) {
	printf("EVENT %d(%d)\n",(int)e->funct,(int)e->source->node->id_seq) ;
      }
    }
  }
  else {
    printf("Event list overflow\n") ;
    exit(5) ;
  }
}
/*---------------------------------------------------------------------------*/
void process_events()
{
  EVENT *events ;
  EVENT *event ;

  events = event_list[cur_index] ;
  event_list[cur_index] = NULL_EVENT ;

  if(debug & DEBUG_EVENTS) {
    printf("PROCESSING EVENTS time = %d index = %d\n",cur_time,cur_index);
  }

  FOREACH(event, events) {
    if(debug & DEBUG_EVENTS) {
      printf("EVENT_S %d(%d)\n",(int)event->funct,
	                      (int)event->source->node->id_seq) ;
    }
    (*event->funct)(event->source) ;
  }
  free_list_of_events(events) ;
}
/*---------------------------------------------------------------------------*/
