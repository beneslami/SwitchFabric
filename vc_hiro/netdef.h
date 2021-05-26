/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator definition file - "netdef.h" 
 |    Hiromichi Aoki,  July 27 1990
 */
/*---------------------------------------------------------------------------*/
/* STRUCTURE DEFINITIONS                                                     */
/*---------------------------------------------------------------------------*/
/*
 |    A node maps a set of input channels to a set of output channels.
 */
/*---------------------------------------------------------------------------*/
struct paramaters {
  int k_array ;             /* number of array in each dimension */
  int n_cube ;              /* number of dimension of network */
  int nr_ary ;              /* number of array in each dimension */
  int nr_nodes ;            /* number of nodes */
  int nr_dim ;              /* number of dimension of network */
  int nr_chan ;             /* number of channels in each node */
  int nr_lanes ;            /* number of VCs */
  int depth ;               /* buffer depth of each VC */
  int slength ;             /* message length */
  int source_type ;         /* destination creation type */
  int sinterval ;           /* message generation intervals */
  int ssat ;                /* saturation mode(1) or not(0) */
  int chan_sched ;          /* VC to physical channel selection scheme */
  int routing_scheme ;      /* adaptive(1) or nonadaptive(2) */
  int chan_finding ;        /* shortest path(1) or any channel(2) */
  int timeout_mode ;        /* within shortest path(1) or any channel(2) */
  int timeout_count ;       /* timeout threshold */
  int chan_sel_priority_1 ; /* 1. free buffer threshold  2. minimizing DR */
  int chan_sel_priority_2 ; /* 3. max delta in each dimension distance */
  int chan_sel_priority_3 ; /* 4. lowest dimension */
  int chan_sel_priority_4 ;
  int tload_mode ;          /* traffic load mode */
  int test_category ;       /* experiment category */
} ;
typedef struct paramaters PARA ;
/*---------------------------------------------------------------------------*/
struct para_channel {
  struct nc_table *nc_table ;
  struct nc_table *first_nc_table ;
  struct para_lane *lane_types ;
  int VC_category ;
  int nr_fixed_lanes ;
  int fixed_min_lane_nr ;
  int fixed_max_lane_nr ;
  int max_class ;
  struct intary *class_i_exist ;
  struct intary *lowest_VC_nr_class_i ;
  struct intary *highest_VC_nr_class_i ;
  int nr_special_lanes ;
  int special_min_lane_nr ;
  int special_max_lane_nr ;
  int nr_free_lanes ;
  int free_min_lane_nr ;
  int free_max_lane_nr ;
  int nr_dynamic_lanes ;
  int dynamic_min_lane_nr ;
  int dynamic_max_lane_nr ;
} ;
typedef struct para_channel PARA_C ;
/*---------------------------------------------------------------------------*/
struct para_lane {
  int category ;
  int class ;
} ;
typedef struct para_lane PARA_LANE ;
/*---------------------------------------------------------------------------*/
struct node {
  int            id_seq ;         /* sequential id number */
  struct intary *id_dimensions;   /* dimensional id number */
  struct channel *inputs ;	  /* input channels - an array of channels */
  int            nr_inputs ;	  /* number of input channels */
  struct channel **outputs ;	  /* output channels - an array of pointers */
  int            nr_outputs ;	  /* number of output channels */
  int            func_type ;      /* the type of traffic restriction control */
  struct source  *source ;        /* message source if this node is source */
} ;
typedef struct node NODE ;
/*---------------------------------------------------------------------------*/
/*
 |    intary is used to create the dimensional ids of nodes.
 |    Dimensional ids are needed in network creation phase.
 */
/*---------------------------------------------------------------------------*/
struct intary {
  int value ;                   /* element of array */
} ;
typedef struct intary INTARY ;
/*---------------------------------------------------------------------------*/
/*
 |    A channel connects a source node to a destination node.
 |    It consists of a set of lanes.
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
 */
