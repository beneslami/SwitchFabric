/*---------------------------------------------------------------------------*/
/*
 |    network simulator commands
 |    (C) Bill Dally - 3-July-1989
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "bfly.h"
#include "netsim.h"
#include "event.h"
#include "source.h"
#include "stats.h"
#include "set.h"
#include "cmds.h"
/*---------------------------------------------------------------------------*/
#define LOG_MAX_LANES 4
#define CYCLES_PER_TEST 20000
#define WARM_CYCLES 10000
#define MAX_LAT 500
/*---------------------------------------------------------------------------*/
#define PRINT_INTERVAL 32
#define SCREEN_WIDTH   70
/*---------------------------------------------------------------------------*/
char sx[256] ;
/*---------------------------------------------------------------------------*/
void print_activity(int j)
{
  int k = PRINT_INTERVAL * SCREEN_WIDTH ;
  if((j%PRINT_INTERVAL) == 0) { printf(".") ; fflush(stdout) ;}
  if((j%k) == (k-1)) { printf("\n") ;} 
}
/*---------------------------------------------------------------------------*/
void do_update()
{
  SOURCE *source ;
  NODE *node ;
  CHANNEL *channel ;
  LANE *lane ;
  int i,j ;
  
  /* update sources */
  for(i=0;i<nr_sources;i++) {
    source = &sources[i] ;
    source->type = stype ;
    source->interval = sinterval ;
    source->length = slength ;
    source->sat = ssat ;
  }

  /* update lanes */
  for(i=0;i<nr_nodes;i++) {
    node = &nodes[i] ;
    for(j=0;j<node->nr_inputs;j++) {
      channel = &(node->inputs[j]) ;
      channel->nr_lanes = nr_lanes ; /* update width of channel */
    }
  }
  updated = 1 ;
}
/*---------------------------------------------------------------------------*/
void do_reset()
{
  if(!graph)printf("Initializing Network--") ;
  do_clear_sources() ;
  do_drain_messages() ;
  reset_event_list() ;
  reset_stats() ;
  schedule_initial_source_events() ; /* start up sources */
  if(!graph)printf("Network Initialized\n") ;
}
/*---------------------------------------------------------------------------*/
#define MAX_CYCLES 1000000
void do_run(int i)
{
  int j ;

  if(!graph)printf("Running for %d cycles\n", i) ;
  if((i>0) && (i<MAX_CYCLES)) {
    for(j=0;j<i;j++) {
      cycle_network() ;
      if(lanehist)log_lane_histogram(lh_stage) ;
      if(!graph)print_activity(j) ;
    }
  }
  if(!graph)printf("--Done\n") ;
}
/*---------------------------------------------------------------------------*/
void do_clear_sources()
{
  int i ;
  for(i=0;i<nr_sources;i++) {
    free_messages_in_queue(&(sources[i])) ;
  }
}
/*---------------------------------------------------------------------------*/
void do_drain_messages()
{
  int i = 0;
  drain_messages = 1 ;
  if(!graph)printf("Draining") ;
  while(active_messages) {
    cycle_network() ;
    if(!graph)print_activity(i++) ;
  }
  drain_messages = 0 ;
  check_messages_and_requests() ;
  if(!graph)printf("--Done\n") ;
}
/*---------------------------------------------------------------------------*/
void do_run_test_cycle()
{
  do_update() ;
  do_reset() ;
  do_run(warm_cycles) ;
  reset_stats() ;
  do_run(nr_cycles) ;
  if(!graph)do_show_vars() ;
  if(!graph)print_stats(cur_stats) ;
  if(lanehist)print_lane_hist() ;
}
/*---------------------------------------------------------------------------*/
void lane_width_test()
{
  int i ;
  double thruput[LOG_MAX_LANES+1] ;
  int    delivered[LOG_MAX_LANES+1] ;

  for(i=0;i<=LOG_MAX_LANES;i++) { 
    nr_lanes = 1<<i ;
    if(unbuffered) depth = 1 ;
    else           depth = 1<<(LOG_MAX_LANES-i) ;
    ssat = 1 ;
    warm_cycles = WARM_CYCLES ;
    nr_cycles = CYCLES_PER_TEST ;
    do_run_test_cycle() ;
    thruput[i] = flits_per_source_cycle(cur_stats) ;
    delivered[i] = cur_stats->nr_messages_received ;
    if(graph) printf("%d, %8.4f\n",nr_lanes,thruput[i]) ;
    do_reset() ;
  }
  if(!graph)printf("\n\nLANE WIDTH TEST SUMMARY\n\n") ;
  if(!graph)printf("LANES DEPTH   THROUGHPUT   MESSAGES\n") ;
  for(i=0;i<=LOG_MAX_LANES;i++) {
    if(!graph)printf("%5d %5d   %10.4g   %8d\n",
	   1<<i,1<<(LOG_MAX_LANES-i),thruput[i],delivered[i]) ;
  }
  if(!graph)printf("\n") ;
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
void last_width_test()
{
  int i=LOG_MAX_LANES ;
  nr_lanes = 1<<i ;
  depth = 1<<(LOG_MAX_LANES-i) ;
  ssat = 1 ;
  nr_cycles = CYCLES_PER_TEST ;
  do_run_test_cycle() ;
  if(!graph)printf("LANES %5d DEPTH %5d THRU %10.4g DELIVERED %8d\n",
	 nr_lanes, depth, 
	 flits_per_source_cycle(cur_stats),
	 cur_stats->nr_messages_received) ;
  do_reset() ;
}     
/*---------------------------------------------------------------------------*/
#define NR_INTERVALS 18
int pct_load[] = {1,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90} ;
void lane_width_latency_test()
{
  int i, j ;
  double lat ;
  double latency[(LOG_MAX_LANES+1)*NR_INTERVALS] ;

  if(graph) {
    printf("; k = %d n = %d unbuffered = %d expsrc = %d\n",
	   1<<k, n, unbuffered, expsrc) ;
    splot_prefix() ;
    splot_postfix("Traffic(fraction of capacity)",
		  "Latency(cycles)",
		  0.0, 1.0, 0.2, 0.0, 300.0, 50.0) ;
    printf("X Numberstyle floating 1\n") ;
    printf("Y unit 1\n\n") ;
  }
  for(i=0;i<=LOG_MAX_LANES;i++) { 
    nr_lanes = 1<<i ;
    if(unbuffered) depth = 1 ;
    else           depth = 1<<(LOG_MAX_LANES-i) ;
    if(graph){
      if(nr_lanes == 1) sprintf(sx,"1 Lane") ;
      else              sprintf(sx,"%d Lanes",nr_lanes) ;
      new_curve(sx, i) ;
      fflush(stdout) ;
    }
    for(j=0;j<NR_INTERVALS;j++) {
      /* set up offered traffic */
      sinterval = (100 * slength)/pct_load[j] ;
      rate = (double)pct_load[j]/100.0 ;
      ssat = 0 ;
      warm_cycles = WARM_CYCLES ;
      nr_cycles = CYCLES_PER_TEST ;
      do_run_test_cycle() ;
      if(!graph)print_latency_histogram(cur_stats) ;
      lat = latency[i*NR_INTERVALS+j] = avg_total_latency(cur_stats) ;
      if(graph) {
	printf("%7.4f, %7.4f\n", 
	       flits_per_source_cycle(cur_stats), 
	       lat) ;
	fflush(stdout) ;
      }
      do_reset() ;
      if( lat > MAX_LAT) break ; 
    }
  }
  if(!graph)printf("\n\nLANE WIDTH LATENCY TEST SUMMARY\n\n") ;
  if(!graph)printf("LANES DEPTH TRAFFIC   LATENCY \n") ;
  for(i=0;i<=LOG_MAX_LANES;i++) {
    for(j=0;j<NR_INTERVALS;j++) {
      if(!graph)printf("%5d %5d %8.3g %10.4g   %8d\n",
	     1<<i,1<<(LOG_MAX_LANES-i), 
	     (double)pct_load[j]/(double)100.0,
	     latency[i*NR_INTERVALS+j]) ;
    }
  }
  if(!graph)printf("\n") ;
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
#define NR_DEADLINE_RUNS 3
void deadline_sched_test()
{
  int i ;
  int pct_load[] = {25,50,75} ;
  /* Control run - normal scheduling */
  printf("DEADLINE SCHEDULING EXPERIMENT\n") ;
  for(i=0;i<NR_DEADLINE_RUNS;i++) {
    chan_sched = CS_FIRST ;
    sinterval = (100 * slength)/pct_load[i] ;
    nr_lanes = 8 ;
    depth = 2 ;
    ssat = 0 ;
    nr_cycles = CYCLES_PER_TEST ;
    do_run_test_cycle() ;
    print_latency_histogram(cur_stats) ;
    do_reset() ;
    /* run with deadline scheduling */
    chan_sched = CS_DEADLINE ;
    do_run_test_cycle() ;
    print_latency_histogram(cur_stats) ;
    do_reset() ;
  }
}
/*---------------------------------------------------------------------------*/
void new_deadline_test()  /* abbreviated */
{
  printf("NEW DEADLINE EXPERIMENT\n") ;
  chan_sched = CS_RANDOM ;
  sinterval = (100 * slength)/50 ;
  nr_lanes = 8 ; depth = 2 ;
  ssat = 0 ;
  nr_cycles = CYCLES_PER_TEST ;
  do_run_test_cycle() ;  
  print_latency_histogram(cur_stats) ;
  do_reset() ;
  chan_sched = CS_DEADLINE ;
  do_run_test_cycle() ;
  print_latency_histogram(cur_stats) ;
  do_reset() ;
  chan_sched = CS_FIRST ;
  do_run_test_cycle() ;
  print_latency_histogram(cur_stats) ;
  do_reset() ;
}
/*---------------------------------------------------------------------------*/
/* as above but with 10% of traffic being high priority voice */
#define VPCT 10
void voice_data_sched_test()
{
  int i ;
  int pct_load[] = {25,50,75} ;
  /* Control run - normal scheduling */
  printf("SCHEDULING EXPERIMENT, %d PERCENT PRIORITY TRAFFIC \n", VPCT) ;
  for(i=0;i<NR_DEADLINE_RUNS;i++) {
    chan_sched = CS_FIRST ;
    stype = S_VOICE_DATA ;
    spct = VPCT ;
    sinterval = (100 * slength)/pct_load[i] ;
    nr_lanes = 8 ;
    depth = 2 ;
    ssat = 0 ;
    nr_cycles = CYCLES_PER_TEST ;
    vstat = 0 ;
    do_run_test_cycle() ;
    print_latency_histogram(cur_stats) ;
    do_reset() ;
    vstat = 1 ;
    do_run_test_cycle() ;
    print_latency_histogram(cur_stats) ;
    do_reset() ;
    /* run with deadline scheduling */
    chan_sched = CS_DEADLINE ;
    vstat = 0 ;
    do_run_test_cycle() ;
    print_latency_histogram(cur_stats) ;
    do_reset() ;
    vstat = 1 ;
    do_run_test_cycle() ;
    print_latency_histogram(cur_stats) ;
    do_reset() ;
  }
}
/*---------------------------------------------------------------------------*/
void do_test()
{
    sinterval = 100 ;
    nr_lanes = 8 ;
    ssat = 0 ;
    depth = 2 ;
    chan_sched = CS_DEADLINE ;
    stype = S_VOICE_DATA ;
    spct = VPCT ;
    vstat = 1 ;
    nr_cycles = 100 ;
    do_run_test_cycle() ;
}
/*---------------------------------------------------------------------------*/
void occupancy_test()
{
  lanehist = 1 ;
  lh_stage = 0 ;
/*  expsrc = 1 ;*/
  unbuffered = 1 ;
  depth = 1 ;
  nr_lanes = 4 ;
  sinterval = 2 * slength ;
  rate = 0.5 ;
  ssat = 0 ;
  warm_cycles = WARM_CYCLES ;
  nr_cycles = CYCLES_PER_TEST ;
  do_run_test_cycle() ;
}
/*---------------------------------------------------------------------------*/
void do_experiment(int experiment_nr)
{
  printf("\nEXPERIMENT %d\n",experiment_nr) ;
  switch(experiment_nr) {
  case 1:
    lane_width_test() ; return ;
  case 2:
    lane_width_latency_test() ; return ;
  case 3:
    deadline_sched_test() ; return ;
  case 4:
    last_width_test() ; return ;
  case 5:
    voice_data_sched_test() ; return ;
  case 6:
    new_deadline_test() ; return ;
  case 7:
    occupancy_test() ; return ;
  default:
    printf("Unknown experiment\n") ;
    return ;
  }
}
/*---------------------------------------------------------------------------*/
