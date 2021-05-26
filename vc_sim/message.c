/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator
 |    Routines for moving messages
 |    (C) Bill Dally, May 29, 1989 
 */
/*---------------------------------------------------------------------------*/
#include "defs.h"
#include "netsim.h"
#include "message.h"
#include "event.h"
#include "stats.h"
#include "source.h"
/*---------------------------------------------------------------------------*/
extern void assign_initial_lane_to_message(LANE *lane, MESSAGE *message) ;
/*---------------------------------------------------------------------------*/
/* for bookkeeping on message and request records */
int nr_messages ;
int nr_requests ;
/*---------------------------------------------------------------------------*/
void init_messages()
{
  free_messages = NULL_MESSAGE ;
  active_messages = NULL_MESSAGE ;
  free_requests = NULL_REQUEST ;
  active_channels = NULL_CHANNEL ;
  nr_messages = 0 ;
  nr_requests = 0 ;
}
/*---------------------------------------------------------------------------*/
/* checks that all allocated message and request records are accounted for */
/* call only when network is drained */
void check_messages_and_requests() 
{
  int nr ;
  MESSAGE *message ;
  REQUEST *request ;

  if(!graph)
    printf("-checking %d messages %d requests-", nr_messages, nr_requests) ;
  /* check messages */
  nr = 0 ;
  FOREACH(message, free_messages) {
    nr++ ;
  }
  if(nr != nr_messages) {
    if(!graph)
      printf("MESSAGES LOST: %d on free list, %d allocated\n", nr, nr_messages) ;
  }
  /* check requests */
  nr = 0 ;
  FOREACH(request, free_requests) {
    nr++ ;
  }
  if(nr != nr_requests) {
    if(!graph)
      printf("REQUESTS LOST: %d on free list, %d allocated\n", nr, nr_requests) ;
  }
}
/*---------------------------------------------------------------------------*/
MESSAGE *new_message(int dest, int length)
{
  MESSAGE *message ; 

  /* if one available, recycle */
  if(free_messages) { 
    message = free_messages ;
    free_messages = message->next ;
    message->next = NULL_MESSAGE ;
  }
  else {
    message = NEW_STRUCT(MESSAGE) ;
    nr_messages++ ;
  }

  /* now initialize fields */
  message->start_time = cur_time ;
  message->priority = cur_time ;
  message->dest = dest ;
  message->length = length ;
  message->state = MESSAGE_INIT ;
  log_message_creation(message) ;
  return(message) ;
}
/*---------------------------------------------------------------------------*/
void free_message(MESSAGE *message)
{
  message->dest = message->length = message->state = message->start_time = 0 ;
  message->head_lane = NULL_LANE ;
  message->next = free_messages ;
  free_messages = message ;
}
/*---------------------------------------------------------------------------*/
/*
 |    When a message is done, set its state done -- it will be
 |    returned to the free list by "clean_messages" 
 */
