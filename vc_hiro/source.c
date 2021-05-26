/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator Network_sim - "source.c"
 |    Routines for message sources
 |    Hiromichi Aoki, July 27, 1990 
 */
/*---------------------------------------------------------------------------*/
#include "math.h"
#include "defs.h"
#include "netdef.h"
#include "message_route.h"
#include "routing_func.h"
#include "event.h"
#include "stats.h"
#include "source.h"
#include "queue.h"

/*---------------------------------------------------------------------------*/
/*
 |  Function Declalation
 */
/*---------------------------------------------------------------------------*/
    void Event_Funct() ;
    void defer_source_message() ;
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* 
 | message destination detemining function
 */
/*---------------------------------------------------------------------------*/
void init_source_destinations(SOURCE *sources) 
{
  int i ;
  for(i=0; i<nr_sources; i++) {
    sources[i].dest_node  = &(nodes[source_dest(&(sources[i]))]) ;
  }
}
/*---------------------------------------------------------------------------*/
int source_dest(SOURCE *source)
{
  int dest ;
  if(para->source_type == RANDOM) {
    dest =source_dest_random(source) ;
  }
  else if(para->source_type == BIT_REVERSAL) {
    dest = source_dest_bit_reversal(source) ;
  }
  else if(para->source_type == ROW_LINE) {
    dest = source_dest_row_line(source) ;
  }
  else if(para->source_type == REFLECTION) {
    dest = source_dest_reflection(source) ;
  }
  else error("error at selection of source dest algorithm") ;
  source->type = para->source_type ;
  return(dest) ;
}
/*---------------------------------------------------------------------------*/
int source_dest_reflection(SOURCE *source)
{
  int i, data, dest_seq_id, a, nr_ary, nr_dim ;
  double p_move_d ;

  if(source->node->state == DEAD) return(source->node->id_seq) ;
  nr_ary = para->k_array ;
  nr_dim = para->n_cube ;
  /* create source address array */ 
  p_move_d = ((double)nr_ary - 1)/2 ;
  for(i=0; i<nr_dim; i++) {
    data = source->node->id_dimensions[i].value ;
    /* operate matrix - pallarel movement & rotate by pai & recover  */
    /* to the original coordinate                                    */
    a = rint(-(data - p_move_d) + p_move_d) ;  
    temp[0][i].value = a ;
  }
  dest_seq_id = 0;
  for(i=0; i<nr_dim; i++) {
    dest_seq_id += temp[0][i].value * power(nr_ary, i) ;
  }
  if((&nodes[dest_seq_id])->state == DEAD) return(source->node->id_seq) ;
  return(dest_seq_id) ;
}
/*---------------------------------------------------------------------------*/
int source_dest_row_line(SOURCE *source)
{
  int i, data, dest_seq_id, a, nr_ary, nr_dim ;

  if(source->node->state == DEAD) return(source->node->id_seq) ;
  nr_ary = para->k_array ;
  nr_dim = para->n_cube ;
  /* create source address array */ 
  for(i=0; i<nr_dim; i++) {
    data = source->node->id_dimensions[i].value ;
    temp[0][nr_dim-1 -i].value = data ;
  }
  dest_seq_id = 0;
  for(i=0; i<nr_dim; i++) {
    dest_seq_id += temp[0][i].value * power(nr_ary, i) ;
  }
  if((&nodes[dest_seq_id])->state == DEAD) return(source->node->id_seq) ;
  return(dest_seq_id) ;
}
/*---------------------------------------------------------------------------*/
int source_dest_random(SOURCE *source)
{
  int dest ;

  if(source->node->state == DEAD) return(source->node->id_seq) ;
  dest = random() % nr_sources ;
  if((&nodes[dest])->state == DEAD) return(source->node->id_seq) ;
  return(dest) ;
}
/*---------------------------------------------------------------------------*/
int source_dest_bit_reversal(SOURCE *source)
{
  int i, nr_weight, excess, end, src_node_id, dest_id, nr_array ;
  double nr_weight_d ;
  int array[20], reverse[20] ;

  if(source->node->state == DEAD) return(source->node->id_seq) ;
  nr_weight = end = nr_nodes-1 ;
  for(i=0; end; i++) {
    nr_weight_d = (double)nr_weight/2 ;
    nr_weight = (int)floor(nr_weight_d) ;
    if(nr_weight_d < 1) end = 0 ;
    else end = 1; 
  }
  nr_array = i ;
  nr_weight_d = source->node->id_seq ;
  for(i=0; i<nr_array; i++) {
    nr_weight = (int)nr_weight_d/2 ;
    excess = (int)nr_weight_d%2 ;
    nr_weight_d = (double)nr_weight ;
    array[i] = excess ;
    reverse[nr_array-i-1] = excess ;
  }
  dest_id = 0 ;
  for(i=0; i<nr_array; i++) {
    dest_id = dest_id + reverse[i]*(1<<i) ;
  }
  if((&nodes[dest_id])->state == DEAD) return(source->node->id_seq) ;
  if(dest_id <= nr_nodes-1) return(dest_id) ;
  else return(source->node->id_seq) ;
}
/*---------------------------------------------------------------------------*/
void new_source_dest(SOURCE *source)
{
  int dest_node_id ;

    dest_node_id = source_dest(source) ;
    source->dest_node = &(nodes[dest_node_id]) ;
}
/*---------------------------------------------------------------------------*/
MESSAGE *create_new_message(SOURCE *source)
{
  int dest_node_id ;
  MESSAGE *message ;

  if(source->node != source->dest_node) {  /* to avoid at center node */
    message = new_message(source) ;
    message->src_node = source->node ;
    return(message) ;
  }
  else return(NULL_MESSAGE) ;
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
    if(para->source_type == RANDOM) {
      new_source_dest(source) ;
    }
    if(check_new_message_can_enter(lane)) {
      message = create_new_message(source) ;
      if(message != NULL_MESSAGE) {
	assign_initial_lane_to_message(lane, message) ;
      }
    }
  }
  else {
    if(para->source_type == RANDOM) {
      new_source_dest(source) ;
    }
    if(check_new_message_can_enter(lane)) {
      message = (MESSAGE *)dequeue(source->message_queue) ;
      if(para->source_type == RANDOM) message->dest_node = source->dest_node ;
      assign_initial_lane_to_message(lane, message) ;
    }
  }
