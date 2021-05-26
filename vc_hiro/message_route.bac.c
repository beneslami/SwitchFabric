/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator "message_route.c"
 |    message advancing functions
 |    Hiromichi Aoki
 |
/*---------------------------------------------------------------------------*/
#include "defs.h"
#include "netdef.h"
#include "message_route.h"
#include "routing_func.h"
#include "event.h"
#include "stats.h"
#include "source.h"
/*---------------------------------------------------------------------------*/
/* for bookkeeping on message and request records */
int nr_messages ;
int nr_requests ;
/*---------------------------------------------------------------------------*/
/* Function declaration */

    void request_next_lane() ;
    void allocate_next_lane() ; 
    void request_all_channels_of_message() ;
    void allocate_channel_for_flit() ;
    void assign_lane_to_message(LANE *lane, MESSAGE *message) ;
    void assign_initial_lane_to_message(LANE *lane, MESSAGE *message) ;
    void advance_lane_flit(LANE *lane) ;
    void set_new_max_DR_for_enter(LANE *lane) ;
    void set_new_max_DR_for_getout(LANE *lane) ;
    int  caluculate_A_NO1(CHANNEL *channel) ;
    int  caluculate_A_NO2(CHANNEL *channel) ;
    REQUEST *select_channel_request(REQUEST *request) ;
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

  printf("-checking %d messages %d requests-", nr_messages, nr_requests) ;
  /* check messages */
  nr = 0 ;
  FOREACH(message, free_messages) {
    nr++ ;
  }
  if(nr != nr_messages) {
    printf("MESSAGES LOST: %d on free list, %d allocated\n", nr, nr_messages) ;
  }
  /* check requests */
  nr = 0 ;
  FOREACH(request, free_requests) {
    nr++ ;
  }
  if(nr != nr_requests) {
    printf("REQUESTS LOST: %d on free list, %d allocated\n", nr, nr_requests) ;
  }
}
/*---------------------------------------------------------------------------*/
MESSAGE *new_message(SOURCE *source)
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
  message->dest_node = source->dest_node ;
  message->src_node = source->node ;
  message->length = source->length ;
  message->state = MESSAGE_INIT ;
  message->head_lane = NULL_LANE ;
  message->M_DR = NULL ;
  message->chang_dir_nr = NULL ;
  message->face = NULL ;
  message->wait_count = NULL ;
  message->misrouted = NULL ;
  log_message_creation(message) ;
  return(message) ;
}
/*---------------------------------------------------------------------------*/
void free_message(MESSAGE *message)
{
  message->dest_node = NULL_NODE ;
  message->length = message->state = message->start_time = 0 ;
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
  if(debug & DEBUG_MESSAGE) {
    printf("MESSAGE src_%d(t%d) DONE:\n", message->src_node->id_seq,
	    message->start_time) ;
    /*print_message(message) ; */
  }
}
/*---------------------------------------------------------------------------*/
void clean_messages()
{
  MESSAGE *messages, *message ;
  LANE *lane ;

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
  if(debug & DEBUG_MESSAGE) {
    print_message_A() ;
  }
  if(debug & DEBUG_ROUTING) {
    print_message_B() ;
  }
}
/*---------------------------------------------------------------------------*/
void clean_active_channels()
{
  CHANNEL *channel, *channels ;
  channels = active_channels ;
  active_channels = NULL_CHANNEL ;
  while(channels) {
    channel = channels ;
    channels = channel->next ;
    channel->next = NULL_CHANNEL ;
    channel->lane_requests = NULL_REQUEST ;
    channel->channel_requests = NULL_REQUEST ;
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
  request->message = message ; 
  return(request) ;
}
/*---------------------------------------------------------------------------*/
REQUEST *new_channel_request(LANE *lane)
{
  REQUEST *request = new_request() ;
  request->lane = lane ;
  request->message = lane->message ;
  return(request) ;
}
/*---------------------------------------------------------------------------*/
void free_request(REQUEST *request)
{
  request->message = NULL_MESSAGE ;
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
void inject_test_message(int src_node_nr, int length)
{
  MESSAGE *message ;
  LANE *lane ;
  SOURCE *source ;

  source = NEW_ARRAY(SOURCE, 1) ;
  init_source(source, src_node_nr) ;
  source->length = length ;
  new_source_dest(source) ;
  message = create_new_message(source) ;
  /* pick first lane of first input of specified source */
  lane = find_next_lane_at_source(source) ;
  /* insert message - mark waiting for output channel */
  assign_initial_lane_to_message(lane, message) ;
}
/*---------------------------------------------------------------------------*/
/*
 |    Request lane
 |    Enqueues a request for a lane by a message on a channel
 */
/*---------------------------------------------------------------------------*/
void request_lane(MESSAGE *message, LANE *lane)
{ 
  REQUEST *request ;
  if(!CHANNEL_ACTIVE(lane->channel)) {
    activate_channel(lane->channel) ;
  }
  request = new_lane_request(message) ;
  request->next = lane->channel->lane_requests ;
  lane->channel->lane_requests = request ;
  lane->channel->lane_requests->lane = lane ;
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
 |   Phase-1: Head flits of all messages try to get next lanes.
 */
/*---------------------------------------------------------------------------*/
void request_next_lanes() 
{
  MESSAGE *message ;
  int destination_lane ;
   FOREACH(message,active_messages) {
     if(message->head_lane) {
       destination_lane = check_dest_lane(message->head_lane, message) ;
     }
     else destination_lane = 0 ;
     if(!destination_lane) { 
       request_next_lane(message) ;
     }
   }
}
/*---------------------------------------------------------------------------*/
/*
 |     A message is processed as follows
 |         A destination channel in a node in which the head flit exists 
 |         is selected, a request is queued on this channel, and the channel
 |         is pushed on the active channel stack.
 |
 |         The selecting function select_next_lane is defined as 
 |
 |         select_next_lane_algorithm_1.
 |         This function returns an address of available next lane object and
 |         set message->state to FOUND_LANE.
 |         If no available lane is found then null value is returned.
 */
/*---------------------------------------------------------------------------*/
void request_next_lane(MESSAGE *message)
{
  LANE *next_lane ;
  next_lane = find_next_lane(message) ;
  if(next_lane != NULL_LANE) {
    if(message->head_lane->nr_flits) {
      request_lane(message, next_lane) ;
    }
    if(debug & DEBUG_ROUTING) {
      printf("message:%x(src%d(t%d)) requested lane: %d-%d(%d(%d))\n", 
	     message,
	     message->src_node->id_seq,
	     message->start_time,
	     next_lane->channel->src->id_seq,
	     next_lane->channel->dst->id_seq,
	     next_lane->lane_id_nr,
	     next_lane->DR ) ;
    }
  }
}  
/*-------------------------------------------------------------------------*/
void allocate_next_lanes()
{
  CHANNEL *channel ;

  FOREACH(channel, active_channels) {
    allocate_next_lane(channel) ;
  }
  if(debug & DEBUG_ROUTING) {
    printf("Phase 1: active channels: ") ;
    FOREACH(channel, active_channels) {
      printf("(%d-%d) ", channel->src->id_seq, channel->dst->id_seq) ;
    }
    printf("\n") ;
  }
  /* initiate active_channels list for advancing phase */
  clean_active_channels() ;
}
/*-------------------------------------------------------------------------*/
/*
 |   Lane allocation:
 |   Lane is allocated to the first message which exists 
 |   in the active message list.  
 */
/*-------------------------------------------------------------------------*/
void allocate_next_lane(CHANNEL *channel)
{
  REQUEST *requests, *request ;
  LANE *lane ;

  requests = channel->lane_requests ;
  channel->lane_requests = NULL_REQUEST ;
  while(requests) {
    request = requests ;
    requests = request->next ;
    lane = request->lane ;
    if(lane->state != OCCUPIED && lane->channel->state != DISCONNECTION) {
      assign_lane_to_message(lane, request->message) ;
    }
    else if(lane->channel->state != DISCONNECTION) {
      lane = find_next_lane(request->message) ;
      if(lane != NULL_LANE) {
	assign_lane_to_message(lane, request->message) ;
      }
      else cur_stats->nr_rejected_in_lane_allocation++ ;
    }
    else error("program error at allocate_next_lane\n") ;
    free_request(request) ;
  }
}      
/*---------------------------------------------------------------------------*/
/*
 |   Phase-2: Advance flits of messages.
 |   For each active message - make a channel request to advance flits
 */
/*---------------------------------------------------------------------------*/
void request_all_channels_of_messages() 
{
  MESSAGE *message ;

  FOREACH(message, active_messages) {
    request_all_channels_of_message(message) ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |    Flits of a message are advanced as follows:
 |       A channel request is made on each channel occupied by the message
 |       to advance a flit and these channels are pushed on the active
 |       channel stack.
 |       This active channel stack is a new version and different from
 |       previous active channel stack made on the lane request phase. 
 */ 
/*---------------------------------------------------------------------------*/
void request_all_channels_of_message(MESSAGE *message)
{
  LANE *from_lane, *to_lane ;
  CHANNEL *channel ;

  if(debug & DEBUG_ROUTING) {
    printf("message:%x(src%d(t%d))  head_lane:%d-%d\n",
	   message,
	   message->src_node->id_seq, 
	   message->start_time,
	   message->head_lane->channel->src->id_seq, 
	   message->head_lane->channel->dst->id_seq) ;
  }
  to_lane = message->head_lane ;
  while(to_lane) {
    from_lane = to_lane->prev ;
    if(check_dest_lane(to_lane, message)) {
      if(from_lane == NULL_LANE) {
      	request_channel(to_lane) ;
      }
      else if(!LANE_EMPTY(from_lane)) {
      	request_channel(to_lane) ;
      }
    }
    else {
      if(from_lane == NULL_LANE) {
	if(!LANE_FULL(to_lane) && !LANE_SATURATED(to_lane)) {
	  request_channel(to_lane) ;
	}
      }
      else if(!LANE_FULL(to_lane) && !LANE_EMPTY(from_lane)) {
	request_channel(to_lane) ;
      }
    }
    to_lane = from_lane ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |   For each active channel - advance flits of messages
 */
/*---------------------------------------------------------------------------*/
void allocate_channels_for_flits()
{
  CHANNEL *channel ;

  if(debug & DEBUG_ROUTING) {
    printf("Phase 2: active channels: ") ;
    FOREACH(channel, active_channels) {
      printf("(%d-%d) ", channel->src->id_seq, channel->dst->id_seq) ;
    }
    printf("\n") ;
  }

  FOREACH(channel, active_channels) {
    allocate_channel_for_flit(channel) ;
    if(channel->nr_ocpied_lanes == 0) {
      printf("Channel Id = %d\n", channel->dst_slot) ;
      print_channel_states(channel->src->id_seq) ;
      exit(6) ;
    }
    cur_stats->total_nr_occupied_lanes += channel->nr_ocpied_lanes ;
    cur_stats->nr_channel_requested++ ;
  }
  /* initialize active channel list for next routing phase */
  clean_active_channels() ;
}
/*---------------------------------------------------------------------------*/
/*
 |   Allocate a channel's bandwidth to a requesting lane
 |   Algorithm is selected by variable chan_sched.
 */
/*---------------------------------------------------------------------------*/
void allocate_channel_for_flit(CHANNEL *channel)
{
  REQUEST *requests, *request ;
  LANE *lane ;

  requests = channel->channel_requests ;
  channel->channel_requests = NULL_REQUEST ;
  request = select_channel_request(requests) ;
  lane = request->lane ;
  if(lane->message->head_lane == lane) {
    lane->message->state = MESSAGE_ADVANCED ;
  }
  advance_lane_flit(lane) ;
  /* printf("lane->message->head_lane=%d, lane=%d\n",
     lane->message->head_lane, lane) ; */
  while(requests) {
    request = requests ;
    requests = request->next ;
    free_request(request) ;
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
  int prev_dim ;
  int cur_dim ;
  int chang_dim_num ;

  /* first set up the lane */
  lane->dest = NULL_LANE ;	    /* no dest yet */
  lane->message = message ;	    /* allocate to message */
  lane->first = 0 ;		    /* head flit */
  lane->nr_flits = 0 ;		    /* no flits tranferred yet */
  lane->prev = message->head_lane ; /* old head lane is prev lane */
  lane->state = OCCUPIED ;          /* lane is occupied by this message */
  lane->channel->nr_ocpied_lanes++ ; /* number of current occupied lanes */ 
  
  /* renew the DR of the message and set it to lane */
  prev_dim = 0 ;
  if(lane->prev) {
    prev_dim = lane->prev->channel->dim ;
  }
  cur_dim = lane->channel->dim ;
  chang_dim_num = message->M_DR ;
  if(prev_dim > cur_dim) {
    chang_dim_num++ ;
  }
  message->M_DR = chang_dim_num ;
  message->wait_count = NULL ;
  if(lane->misrouted == 1) message->misrouted = 1 ;
  lane->DR = chang_dim_num ;
if(lane->DR > para_c->max_class && 
   (para_c->lane_types[lane->lane_id_nr]).category != SPECIAL) {
  printf("!!!!!!!!!!!!!!!!!\n") ;
}
  /* caluculate the new A_DR(max in the acceptable DRs) */
  set_new_max_DR_for_enter(lane) ;

  /* link in other direction too */
  if(message->head_lane) {
    message->head_lane->dest = lane ;
  }

  /* for message just set lane and state */
  message->head_lane = lane ;	/* this lane is new head lane */
  message->state = MESSAGE_NEED_CHANNEL ;
if(lane->prev != NULL_LANE) {
  if(lane->channel->src->id_seq != lane->prev->channel->dst->id_seq){
    printf("ERROR !!!!!!! on channel finding\n") ;
    print_one_message(message->src_node->id_seq, message->start_time) ;
    exit(9) ;
  }
}
  /*  if(debug & DEBUG_ROUTING) {
    printf("message src_%d head_lane advanced to (%d-%d)\n", 
	   message->src_node->id_seq,
	   lane->channel->src->id_seq,
	   lane->channel->dst->id_seq) ;
  }*/
}
/*---------------------------------------------------------------------------*/
void assign_initial_lane_to_message(LANE *lane, MESSAGE *message)
{
  assign_lane_to_message(lane, message) ; /* normal routine */
  lane->init = FIRST ; /* first lane of a message */
  lane->nr_flits = 1 ;
  lane->first = 0 ;
  activate_message(message) ;	/* activate */
  log_message_injection(message) ; /* log for stats */
}
/*---------------------------------------------------------------------------*/
void set_new_max_DR_for_enter(LANE *lane) 
{
  int max_DR ;

  max_DR = lane->channel->max_DR ;
  if(lane->DR <= max_DR) return ;
  else {
    lane->channel->max_DR = lane->DR ;
  }
}
/*---------------------------------------------------------------------------*/
void set_new_max_DR_for_getout(LANE *lane) 
{
  int i, max_DR, c_DR ;
  LANE *lanes ;

  lanes = lane->channel->lanes ;
  max_DR = c_DR = 0 ;
  for(i=0; i<para->nr_lanes; i++) {
    c_DR = lanes[i].DR ;
    if(lanes[i].state == OCCUPIED && (c_DR > max_DR) &&
       (para_c->lane_types[i].category != SPECIAL)) {
      max_DR = c_DR ;
    }
  }
  lane->channel->max_DR = max_DR ;
}
/*---------------------------------------------------------------------------*/
/* 
 |  sort requests according to specified function.  Returns pointer to
 |  first.
 */
/*---------------------------------------------------------------------------*/
/*request *sort_requests(request *rqs)
{
    to be implemented 
}*/
/*---------------------------------------------------------------------------*/
/*
 |   A lane is done when 
 |   Its first flit number is beyond the end of the message.
 */
/*---------------------------------------------------------------------------*/
int lane_done(LANE *lane)
{
  return(lane->first >= lane->message->length) ; 
}
/*---------------------------------------------------------------------------*/
/*
 |   A lane is done when 
 |   Its nr_flits is beyond the length of the message.
 */
/*---------------------------------------------------------------------------*/
int check_done(LANE *lane)
{
  return(lane->nr_flits >= lane->message->length) ; 
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
  /*lane->message = NULL_MESSAGE ; */
  lane->init = lane->first = lane->nr_flits = 0 ;
  lane->state = IDLE ;
  lane->DR = NULL ;
  lane->misrouted = NULL ;
  lane->channel->nr_ocpied_lanes-- ;
  /* renew A_DR of the channel object */
  set_new_max_DR_for_getout(lane) ;
  /* if this is a source lane, check for pending messages */
  if(lane->channel->src->source) {
    source = lane->channel->src->source ;
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
  int destination_lane ;

  if(debug & DEBUG_ROUTING) {
    printf("lane:%x  lane->message:%x\n", lane, lane->message) ;
  }
  if(lane->prev != NULL_LANE && lane->prev->nr_flits) {
    lane->nr_flits++ ;		/* one more flit in this lane */
  }
  else if(lane->init==FIRST) {
    lane->nr_flits++ ;
  }
  destination_lane = check_dest_lane(lane, lane->message) ;
  if(lane->prev != NULL_LANE) {
    lane->prev->nr_flits-- ;	/* one less flit in prev lane */
    lane->prev->first++ ;	/* off the front of the message */
    /* check for previous lane done */
    if(lane_done(lane->prev)) {
      set_lane_idle(lane->prev) ;
      lane->prev = NULL_LANE ;
      /* if this is the destination, we're all done with this message */
      if(destination_lane) {
	set_message_done(lane->message) ;
	set_lane_idle(lane) ;  }
    }
  }
  else {
    if(destination_lane) {
      if(check_done(lane)) {
	set_message_done(lane->message) ;
	set_lane_idle(lane) ;  }
	
    }
  }
}
/*---------------------------------------------------------------------------*/
/*
 |   Pick request to be granted the channel.
 */
/*---------------------------------------------------------------------------*/
REQUEST *select_channel_request(REQUEST *requests)
{
  REQUEST *request, *rq ;
  int time, new_time ;

  switch(para->chan_sched) {
  case CS_FIRST: 
    request = requests ; break ;
  case CS_DEADLINE:
    request = rq = requests ;
    time = rq->message->start_time ;
    while(rq) {
      rq = rq->next ;
      if(rq) {
	new_time = rq->message->start_time ;
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
      printf("final time %d \n",request->message->start_time) ;
    }
    break ;
  default:
    error("Unknown value for chan_sched") ;
  }
  return(request) ;
}
/*---------------------------------------------------------------------------*/
/*
 |   Check destination lane
 */
/*---------------------------------------------------------------------------*/
int check_dest_lane(LANE *lane, MESSAGE *message)
{ 
  int dest_node_id, cur_dst_node_id ;
  
  dest_node_id = message->dest_node->id_seq ;
  cur_dst_node_id = lane->channel->dst->id_seq ;
  return(cur_dst_node_id == dest_node_id) ;
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
	 message->dest_node, pm_array[message->state],
	 message->length, message->start_time, 
	 MESSAGE_ID_SEQ(message), MESSAGE_SLOT(message)) ;
} 
/*---------------------------------------------------------------------------*/
/*
 |   Print out a lane's state 
 */
/*---------------------------------------------------------------------------*/
void print_lane(LANE *lane)
{
  printf("LANE: %d.%d.%d first %d nr_flits %d\n", 
	 LANE_NODE_NR(lane), lane->channel->dst_slot, lane->lane_id_nr,
	 lane->first, lane->nr_flits) ;
}
/*---------------------------------------------------------------------------*/
