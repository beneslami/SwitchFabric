/*---------------------------------------------------------------------------*/
/*      
 |      Network simulater - "Network_sim" file-1: "kary_ncube.c"
 |        [K-array N-cube network creation & connection] 
 |       Hiromichi Aoki,  July 27, 1990
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "netdef.h"
#include "event.h"
#include "queue.h"
/*---------------------------------------------------------------------------*/
/*
 |    Functions to create k-ary n-cube topologies.
 */
/*---------------------------------------------------------------------------*/
/* VARIABLES */
/*---------------------------------------------------------------------------*/
int nr_ary ;
int nr_dim ;
int nr_chan ;
INTARY **temp ;                /* temporary array */
/*---------------------------------------------------------------------------*/
/* FUNCTIONS declaration */
/*---------------------------------------------------------------------------*/
void init_kary_ncube_nodes() ;
void create_channels_for_node() ;
void connect_kary_ncube() ;
void init_input_channel() ;
void init_lane() ;
void init_source() ;
void init_sources(SOURCE *sources) ;
/*---------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------*/
/*
/*---------------------------------------------------------------------------*/
/*
 |     Creation of k-array n-cube network
 */
/*---------------------------------------------------------------------------*/
void new_kary_ncube(int k, int n)
{
  int i, j, seq_id ;

  nr_ary = k ; nr_dim = n ;
  nr_chan = para->nr_chan ;
 /* create node arrays */
  nr_nodes = power(nr_ary, nr_dim) ; /*number of nodes */
  nodes = NEW_ARRAY(NODE, nr_nodes) ;  
    if(!nodes) error("unable to create node array") ;
 /* create dimensional node id array */
  for(i=0; i<nr_nodes; i++) {
    (&nodes[i])->id_dimensions = NEW_ARRAY(INTARY, nr_dim) ;
  }
 /* initialize nodes - setting node ids */
  init_kary_ncube_nodes(nodes, nr_nodes) ;

 /* create channels & initialize channels */
  for(i=0 ;i<nr_nodes ;i++) {
    create_channels_for_node(&(nodes[i])) ;
  }
 /* initialization is done, now connect it up */
  /* make temporary array for connection function */
  temp = NEW_ARRAY(INTARY*, nr_chan) ;
  for(i=0; i<nr_chan; i++) {
    temp[i] = NEW_ARRAY(INTARY, nr_dim) ; 
  }
  /* call connecting function */
  for(i=0; i<nr_nodes; i++) {
          connect_kary_ncube(&(nodes[i])) ;
  } 
  /* add a source for each node */
  nr_sources = nr_nodes ;
  sources = NEW_ARRAY(SOURCE, nr_sources) ;
  for(i=0; i<nr_sources; i++) {
    init_source(&(sources[i]), i) ;
  }
}
/*---------------------------------------------------------------------------*/
/*
 |  Create the sequential ids & dimensional ids in all nodes.
 */