/*---------------------------------------------------------------------------*/
void set_message_done(MESSAGE *message)
{
  message->state = MESSAGE_DONE ;
  if(debug & DEBUG_ADVANCING) {
    printf("MESSAGE DONE:\n") ;
    print_message(message) ;
  }
}
/*---------------------------------------------------------------------------*/
void clean_messages()
{
  MESSAGE *messages, *message ;
  messages = active_messages ;
  active_messages = NULL_MESSAGE ;
  while(messages) {
    message = messages ;
    messages = message->next ; 
    if(message->state == MESSAGE_DONE) {
      log_message_receipt(message) ;
      free_message(message) ;
    }
    else {
      activate_message(message) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
REQUEST *new_request()
{
  REQUEST *request ;

  if(free_requests) {
    request = free_requests ;
    free_requests = request->next ;
    request->next = NULL_REQUEST ;
  }
  else {
    request = NEW_STRUCT(REQUEST) ;
    nr_requests++ ;
  }
  return(request) ;
} 
/*---------------------------------------------------------------------------*/
REQUEST *new_lane_request(MESSAGE *message)
{
  REQUEST *request = new_request() ;
  request->data.message = message ; 
  return(request) ;
}
/*---------------------------------------------------------------------------*/
REQUEST *new_channel_request(LANE *lane)
{
  REQUEST *request = new_request() ;
  request->data.lane = lane ; 
  return(request) ;
}
/*---------------------------------------------------------------------------*/
void free_request(REQUEST *request)
{
  request->data.message = NULL_MESSAGE ;
  request->next = free_requests ;
  free_requests = request ;
}
/*---------------------------------------------------------------------------*/
void activate_message(MESSAGE *message)
{
  message->next = active_messages ;
  active_messages = message ;    
}
/*---------------------------------------------------------------------------*/
void activate_channel(CHANNEL *channel)
{
  channel->next = active_channels ;
  active_channels = channel ;
}
/*---------------------------------------------------------------------------*/
/* find first free lane - return index if found, return number of lanes */
/* if not found */
/*---------------------------------------------------------------------------*/
int first_free_lane_nr(CHANNEL *channel) 
{
  int lane_nr = 0 ;
  while((lane_nr < channel->nr_lanes) &&
	LANE_BUSY(&(channel->lanes[lane_nr]))) lane_nr++ ;
  return(lane_nr) ;
}
/*---------------------------------------------------------------------------*/
int nr_lanes_busy(CHANNEL *channel)
{
  int lane_nr, nr_busy = 0 ;

  nr_busy = 0 ;
  for(lane_nr=0;lane_nr< channel->nr_lanes;lane_nr++) {
    if(LANE_BUSY(&(channel->lanes[lane_nr]))) nr_busy++ ;
  }
  return(nr_busy) ;
}
/*---------------------------------------------------------------------------*/
void inject_test_message(int dest, int length, int src_node_nr)
{
  MESSAGE *message ;
  LANE *lane ;

  message = new_message(dest, length) ;

  /* pick first lane of first input of specified source */
  lane = &(nodes[src_node_nr].inputs[0].lanes[0]) ;
  /* insert message - mark waiting for output channel */
  assign_initial_lane_to_message(lane, message) ;
}
/*---------------------------------------------------------------------------*/
/*
 |    Request lane
 |    Enqueues a request for a lane by a message on a channel
 */
/*---------------------------------------------------------------------------*/
void request_lane(MESSAGE *message, CHANNEL *channel)
{ 
  REQUEST *request ;

  if(!CHANNEL_ACTIVE(channel)) {
    activate_channel(channel) ;
  }
  request = new_lane_request(message) ;
  request->next = channel->lane_requests ;
  channel->lane_requests = request ;
}
/*---------------------------------------------------------------------------*/
/*
 |    Request channel
 |    Enqueues a request on a channel to advance a flit to the specified
 |    lane.
 *
/*---------------------------------------------------------------------------*/
void request_channel(LANE *lane)
{
  REQUEST *request  ;

  if(!CHANNEL_ACTIVE(lane->channel)) {
    activate_channel(lane->channel) ;
  }
  request = new_channel_request(lane) ;
  request->next = lane->channel->channel_requests ;
  lane->channel->channel_requests = request ;
}
/*---------------------------------------------------------------------------*/
/*
 |     A message is processed as follows
 |         A destination channel is selected, a request is queued on this
 |         channel, and the channel is pushed on the active channel stack.
 */
/*---------------------------------------------------------------------------*/
void route_active_message(MESSAGE *message)
{
  int dest_channel_nr ;
  CHANNEL *dest_channel ;

  if(debug & DEBUG_ROUTING) print_message(message) ;
  /* make a request for the next step */
  dest_channel_nr = (*routing_function)(MESSAGE_NODE_NR(message),
					MESSAGE_SLOT(message),
					message->dest) ;
  if(debug & DEBUG_ROUTING) { printf("selecting %d\n",dest_channel_nr) ;}
  dest_channel = MESSAGE_NODE(message)->outputs[dest_channel_nr] ;
  /* if this is not the actual destination, request the next step */
  if(dest_channel) request_lane(message, dest_channel) ;
}
/*---------------------------------------------------------------------------*/
/*
 |    A message is advanced as follows:
 |       A request is made on each channel occupied by the message
 |       to advance a flit and these channels are pushed on the active
 |       channel stack. 
 */ 
/*---------------------------------------------------------------------------*/
void advance_active_message(MESSAGE *message)
{
  LANE *from_lane, *to_lane ;

  if(debug & DEBUG_ADVANCING) {
    printf("ADVANCING:\n") ;
    print_message(message) ;
  }
  to_lane = message->head_lane ;
  while(to_lane) {
    if(debug & DEBUG_ADVANCING) { print_lane(to_lane) ; }
    from_lane = to_lane->prev ;
    if(to_lane && from_lane && !LANE_FULL(to_lane) && !LANE_EMPTY(from_lane)) {
      request_channel(to_lane) ;
    }
    to_lane = from_lane ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |    For each active message - make a request for next lane in path
 */
/*---------------------------------------------------------------------------*/
void route_active_messages() 
{
  MESSAGE *message ;

  if(debug & DEBUG_ROUTING) { printf("PROCESSING ACTIVE MESSAGES:\n") ;}
  FOREACH(message,active_messages){
    route_active_message(message) ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |   For each active message - make a request to advance flits
 */
/*---------------------------------------------------------------------------*/
void advance_active_messages() 
{
  MESSAGE *message ;

  if(debug & DEBUG_ADVANCING) { printf("ADVANCING ACTIVE MESSAGES:\n") ;}
  FOREACH(message,active_messages){
    advance_active_message(message) ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |   Assigns a lane to a message:
 |   
 */
/*---------------------------------------------------------------------------*/
void assign_lane_to_message(LANE *lane, MESSAGE *message)
{
  /* first set up the lane */
  lane->dest = NULL_LANE ;	/* no dest yet */
  lane->message = message ;	/* allocate to message */
  lane->first = 0 ;		/* head flit */
  lane->nr_flits = 0 ;		/* no flits tranferred yet */
  lane->prev = message->head_lane ; /* old head lane is prev lane */
  if(message->head_lane) {
    message->head_lane->dest = lane ; /* link in other direction too */
  }

  /* for message just set lane and state */
  message->head_lane = lane ;	/* this lane is new head lane */
  message->state = MESSAGE_NEED_CHANNEL ;
}
/*---------------------------------------------------------------------------*/
void assign_initial_lane_to_message(LANE *lane, MESSAGE *message)
{
  assign_lane_to_message(lane, message) ; /* normal routine */
  lane->nr_flits = message->length ; /* may exceed max */
  activate_message(message) ;	/* activate */
  log_message_injection(message) ; /* log for stats */
}
/*---------------------------------------------------------------------------*/
/* 
 |  sort requests according to specified function.  Returns pointer to
 |  first.
 */
/*---------------------------------------------------------------------------*/
REQUEST *sort_requests(REQUEST *rqs)
{
   /* to be implemented */
   return(rqs) ;
}
/*---------------------------------------------------------------------------*/
/*
 |   Allocate a channel's lanes to requesting messages
 |   Current algorithm is FIFO
 |   For each request - if lane available allocate - free request.
 */
/*---------------------------------------------------------------------------*/
void route_active_channel(CHANNEL *channel)
{
  REQUEST *requests, *request ;
  int lane_nr ;

  requests = channel->lane_requests ;
  channel->lane_requests = NULL_REQUEST ;
  while(requests) {
    request = requests ;
    requests = request->next ;
    /* scan to next free lane if any */
    lane_nr = first_free_lane_nr(channel) ;
    if(lane_nr < channel->nr_lanes) {
      assign_lane_to_message(&(channel->lanes[lane_nr]), 
			     request->data.message) ;
    }
    free_request(request) ;
  }
  /* zap request field now that they are all processed */
}
/*---------------------------------------------------------------------------*/
/*
 |   For each active channel - allocate lanes to requesters
 */
/*---------------------------------------------------------------------------*/
void route_active_channels()
{
  CHANNEL *channel ;

  if(debug & DEBUG_ROUTING) { printf("ROUTING ACTIVE CHANNELS:\n") ;}
  FOREACH(channel, active_channels) {
    route_active_channel(channel) ;
  }
  /* initialize list for advancing phase */
  active_channels = NULL_CHANNEL ;
}
/*---------------------------------------------------------------------------*/
/*
 |   A lane is done when 
 |   Its first flit is beyond the end of the message.
 */
/*---------------------------------------------------------------------------*/
int lane_done(LANE *lane)
{
  return(lane->first >= lane->message->length) ; 
}
/*---------------------------------------------------------------------------*/
/*
 |   Set a lane idle
 |   If this is an input lane and source is pending, start next message 
 */
/*---------------------------------------------------------------------------*/
void set_lane_idle(LANE *lane)
{
  SOURCE *source ;

  lane->dest = lane->prev = NULL_LANE ;
  lane->message = NULL_MESSAGE ;
  lane->first = lane->nr_flits = 0 ;
  /* if this is a source lane, check for pending messages */
  if(lane->channel->source) {
    source = lane->channel->source ;
    if(source_message_pending(source)) {
      dequeue_source_to_lane(source, lane) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
/*
 |   Advance a flit into a lane
 */
/*---------------------------------------------------------------------------*/
void advance_lane_flit(LANE *lane)
{
  int at_destination = 
    (*destination_function)(LANE_NODE_NR(lane), lane->message->dest) ;
  if(!at_destination) {
    lane->nr_flits++ ;		/* one more flit in this lane */
  }
  lane->prev->nr_flits-- ;	/* one less flit in prev lane */
  lane->prev->first++ ;		/* off the front of the message */
  /* check for previous lane done */
  if(lane_done(lane->prev)) {
    set_lane_idle(lane->prev) ;
    lane->prev = NULL_LANE ;
    /* if this is the destination, we're all done with this message */
    if(at_destination) {
      set_message_done(lane->message) ;
      set_lane_idle(lane) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
int count_requests(REQUEST *requests)
{
  int i = 0 ;
  REQUEST *rq = requests ;
  while(rq) {
    rq = rq->next ;
    i++ ;
  }
}
/*---------------------------------------------------------------------------*/
REQUEST *nth_request(int n, REQUEST *requests)
{
  int i = n-1 ;
  REQUEST *rq = requests ;
  if(rq) {
    while(i && rq->next) {
      i-- ;
      rq = rq->next ;
    }
  }
  return(rq) ;
}
/*---------------------------------------------------------------------------*/
/*
 |   Pick request to be granted the channel.
 |   Note that the requests here are lanes requesting channels. 
 */
/*---------------------------------------------------------------------------*/
REQUEST *select_channel_request(REQUEST *requests)
{
  REQUEST *request, *rq ;
  int time, new_time, nr, pick ;

  switch(chan_sched) {
  case CS_RANDOM:
    nr = count_requests(requests) ;
    pick = random() % nr ;
    request = nth_request(pick, requests) ;
    break ;
  case CS_FIRST: 
    request = requests ; break ;
  case CS_DEADLINE:
    request = rq = requests ;
    time = rq->data.lane->message->priority ;
    while(rq) {
      rq = rq->next ;
      if(rq) {
	new_time = rq->data.lane->message->priority ;
	if (debug & DEBUG_ADVANCING)  {
	  printf("time %d new %d\n",time, new_time) ;
	}
	if(new_time < time) {
	  time = new_time ;
	  request = rq ;
	}
      }
    }
    if (debug & DEBUG_ADVANCING) {
      printf("final time %d \n",request->data.lane->message->start_time) ;
    }
    break ;
  default:
    error("Unknown value for chan_sched") ;
  }
  return(request) ;
}
/*---------------------------------------------------------------------------*/
/*
 |   Allocate a channel's bandwidth to a requesting lane
 |   Algorithm is selected by variable chan_sched.
 */
/*---------------------------------------------------------------------------*/
void advance_active_channel(CHANNEL *channel)
{
  REQUEST *requests, *request ;

  if(debug & DEBUG_ADVANCING) {
    printf("ADVANCING FLIT IN ") ;
    print_lane(channel->channel_requests->data.lane) ;
  }
  requests = channel->channel_requests ;
  channel->channel_requests = NULL_REQUEST ;
  request = select_channel_request(requests) ;
  advance_lane_flit(request->data.lane) ;
  while(requests) {
    request = requests ;
    requests = request->next ;
    free_request(request) ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |   For each active channel - advance flits of messages
 */
/*---------------------------------------------------------------------------*/
void advance_active_channels()
{
  CHANNEL *channel ;

  if(debug & DEBUG_ADVANCING) { printf("ADVANCING ACTIVE CHANNELS:\n") ; }
  FOREACH(channel, active_channels) {
    advance_active_channel(channel) ;
  }
  /* initialize list for next routing phase */
  active_channels = NULL_CHANNEL ;
}
/*---------------------------------------------------------------------------*/
/*
 |   Print out a message's state 
 */
/*---------------------------------------------------------------------------*/
char *pm_array[] = {"INIT", "NEED_OUT", "NEED_CHAN", ""} ;
void print_message(MESSAGE *message)
{
  ASSERT((message->state <= MAX_MESSAGE_STATE), "print_message") ; 
  printf("MESSAGE: dest %4d state %10s length %2d time %d pos %d.%d\n",
	 message->dest, pm_array[message->state],
	 message->length, message->start_time, 
	 MESSAGE_NODE_NR(message), MESSAGE_SLOT(message)) ;
} 
/*---------------------------------------------------------------------------*/
/*
 |   Print out a lane's state 
 */
/*---------------------------------------------------------------------------*/
void print_lane(LANE *lane)
{
  printf("LANE: %d.%d.%d first %d nr_flits %d\n", 
	 LANE_NODE_NR(lane), lane->channel->dst_slot, lane->lane_nr,
	 lane->first, lane->nr_flits) ;
}
/*---------------------------------------------------------------------------*/
