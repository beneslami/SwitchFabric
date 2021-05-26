/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator include file - 
 |    (C) Bill Dally,  May 3, 1989
 */
/*---------------------------------------------------------------------------*/
/* STRUCTURE DEFINITIONS                                                     */
/*---------------------------------------------------------------------------*/
/*
 |    A node maps a set of input channels to a set of output channels.
 */
/*---------------------------------------------------------------------------*/
struct node {
  int            node_nr ;	/* the index of this node in the node array */
  struct channel *inputs ;	/* input channels - an array of channels */
  int            nr_inputs ;	/* number of input channels */
  struct channel **outputs ;	/* output channels - an array of pointers */
  int            nr_outputs ;	/* number of output channels */
} ;
typedef struct node NODE ;

/*---------------------------------------------------------------------------*/
/*
 |    A channel connects a source node to a destination node.
 |    It consists of a set of lanes.
 |
 |    A source node of nil implies that this is a network input channel.
 |    A destination node of nil implies that this is a network output channel.
 |
 |    Physically the channel consists of an output controller on the
 |    source node that allocates resources (buffers and wires), a
 |    single physical channel connecting the source and destination nodes,
 |    and a set of buffers on the destination node.
 |
 |    Two types of communication must occur over channels.  In the 
 |    forward direction, flits of a message are sent, each destined for a 
 |    particular lane on the destination node.  In the reverse direction
 |    flow control information is transmitted to inform the source of 
 |    buffer space available for each lane.
 |
 |    An input channel has no source node, but instead specifies a message 
 |    source.  A non-nil message source field indicates an input channel. 
 */
/*---------------------------------------------------------------------------*/
struct channel {
  struct node *src ;		/* source node */
  struct source *source ;	/* message source if this is an input channel */
  int         src_slot ;	/* index of this channel in source's input array*/
  struct node *dst ;		/* destination node */
  int         dst_slot ;	/* index of this channel in dest nodes's array */
  struct lane *lanes ;		/* lanes for this channel */
  int         nr_lanes ;	/* number of lanes */
  struct request *lane_requests ; /* requests for buffering resources */
  struct request *channel_requests ; /* requests to advance flits */
  struct channel *next ;	/* next channel in active list */
} ;
typedef struct channel CHANNEL, *CHANNEL_P ;

/*---------------------------------------------------------------------------*/
/*
 |    A lane is a buffer and associated control that holds part of
 |    a message while it is using the channel (its a virtual channel).
 |
 |    Like the channel it is a part of, a lane is physically spit
 |    between the source and destination nodes.
 |
 |    A lane is originally in the idle state (message = nil)
 |
 |    Once a lane is allocated to a message, the lane enters the
 |        requesting state (dest = nil).  In this state, the lane may accept
 |        additional flits of the message (up to its depth), but is unable
 |        to pass them on until it is allocated a destination lane.
 |
 |    After a message in a lane is allocated a lane on the next node of
 |        its route, it enters the transmitting state where it competes
 |        with other lanes transmitting on the destination channel to
 |        forward flits to the next node along the route.
 |
 |    Once the last flit of a message is transmitted, the lane returns
 |        to the idle state or is immediately allocated to a new message
 |        and enters the requesting state.
 */
/*---------------------------------------------------------------------------*/
struct lane { 
  struct channel *channel ;	/* channel containing this lane */ 
  int            lane_nr ;	/* index of this lane in channel's lanes array*/
  int            depth ;	/* number of flits this lane can store */ 
  struct lane	 *dest ;	/* destination lane */ 
  struct message *message ;	/* message occupying this lane */ 
  int            first ;	/* first flit of  the message in the lane */
  int		 nr_flits ;	/* number of flits in the lane */
  struct lane    *prev ;	/* previous lane */
} ; 
typedef struct lane LANE ;

/*---------------------------------------------------------------------------*/
/*
 |    A message holds the state of a message in transit.
 |
 |    A message is inserted by setting its head_lane field to
 |    the entering lane, modifying the lane state appropriately,
 |    and inserting the message on the active message list.
 |
 |    All messages on the active list are advanced by starting at
 |    the head, trying to advance to the next stage, and then following
 |    pointers back to the tail advancing into queueing as available.
 |
 |    When a message reaches its destination (output channels are nil)
 |    it remains in place (same lane) until the tail catches up.
 |    It is then logged as received.
 |
 */