/*printf("message=%x\n",message);*/
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
  time = cur_time + 1 + (random() % (2*para->sinterval)) ;
  schedule_source_event_at_time(source, time) ;
}
/*---------------------------------------------------------------------------*/
void schedule_source_event_at_time(SOURCE *source, int time)
{
  EVENT *event ;
  event = new_event(Event_Funct, source) ;
  schedule_event(time, event) ;
}
/*---------------------------------------------------------------------------*/
void Event_Funct(SOURCE *src)
{
  SOURCE *source = src ;
  EVENT *event  ;
  CHANNEL *channel ;
  MESSAGE *message ;
  LANE *lane ;
  int time, lane_nr, dest ;

  /* first, schedule next injection */
  schedule_source_event(source) ;

  /* if drain switch on do nothing */
  if(drain_messages) return ;

  /* if source node equals destination node, do nothing */
  if(para->source_type == RANDOM) new_source_dest(source) ; 
  if(source->node->id_seq == source->dest_node->id_seq) return ;

  /* message generation */
  if(debug & DEBUG_MESSAGE) {
    printf("MESSAGE GENERATED AT:src_%d\n", source->node->id_seq) ;
  }
  lane = find_next_lane_at_source(source);
  if(lane != NULL_LANE) {
    message = create_new_message(source) ;
    if(message != NULL_MESSAGE) {
      assign_initial_lane_to_message(lane, message) ;
    }
  }
  else {
    if(!source->sat) {
      message = create_new_message(source) ;
      if(message != NULL_MESSAGE) {
	defer_source_message(source, message) ; 
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void schedule_initial_source_events()
{
  int i ;

  for(i=0; i<nr_sources; i++) {
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