/*---------------------------------------------------------------------------*/
void init_kary_ncube_nodes(NODE *node, int nr_nodes) 
{
  int i, j, a ;
  for(j=0; j<nr_dim; j++) {
    for(i=0; i<nr_nodes; i++) {
      a = (i/power(nr_ary, j))%nr_ary ;  
      (&node[i])->id_dimensions[j].value = a ; 
      (&node[i])->id_seq = i ;  
      (&node[i])->state = ALIVE ;  
    }
  }
}
/*---------------------------------------------------------------------------*/
void create_channels_for_node(NODE *node)
{
  int i ;

  nr_chan = para->nr_chan ;
  node->inputs = NEW_ARRAY(CHANNEL, nr_chan) ;
  node->outputs = NEW_ARRAY(CHANNEL_P, nr_chan) ;
  if((!node->inputs)||(!node->outputs)) error("Unable to create node ports");
  node->nr_inputs = node->nr_outputs = nr_chan ;
  for(i=0 ;i<nr_chan ;i++) {
    init_input_channel(node, &(node->inputs[i]), i) ;
  }
}
/*---------------------------------------------------------------------------*/
void init_input_channel(NODE *dest, CHANNEL *chan, int i) 
{
  int dim_nr, direction ;

  /* allocate channel dimension */
  dim_nr = i/2 ;
  direction = (i+1)%2 ;
  /* link to destination node */
  chan->dst = dest ;                          /* destination node */
  chan->dim = dim_nr ;                        /* set dimension value */
  chan->dire = direction ;                    /* set direction */
  chan->lanes = NEW_ARRAY(LANE, MAX_LANES) ;  /* create lanes for this chan */
  if(!chan->lanes) error("Unable to create lanes") ;
  chan->nr_lanes = para->nr_lanes ;            /* number of lanes */
  chan->nr_ocpied_lanes = 0 ;            /* number of current occupied lanes */
  /* initialize lanes */
  for(i=0;i<MAX_LANES;i++) {
    init_lane(&(chan->lanes[i]), chan, i) ;   /* init all element of lane */
  }
}
/*---------------------------------------------------------------------------*/
void init_lane(LANE *lane, CHANNEL *chan, int lane_nr) 
{
  lane->channel = chan ;          /* channel containing this channel */
  lane->dest = NULL_LANE ;        /* destination lane */
  lane->message = NULL_MESSAGE ;  /* message occupying this lane */
  lane->first = 0 ;               /* header flit of the message in this lane */
  lane->nr_flits = 0 ;            /* number of flits in this lane */
  lane->prev = NULL_LANE ;        /* previous lane */
  lane->depth = para->depth ;     /* number of flits this lane can store */
  lane->lane_id_nr = lane_nr ;    /* lane fixed identity */
  lane->state = IDLE ;            /* set initial lane state */
  lane->type = para_c->lane_types[lane_nr].category ; /* set lane type normal */
  lane->init = 0 ;
}
/*---------------------------------------------------------------------------*/
void connect_kary_ncube(NODE *node)
{
  int src_seq_index, dest_node_nr, in_slot, i, j, element_value,
      dst_node_id, dim_src_chan, dire_src_chan, dim_dst_chan,
      dire_dst_chan ;
  NODE *dst_node ;

  if(debug & DEBUG_CREATION) {
    printf("node%d: ",node->id_seq) ;
  }
  /* get seq_id number of source node */
  src_seq_index = node->id_seq ;
  /* culculate the dimensional id of dest nodes */ 
  if(nr_ary==2) {  /* binary cube */
   /* ith loop is for the destination node which exists toward ith       */
   /* dimension.  j represents jth dimension value of node.              */
   /* (&temp[i])[j].value represents jth dimension value of ith          */ 
   /* destination node.                                                  */
    for(i=0 ;i<nr_chan ;i++) { 
      for(j=0 ;j<nr_dim ;j++) {
	element_value = node->id_dimensions[j].value ; /* get source id */
	/* if j=i change the jth dimension value of temp[i]          */
	/* else copy the dimension value of source node to temp[i]   */
	if(j==i) { 
	  if(element_value==0)   temp[i][j].value = 1 ;
	  else if(element_value==1)   temp[i][j].value = 0 ;
	  else error("binary cube initialize fail") ;
	}
	else   temp[i][j].value = node->id_dimensions[j].value ;
      }
    }
  }
  else {  /* k>=3 array cube */
   /* ith and (i+1)th loops are for the two destination nodes which exist */ 
   /* toward ith dimension. ith is for the node in positive direction,    */
   /* (i+1)th is for in negative direction.  j represents jth dimension   */
   /* value of source node.  (&temp[i])[j].value represents jth dimension */
   /* value of ith destination node.                                      */
    for(i=0; i<nr_chan; i++) {
      for(j=0; j<nr_dim; j++) {
	int a ;
	element_value = node->id_dimensions[j].value ;
	/* if j=i change the jth dimension value of temp[i]          */
	/* else copy the dimension value of source node to temp[i]   */
	a = (i/2)%nr_dim ;
	if(j==a && i%2==0)       /* find a right neighbor */
	  if(element_value==(nr_ary-1)) /* rightest node in jth dim line*/
	    temp[i][j].value = 0 ;
	  else temp[i][j].value = element_value +1 ;
	else if(j==a && i%2==1)   /* find a left neighbor */
	  if(element_value==0) /* leftest node in jth dimension line */
	    temp[i][j].value = nr_ary-1 ;
	  else temp[i][j].value = element_value -1 ;
	else temp[i][j].value = node->id_dimensions[j].value ;
      }
    }
  }
  /* dst dimensional ids are set in temp ary, culculate the seq id */
  for(i=0 ;i<nr_chan ;i++) {  /* all destination channels */
    dst_node_id= 0 ;
    for(j=0 ;j<nr_dim ;j++) {  /* each destination channel */
      dst_node_id += (temp[i][j].value * power(nr_ary, j)) ;
    }
    if(debug & DEBUG_CREATION) {
      printf("%d ", dst_node_id) ; 
    }
    dst_node = &(nodes[dst_node_id]) ;
    if(nr_ary==2) {
      node->outputs[i] = &(dst_node->inputs[i]) ;
      dst_node->inputs[i].src = node ;
    }
    else {
      if(i%2==0) {
	node->outputs[i] = &(dst_node->inputs[i+1]) ;
	dst_node->inputs[i+1].src = node ;
      }
      else {
	node->outputs[i] = &(dst_node->inputs[i-1]) ;
	dst_node->inputs[i-1].src = node ;
      }
    }
  }
  if(debug & DEBUG_CREATION) {
    printf("\n");
  }
}
/*---------------------------------------------------------------------------*/
void init_source(SOURCE *source, int number)
{
  int dest_id ;
  /* make the queue */
  source->message_queue = new_queue() ;
  /* set paramaters */
  source->source_nr = number ;
  source->node = &(nodes[number]) ;
  dest_id  = source_dest(source) ;
  if(dest_id == MAXINT) dest_id = source->node->id_seq ;
  source->dest_node = &nodes[dest_id] ;
  source->type = para->source_type ;
  source->interval = para->sinterval ;
  source->length = para->slength ;
  source->sat = para->ssat ;
  nodes[number].source = source ;
}
/*---------------------------------------------------------------------------*/
void make_network_to_mesh_type()
{
  int i, j, k, element, a ;
  NODE *node ;

  for(i=0; i<nr_nodes; i++) {
    node = &(nodes[i]) ;
    for(j=0; j<para->nr_chan; j++) {
      for(k=0; k<para->nr_dim; k++) {
	element = node->id_dimensions[k].value ;
	a = (j/2)%(para->nr_dim) ;
	if(k==a && j%2==0) {
	  if(element==(para->nr_ary -1)) {
	    node->outputs[j]->state = DISCON ;
	  }
	}
	else if(k==a && j%2==1) {
	  if(element==0) {
	    node->outputs[j]->state = DISCON ;
	  }
	}
      }
    }
  }
} 