/*---------------------------------------------------------------------------*/
struct channel {
  int           dim ;                /* dimension of this channel */
  int           dire ;               /* direction of output [0,1] */
  struct node   *src ;	             /* source node */
  struct node   *dst ;	             /* destination node */
  int           dst_slot ;           /* index of this channel */
  struct lane   *lanes ;             /* lanes for this channel */
  int           state ;              /* channel state */
  int           nr_lanes ;           /* number of lanes */
  int           special_lane ;       /* set if special lane is used */
  int           inject_lane ;        /* set if injection lane is used */ 
  int           nr_ocpied_lanes ;    /* number of current occupied lanes */
  int           max_DR ;             /* max DR in occupied VCs */
  int           selected_lane_nr ;   /* used in lane selection stage */
  int           misrouted ;
  struct request *lane_requests ;    /* requests for buffering resources */
  struct request *channel_requests ; /* requests to advance flits */
  struct channel *next ;	     /* next channel in active list */
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
  struct channel *channel ;	  /* channel containing this lane */ 
  int            depth ;	  /* number of flits this lane can store */ 
  struct lane	 *dest ;	  /* destination lane */ 
  int            state ;          /* lane state */
  struct message *message ;	  /* message occupying this lane */ 
  int            first ;	  /* first flit of  the message in the lane */
  int		 nr_flits ;	  /* number of flits in the lane */
  int            lane_id_nr ;     /* fixed lane identity */
  int            DR ;             /* changing dimension numbers */ 
  int            type ;           /* lane type */
  int            init ;           /* first lane of the message, if FIRST */
  int            priority ;       /* priority of this lane   */
  int            misrouted ;      /* a flag of misrouted lane */
  struct lane    *prev ;	  /* previous lane */
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
  struct node    *dest_node ;	/* destination node pointer */
  struct node    *src_node ;    /* source node pointer */
  int            M_DR ;         /* changing dimension number */  
  int            wait_count ;   /* number of waiting times */
  int            chang_dir_nr ; /* changing direction number */
  int            state ;	/* for routing */
  int            misrouted ;    /* selected misrouted channel */ 
  int            face ;         /* set in using special lane */
  int            length ;	/* number of words in message */
  struct lane    *head_lane ;	/* pointer to lane containing head */
  int            start_time ;	/* time message originated */
  struct message *next ;	/* for free list */
} ;
typedef struct message MESSAGE ;
/*---------------------------------------------------------------------------*/
/*
 |    A message source generates messages for input to the network.
 |    It contains a queue of messages waiting to be injected, a pointer
 |    to the channel it feeds, the destination object address and
 |    scheduling parameters.
 |    Destination addresses are previously determined in network creating 
 |    stage in this simulator but it is possible to change those addresses
 |    in real time in the revised version.    
 |    The source type indicates the traffic pattern (destination selection).
 |    The field "sat" if true indicates that the source is saturating the
 |    network and will always be generating messages - the queue is ignored
 |    for saturation sources.
 */
/*---------------------------------------------------------------------------*/
struct source {
  int             source_nr ;   	/* index of this source in array */
  struct queue    *message_queue ;	/* queue of messages awaiting entry */
  struct node     *node ;	        /* source node */
  struct node     *dest_node ;          /* final destination node */
  int             type ;	        /* type of source */
  int             interval ;    	/* scheduling interval */
  int             length ;      	/* message length */
  int             sat ;	        	/* saturation source - always busy */
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
  struct message *message ;
  struct lane *lane ; 
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
  struct event *next ;		  /* pointer to next event in list */
  void (*funct)(SOURCE *source) ; /* function to be performed */
  SOURCE *source ;			  /* pointer to source */
} ;
typedef struct event EVENT ;
typedef struct event *TIMESTEP ;
/*---------------------------------------------------------------------------*/
struct stats {
  int start_time ;
  int interval_time ;
  int nr_messages_sent ;
  int nr_messages_injected ;
  int nr_messages_received ;
  int nr_messages_received_interval ;
  int nr_misrouted_messages ;
  int total_latency ;
  int total_latency_sq ;	/* squared - for variance */
  int total_source_latency ;
  int interval_latency ;
  int nr_scratched_messages ;
  int *latency_histogram ;
  int *nr_received_class ;
  int *nr_scr_received_class ;
  int max_hist ;
  int nr_failure_next_lane_finding ;
  int nr_rejected_in_lane_allocation ;
  int total_nr_occupied_lanes ;
  int nr_channel_requested ;
} ;
typedef struct stats STATS ;
/*---------------------------------------------------------------------------*/
struct set_rec {
  char *name ;			/* variable name */
  int  *var ;			/* pointer to integer variable */
  int  min ;			/* minimum value for var */
  int  max ;			/* maximum - cannot be set out of range */
} ;
typedef struct set_rec SET_REC ;
/*---------------------------------------------------------------------------*/
struct nc_table {
  struct channel *channel ;
  struct lane *lane ;
  struct nc_table *next ;
  struct nc_table *prev ;
  int seq_nr ;
} ;
typedef struct nc_table NC_TABLE ;
/*---------------------------------------------------------------------------*/
/* DEFINES */
/*---------------------------------------------------------------------------*/
#define MAX_LANES 32
#define MAX_CLASS_i 20
#define MAX_TIMEOUT_COUNT 200
#define MAX_DEPTH 256
#define INIT_MEM_SIZE (1<<22)
#define NR_TIMESTEPS (1<<15)
#define NULL          0
#define NULL_NODE     ((NODE *)0)
#define NULL_CHANNEL  ((CHANNEL *)0)
#define NULL_LANE     ((LANE *)0)
#define NULL_MESSAGE  ((MESSAGE *)0) 
#define NULL_REQUEST  ((REQUEST *)0)
#define NULL_EVENT    ((EVENT *)0)
#define NULL_TIMESTEP ((TIMESTEP *)0)
#define NULL_STATS    ((STATS *)0)
#define NULL_NC_TABLE ((NC_TABLE *)0) 
#define POS 0
#define NEG 1 
#define RANDOM 1
#define BIT_REVERSAL 2
#define ROW_LINE 3
#define REFLECTION 4
#define ADAPTIVE 1
#define NONADAPTIVE 2
#define SHORTEST_PATH 1
#define ANY_CHANNEL 2
#define DYNAMIC 3
#define BOTH 3
#define SPECIAL 4
#define NORMAL 0
#define SCRATCHED 4
#define INJECTION 5
#define FAULT_TEST 1
/* source types */
#define S_SPOT 0		/* random traffic */
#define S_RANDOM 1		/* random traffic */
#define S_REVERSE 2		/* reverse permutation  0->n-1, 1->n-2, etc */
#define MAX_SOURCE_TYPE 3