/*---------------------------------------------------------------------------*/
struct message {
  int            dest ;		/* destination number */
  int            state ;	/* for routing */
  int            length ;	/* number of words in message */
  struct lane    *head_lane ;	/* pointer to lane containing head */
  int            start_time ;	/* time message originated */
  int            priority ;	/* lower numbers go first in deadline mode */
  struct message *next ;	/* for free list */
} ;
typedef struct message MESSAGE ;
/*---------------------------------------------------------------------------*/
/*
 |    A message source generates messages for input to the network.
 |    It contains a queue of messages waiting to be injected, a pointer
 |    to the channel it feeds, and scheduling parameters.
 |
 |    The source type indicates the traffic pattern (destination selection).
 |    The field "sat" if true indicates that the source is saturating the
 |    network and will always be generating messages - the queue is ignored
 |    for saturation sources.
 */
/*---------------------------------------------------------------------------*/
struct source {
  int             source_nr ;	/* index of this source in array */
  struct queue    *message_queue ;	/* queue of messages awaiting entry */
  struct channel  *dst ;	/* destination channel */
  int             type ;	/* type of source */
  int             interval ;	/* scheduling interval */
  int             length ;	/* message length */
  int             sat ;		/* saturation source - always busy */
} ;
typedef struct source SOURCE ;
/*---------------------------------------------------------------------------*/
/*
 |    A request represents a message competing for a lane or for a
 |    physical channel.
 |    Requests are enqueued in a list on the channel to which they apply.
 |    An arbitration function selects which requests will be serviced. 
 */
/*---------------------------------------------------------------------------*/
struct request {
  union {
    struct message *message ;
    struct lane *lane ; 
  } data ;
  struct request *next ;
} ;
typedef struct request REQUEST ;
/*---------------------------------------------------------------------------*/
/*
 |   An event list is built as an array of (possibly empty) timesteps.
 |   Scheduling beyond the size of the array is currently not allowed.
 |   in the future some overflow structure may be added.
 |
 |   Each timestep is a linked list of events.
 |
 |   An event is a deferred activity.  It contains a function to be
 |   called and a pointer to some data to be passed to the function.
 |
 */
/*---------------------------------------------------------------------------*/
struct event {
  struct event *next ;		/* pointer to next event in list */
  void (*funct)(char *data) ;	/* function to be performed */
  char  *data ;			/* pointer to just about anything */
} ;
typedef struct event EVENT ;
typedef struct event *TIMESTEP ;
/*---------------------------------------------------------------------------*/
struct stats {
  struct stats *next ;
  int start_time ;
  int nr_messages_sent ;
  int nr_messages_injected ;
  int nr_messages_received ;
  int total_latency ;
  int total_latency_sq ;	/* squared - for variance */
  int total_source_latency ;
  int *latency_histogram ;
  int max_hist ;
  int *lane_hist ;
} ;

typedef struct stats STATS ;
/*---------------------------------------------------------------------------*/
/* DEFINES */
/*---------------------------------------------------------------------------*/
#define MAX_LANES 16
#define MAX_DEPTH 256
#define INIT_MEM_SIZE (1<<22)
#define NR_TIMESTEPS (1<<14)
#define NULL 0
#define NULL_NODE ((NODE *)0)
#define NULL_CHANNEL ((CHANNEL *)0)
#define NULL_LANE ((LANE *)0)
#define NULL_MESSAGE ((MESSAGE *)0) 
#define NULL_REQUEST ((REQUEST *)0)
#define NULL_EVENT   ((EVENT *)0)
#define NULL_TIMESTEP ((TIMESTEP *)0)
#define NULL_STATS   ((STATS *)0)

/* source types */
#define S_RANDOM 0		/* random traffic */
#define S_VOICE_DATA 1		/* combined voice/data percentage given by sfrac*/
#define S_REVERSE 2		/* reverse permutation  0->n-1, 1->n-2, etc */
#define MAX_SOURCE_TYPE 1
#define LOW_PRIORITY (1<<30)	/* very large number for deadline */

/* channel scheduling algorithms */
#define CS_FIRST 0		/* advance first request */
#define CS_RANDOM 1		/* advance random message */
#define CS_DEADLINE 2		/* advance oldest message */
#define MAX_CHAN_SCHED 2

