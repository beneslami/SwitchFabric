/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "netsim.h"
#include "message.h"
#include "event.h"
#include "queue.h"
/*---------------------------------------------------------------------------*/
/* number of bit positions to shift to get dest node or port number */
#define BIT_NODE_SHIFT(stage)	((n-2-stage)*lk)
#define BIT_PORT_SHIFT(stage)	((n-1-stage)*lk)
/* mask lk bits wide */
#define LK_MASK			((1<<lk)-1)
/* mask lk bits wide shifted the appropriate amount */
#define BIT_NODE_MASK(stage)	(LK_MASK<<BIT_NODE_SHIFT(stage))
/*---------------------------------------------------------------------------*/
/*
 |    Subroutines to simulate a k-ary n-fly topology.
 */
/*---------------------------------------------------------------------------*/
/* VARIABLES */
/*---------------------------------------------------------------------------*/
int    k ;			/* number of input and outputs per switch */
int    lk ;			/* log base 2 of k */
int    n ;			/* number of stages in network */
int    height ;			/* number of switches in each stage */
int    log_height ;		/* log of number of switches in each stage */
/*---------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------*/
void init_bfly_lane(LANE *lane, CHANNEL *chan, int lane_nr) 
{
  lane->channel = chan ; lane->lane_nr = lane_nr ;
  lane->dest = NULL_LANE ; lane->message = NULL_MESSAGE ;
  lane->first = lane->nr_flits = 0 ;
  lane->prev = NULL_LANE ;
  lane->depth = depth ;
}
/*---------------------------------------------------------------------------*/
void init_bfly_chan(CHANNEL *chan, NODE *dest, int dest_slot) 
{
  int i ;
  /* link to destination node */
  chan->dst = dest ; chan->dst_slot = dest_slot ;
  chan->lanes = NEW_ARRAY(LANE, MAX_LANES) ;
  if(!chan->lanes) error("Unable to create lanes") ;
  chan->nr_lanes = nr_lanes ;
  for(i=0;i<MAX_LANES;i++) {
    init_bfly_lane(&(chan->lanes[i]), chan, i) ;
  }
}
/*---------------------------------------------------------------------------*/
void init_bfly_node(NODE *node, int index)
{
  int i ;
  /* init a node - a node creates its input channels, but links to its outputs */
  node->node_nr = index ;
  node->inputs = NEW_ARRAY(CHANNEL, k) ;
  node->outputs = NEW_ARRAY(CHANNEL_P, k) ;
  if((!node->inputs) || (!node->outputs)) error("Unable to create node ports") ;
  node->nr_inputs = node->nr_outputs = k ;
  for(i=0;i<k;i++) {		/* init each channel */
    init_bfly_chan(&(node->inputs[i]), node, i) ;
  }
}
/*---------------------------------------------------------------------------*/
void connect_bfly_node(NODE *node, int stage, int index)
{
  int out_slot, dest_index, dest_node_nr, in_slot ;
  int shift = BIT_NODE_SHIFT(stage) ;
  int mask =  ~BIT_NODE_MASK(stage) ;
  NODE *dest_node ;
  /* for each output channel, calculate node and slot */
  for(out_slot=0;out_slot<k;out_slot++) {
    /* calculate destination node */
    dest_index = (index & mask) | (out_slot<<shift) ;
    dest_node_nr = ((stage+1)*height) + dest_index ;
    dest_node = &(nodes[dest_node_nr]) ;
    /* find first unused input slot */
    in_slot = first_open_input(dest_node) ;
    if(in_slot < 0) error("UNABLE TO FIND EMPTY INPUT") ;
    /* connect it up */
    node->outputs[out_slot] = &(dest_node->inputs[in_slot]) ;
    dest_node->inputs[in_slot].src = node ;
    dest_node->inputs[in_slot].src_slot = out_slot ;
    /* for debugging */
    if(debug & DEBUG_CREATION) {
      printf("STAGE %d NR %d OUTPUT %d ==> NR %d IN %d\n",
	     stage,index,out_slot,dest_index,in_slot) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void init_bfly_source(SOURCE *source, int index) 
{
  int switch_nr, input_nr, time ;
  EVENT *event ;
  CHANNEL *channel ;

  /* make the queue */
  source->message_queue = new_queue() ;
  /* connect it up */
  source->source_nr = index ;
  switch_nr = index>>lk ;
  input_nr = index - (switch_nr<<lk) ; 
  channel = &(nodes[switch_nr].inputs[input_nr]) ;
  source->dst = channel ;
  channel->source = source ; 
  /* set default parameters for now */
  source->type = stype ;
  source->interval = sinterval ;
  source->length = slength ; 
  source->sat = ssat ;
  /* debug message */
  if(debug & DEBUG_SOURCING) {
    printf("New source %d.%d\n", 
	   source->dst->dst->node_nr, source->dst->dst_slot) ;
  }
}
/*---------------------------------------------------------------------------*/
/* 
 |    Create and initialize a k-ary n-fly network given the log base 2
 |    of the radix (llk) and the number of stages (ln).
 */
/*---------------------------------------------------------------------------*/
void new_bfly(int llk, int ln) 
{
  int i, stage ;
  lk = llk ; n = ln ; 
  k = 1<<lk ;
  log_height = lk*(n-1) ;
  height = power(k,(n-1)) ;
  printf("height %d log %d\n", height, log_height) ;
  nr_nodes = height*n ;		/* compute number of nodes */
  nodes = NEW_ARRAY(NODE,nr_nodes) ; /* make the node array */
  if(!nodes) error("Unable to create node array") ;
  for(i=0;i<nr_nodes;i++) {
    init_bfly_node(&(nodes[i]),i) ; /* initialize each node */
  }
  /* now that everything is initialized, connect it up */
  for(stage=0;stage<(n-1);stage++) {
    for(i=0;i<height;i++) {
      connect_bfly_node(&(nodes[stage*height+i]),stage,i) ;
    }
  }
  /* add a source for each input node */
  nr_sources = height*k ;
  sources = NEW_ARRAY(SOURCE, nr_sources) ;
  for(i=0;i<nr_sources;i++) {
    init_bfly_source(&sources[i], i) ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |    Pick output channel for next step of route -- simple destination tag
 |    routing.
 */
/*---------------------------------------------------------------------------*/
int bfly_route(int source_node_nr, int in_channel, int dest)
{
   int stage = source_node_nr >> log_height ;
   int level = source_node_nr - (stage<<log_height) ;
   int shift = BIT_PORT_SHIFT(stage) ;
   int out_chan = (dest>>shift)&LK_MASK ;
   if(debug & DEBUG_ROUTING) {
     printf("bfly_route: source node %d in stage %d.%d to dest %d => %d\n",
	    source_node_nr, stage, level, dest, out_chan) ;
   }
   return(out_chan) ;
}
/*---------------------------------------------------------------------------*/
/*
 |  At destination if we're in last stage and all but last lk bits
 |  of dest match node number.
 */
/*---------------------------------------------------------------------------*/
int bfly_dest(int node_nr, int dest)
{
  int stage = node_nr >> log_height ;
  int level = node_nr - (stage<<log_height) ;
  return((stage == (n-1)) && ((dest>>lk) == level)) ;
}
/*---------------------------------------------------------------------------*/