/* channel scheduling algorithms */
#define CS_FIRST 0		/* advance first request */
#define CS_DEADLINE 1		/* advance oldest message */
#define MAX_CHAN_SCHED 2

/* random maximums */
#define MAX_SOURCE_LENGTH (1<<10)
#define MAX_SOURCE_INTERVAL (1<<20)
#define MAX_NR_CYCLES (1<<20)

/* trffic types */
#define DETERMINISTIC 0
#define STATIC 1
#define STATIC_DBP 2
#define DYNAMIC 3
#define HYBRID 4

/* message states */
#define MESSAGE_INIT  0	        /* initialized */
#define MESSAGE_NEED_OUTPUT  1	/* awaiting assignment of output lane */
#define MESSAGE_FOUND_LANE  2	/* message found next empty lane */
#define MESSAGE_FAILED_LANE  3  /* message failed to find next lane */
#define MESSAGE_NEED_CHANNEL  4	/* awaiting use of channel */
#define MESSAGE_ADVANCED  5	/* head lane advanced flits */
#define MESSAGE_DONE  6		/* message delivery completed */
#define MAX_MESSAGE_STATE  7

/* lane states */
#define FIXED 1
#define FREED 2
#define IDLE  0
#define FIRST 1
#define OCCUPIED  2
#define DISCONNECTION  5
#define CHAN_FAIL  6
#define REACHED_DEST  1
#define NOT_REACHED_DEST 0

/* debug options - bit masks*/
#define DEBUG_CREATION	1
#define DEBUG_ROUTING	2
#define DEBUG_ADVANCING 4
#define DEBUG_MESSAGE   8 
#define DEBUG_EVENTS    0x10
/*---------------------------------------------------------------------------*/
#define MESSAGE_CHANNEL(mesage) (message->head_lane->channel)
#define MESSAGE_NODE(message) (MESSAGE_CHANNEL(message)->dst)
#define MESSAGE_ID_SEQ(message) (MESSAGE_NODE(message)->id_seq)
#define MESSAGE_SLOT(message) (MESSAGE_CHANNEL(message)->dst_slot)
/*---------------------------------------------------------------------------*/
#define SOURCE_NODE(source)    (source->dst->dst)
#define SOURCE_NODE_NR(source) (SOURCE_NODE(source)->node_nr)
#define SOURCE_SLOT(source)    (source->dst->dst_slot)
#define SOURCE_CHANNEL(source) (source->dst)
/*---------------------------------------------------------------------------*/
#define LANE_NODE(lane) (lane->channel->dst)
#define LANE_NODE_NR(lane) (LANE_NODE(lane)->id_seq)
#define LANE_FULL(lane) (lane->nr_flits >= para->depth)
#define LANE_EMPTY(lane) (lane->nr_flits == 0)  
#define LANE_BUSY(lane) ((lane)->message)
#define LANE_SATURATED(lane) (lane->nr_flits + lane->first >= para->slength)
/*---------------------------------------------------------------------------*/
#define CHANNEL_ACTIVE(channel) (channel->lane_requests || \
				 channel->channel_requests)
/*---------------------------------------------------------------------------*/
/* GLOBALS */
/*---------------------------------------------------------------------------*/
extern NODE *nodes ;	          /* array of nodes */
extern INTARY **temp ;           /* temporary array */
extern MESSAGE *free_messages ;	  /* list of free message structures */
extern MESSAGE *active_messages ; /* list of active message structures */
extern REQUEST *free_requests ;	  /* list of free request structures */
extern CHANNEL *active_channels ; /* list of channels with requests pending */
extern SOURCE *sources ;          /* array of sources */
extern TIMESTEP *event_list ;     /* array of timesteps */
extern EVENT *free_events ;	  /* free event list */
extern PARA *para ;               /* paramater */
extern PARA_C *para_c ;           /* channel paramater */
extern SET_REC *set_table ;       /* variable set table */
extern STATS *cur_stats ;         /* current stats */
extern int nr_nodes ;		  /* number of nodes */
extern int nr_sources ;		  /* number of sources in array */
extern int cur_time ;		  /* the current time */
extern int cur_index ;		  /* the position of the current timestep in */
extern int debug ;		  /* debugging bit mask */
extern int nr_cycles ;		  /* number of cycles to run a test */
extern int drain_messages ;	  /* if true, sources disabled */
/*---------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------*/
extern void error(char *s)  ;	/* error exit routine */
/*---------------------------------------------------------------------------*/