/* random maximums */
#define MAX_SOURCE_LENGTH (1<<10)
#define MAX_SOURCE_INTERVAL (1<<20)
#define MAX_NR_CYCLES (1<<20)

/* message states */
#define MESSAGE_INIT	0	/* initialized */
#define MESSAGE_NEED_OUTPUT 1	/* awaiting assignment of output lane */
#define MESSAGE_NEED_CHANNEL 2	/* awaiting use of channel */
#define MESSAGE_DONE 3		/* message delivery completed */
#define MAX_MESSAGE_STATE    3

/* debug options - bit masks*/
#define DEBUG_CREATION	1
#define DEBUG_ROUTING	2
#define DEBUG_ADVANCING 4
#define DEBUG_SOURCING  8 
#define DEBUG_EVENTS    0x10
/*---------------------------------------------------------------------------*/
#define MESSAGE_CHANNEL(mesage) (message->head_lane->channel)
#define MESSAGE_NODE(message) (MESSAGE_CHANNEL(message)->dst)
#define MESSAGE_NODE_NR(message) (MESSAGE_NODE(message)->node_nr)
#define MESSAGE_SLOT(message) (MESSAGE_CHANNEL(message)->dst_slot)
/*---------------------------------------------------------------------------*/
#define SOURCE_NODE(source)    (source->dst->dst)
#define SOURCE_NODE_NR(source) (SOURCE_NODE(source)->node_nr)
#define SOURCE_SLOT(source)    (source->dst->dst_slot)
#define SOURCE_CHANNEL(source) (source->dst)
/*---------------------------------------------------------------------------*/
#define LANE_NODE(lane) (lane->channel->dst)
#define LANE_NODE_NR(lane) (LANE_NODE(lane)->node_nr)
#define LANE_FULL(lane) (lane->nr_flits >= lane->depth)
#define LANE_EMPTY(lane) (lane->nr_flits == 0)  
#define LANE_BUSY(lane) ((lane)->message)
/*---------------------------------------------------------------------------*/
#define CHANNEL_ACTIVE(channel) (channel->lane_requests || \
				 channel->channel_requests)
/*---------------------------------------------------------------------------*/
/* GLOBALS */
/*---------------------------------------------------------------------------*/
extern NODE *nodes ;		/* array of nodes */
extern int nr_nodes ;		/* number of nodes */

extern MESSAGE *free_messages ;	/* list of free message structures */
extern MESSAGE *active_messages ; /* list of active message structures */

extern REQUEST *free_requests ;	/* list of free request structures */

extern CHANNEL *active_channels ; /* list of channels with requests pending */

extern SOURCE *sources ;	/* array of sources */
extern int nr_sources ;		/* number of sources in array */

extern TIMESTEP *event_list ;	/* array of timesteps */
extern int cur_time ;		/* the current time */
extern int cur_index ;		/* the position of the current timestep in */
				/* the event list */
EVENT *free_events ;		/* free event list */

/* routing function */
extern int (*routing_function)(int source_node_nr, int in_channel, int dest) ;
/* destination function - returns true if node is destination */
extern int (*destination_function)(int node_nr, int dest) ;


extern int nr_lanes ;		/* number of lanes per channel */
extern int depth ;		/* depth of each queue */
extern int debug ;		/* debugging bit mask */
				/* 1-bfly creation, 2-routing */
extern int stype;		/* source type */
extern int slength;		/* source message length in flits */
extern int sinterval;		/* source generation interval */
extern int ssat ;
extern int spct ;		/* fraction of time critical messages */

extern int chan_sched ;		/* algorithm used to allocate channel */

extern int nr_cycles ;		/* number of cycles to run a test */

extern int drain_messages ;	/* if true, sources disabled */

extern int vstat ;		/* if true, compute statistics only for */
				/* priority messages */
extern int graph ;              /* if true, produce output for splot */
extern int warm_cycles ;        /* number of cycles to overcome transient*/
extern int expsrc ;		/* 1 if poisson source */
extern double rate ;		/* rate of poisson source */
extern int unbuffered ;		/* if true, only length one buffers per VC */
extern int lanehist ;		/* if true, log lane occupancies */
extern int lh_stage ;		/* stage to log */
/*---------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------*/
extern void error(char *s)  ;	/* error exit routine */
/*---------------------------------------------------------------------------*/

