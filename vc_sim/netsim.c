/*---------------------------------------------------------------------------*/
/*
 |    Netsim.c - network simulator
 |    (C) Bill Dally - 3-May-1989
 |
 |    Netsim simulates the performance of networks with arbitrary toplogy
 |    and routing that use virtual channel flow control.
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "netsim.h"
#include "bfly.h"
#include "message.h"
#include "event.h"
#include "stats.h"
#include "set.h"
#include "cmds.h"
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------*/
void init_net() ;
void cycle_network() ;
/*---------------------------------------------------------------------------*/
/* GLOBALS */
/*---------------------------------------------------------------------------*/
NODE *nodes ;			/* array of nodes */
int nr_nodes ;			/* number of nodes */

MESSAGE *free_messages ;	/* list of free message structures */
MESSAGE *active_messages ;	/* list of active message structures */

REQUEST *free_requests ;	/* list of free request structures */

CHANNEL *active_channels ;	/* list of channels with requests pending */

SOURCE *sources ;		/* array of sources */
int nr_sources ;		/* number of sources in array */

TIMESTEP *event_list ;		/* array of timesteps */
int cur_time ;			/* the current time */
int cur_index ;			/* the position of the current timestep in */
				/* the event list */
EVENT *free_events ;		/* free event list */

/* routing function */
int (*routing_function)(int source_node_nr, int in_channel, int dest) = 
       bfly_route ;
/* destination function - returns true if node is destination */
int (*destination_function)(int node_nr, int dest) = bfly_dest ;

int nr_lanes = 8 ;		/* number of lanes per channel */
int depth = 2 ;			/* depth of each queue */
int debug = 0;			/* debugging bit mask */
				/* 1-bfly creation, 2-routing */
int stype = S_RANDOM ;		/* source type */
int slength = 20 ;		/* source message length in flits */
int sinterval = 10 ;		/* source generation interval */
int ssat = 0 ;			/* source saturates network */
int spct = 0 ;			/* percent of time critical messages in V/D mode */

int chan_sched = CS_FIRST ;	/* allocate channel to first request */

int nr_cycles = 10000 ;		/* number of cycles to run a test */

int drain_messages = 0 ;	/* if true, sources disabled */

int vstat = 0 ;			/* if true, compute stats only for priority  */
				/* messages */
int graph = 0 ;                 /* if true, produce output for splot */
int warm_cycles = 20000 ;        /* number of cycles to overcome transient*/
int expsrc = 0 ;		/* 1 if poisson source */
double rate = 0.0 ;		/* rate of poisson source */
int unbuffered = 0 ;		/* if true, only length one buffers per VC */
int lanehist = 0 ;		/* if true, log lane occupancies */
int lh_stage = 0 ;		/* stage to log */
/*---------------------------------------------------------------------------*/
/*
 |   netsim lk n debug exper graph unbuf expsrc lh_stage
 |
 |   NOTE lk is the LOG of k 
 |
 |   e.g.,
 |
 |      netsim 1 8 0 2 1 1
 |      selects a 2-ary 8-fly, debug mode off, experiment 2 (lane width)
 |      and selects graph mode with an unbuffered network
 |
 |      netsim 1 8 0 1 1 1
 |      runs a throughput experiment (#1) on the same network.
 */  
/*---------------------------------------------------------------------------*/
main(int argc, char *argv[])
{
  int k = 1, n = 3;
  int done ;
  int exper = 0 ;

  /* gobble up argument list */
  if(argc >= 3) {
    k = atoi(argv[1]) ; 
    n = atoi(argv[2]) ;
    if(argc >= 4) debug = atoi(argv[3]) ;
    if(argc >= 5) exper = atoi(argv[4]) ;
    if(argc >= 6) graph = argv[5][0] ;
    if(argc >= 7) unbuffered = argv[6][0] ;
    if(argc >= 8) expsrc = argv[7][0] ;
    if(argc >= 9) {
      lanehist = 1 ;
      lh_stage = atoi(argv[8][0]) ;
    }
    if(!graph)printf("k = %d  n = %d  debug = %d exper = %d\n", 1<<k, n, debug,exper) ;
  } 
  init_net() ;			/* initialize everything */
  new_bfly(k,n) ;		/* make a radix 4, 3-stage b-fly */
  
  /* inject_test_message(6,8,0) ;	/* inject a test message */

  /* if batch experiment mode -- go do it */
  if(exper) {
    do_experiment(exper) ;
  }
  /* otherwise interactive mode */
  else {
    do {
      done = process_command() ;
    } while(!done) ;
  }
}
/*---------------------------------------------------------------------------*/
int process_command() 
{
  char c ;
  char s[256] ;
  int i,j ;

  c = getchar() ;
  switch(c) {
  case 'c':
    cycle_network() ;		/* cycle network until delivered */
    return(0) ;
  case 'd':
    do_drain_messages() ;
    return(0) ;
  case 'e':
    scanf("%d",&i) ;
    do_experiment(i) ;
    return(0) ;
  case 'h':
    do_show_vars() ; return(0) ;
  case 'i':
    do_reset() ; return(0) ;
  case 'p':
    print_stats(cur_stats) ; return(0) ;
  case 'r':
    scanf("%d",&i) ; do_run(i) ; return(0) ;
  case 's':
    scanf("%s %d",s,&i) ;
    do_set_var(s,i) ; return(0) ;
  case 'u':
    do_update() ; return(0) ;
  case '?':
    printf
      ("\nc(ycle), d(rain), e(xper), (s)h(ow), i(nit), p(rint), r(un), s(et),\
 u(pdate), q(uit)\n") ;
    return(0) ;
  case 'q':
    return(1) ;
  default:
    return(0) ;
  }
}
/*---------------------------------------------------------------------------*/
void advance_time()
{
  cur_time++ ;
  cur_index = cur_time % NR_TIMESTEPS ;
  process_events() ;
}
/*---------------------------------------------------------------------------*/
void cycle_network() 
{
  if(debug) printf("\nTIME %04d\n",cur_time) ;
  route_active_messages() ;	/* active messages compete for lanes */
  route_active_channels() ;	/* channels allocate lanes to messages */
  advance_active_messages() ;	/* messages compete for physical channels */
  advance_active_channels() ;	/* channels allocate wires to lanes */
  clean_messages() ;		/* log finished messages and return to free */
  advance_time() ;
}
/*---------------------------------------------------------------------------*/
void init_net()
{
  init_mem(INIT_MEM_SIZE) ;	/* initialize the memory allocator */
  init_messages() ;		/* initialize the message system */
  init_event_list() ;		/* set up the event list */
  init_stats() ;		/* set up statistics */
  init_queues() ;		/* set up queues */
  schedule_initial_source_events() ; /* start up sources */
}
/*---------------------------------------------------------------------------*/
/* UTILITIES */
/*---------------------------------------------------------------------------*/
/* finds the first unconnected input slot of a node and returns its index */
/* returns -1 if not found */
/*---------------------------------------------------------------------------*/
int first_open_input(NODE *node)
{
  int i ;
  for(i=0;i<node->nr_inputs;i++) {
    if(node->inputs[i].src == NULL_NODE) return(i) ;
  }
  return(-1) ;
}
/*---------------------------------------------------------------------------*/
void error(char *s) 
{
  fprintf(stderr,"%s\n",s) ; exit(1) ;
}
/*---------------------------------------------------------------------------*/
int power(int k,int n)
{
  int i,j ;
  j = 1 ;
  for(i=0;i<n;i++) j = j*k ;
  return(j) ; 
} 
/*---------------------------------------------------------------------------*/
