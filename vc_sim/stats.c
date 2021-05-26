/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator - statistics functions
 |    (C) Bill Dally,  July 2, 1989
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include "defs.h"
#include "netsim.h"
#include "bfly.h"
#include "message.h"
#include "event.h"
/*---------------------------------------------------------------------------*/
extern double sqrt(double x) ;
/*---------------------------------------------------------------------------*/
#define HIST_SIZE (1<<8)
/*---------------------------------------------------------------------------*/
STATS *cur_stats ;
STATS *free_stats ;
/*---------------------------------------------------------------------------*/
STATS *new_stats()
{
  STATS *stats ; int i ;
  if(free_stats) {
    stats = free_stats ;
    free_stats = free_stats->next ;
    stats->next = NULL_STATS ;
  }
  else {
    stats = NEW_STRUCT(STATS) ;
  }
  stats->start_time = cur_time ;
  stats->nr_messages_sent = 0 ;
  stats->nr_messages_received = 0 ;
  stats->total_latency = 0 ;
  stats->total_latency_sq = 0 ;
  stats->max_hist = HIST_SIZE ;
  stats->latency_histogram = NEW_ARRAY(int,HIST_SIZE) ;
  for(i=0;i<HIST_SIZE;i++) stats->latency_histogram[i] = 0 ;
  stats->lane_hist = NEW_ARRAY(int, (MAX_LANES+1)) ;
  for(i=0;i<(MAX_LANES+1);i++) stats->lane_hist[i] = 0 ;
  return(stats) ;
}
/*---------------------------------------------------------------------------*/
void init_stats()
{
  free_stats = NULL_STATS ;
  cur_stats = new_stats() ;
}
/*---------------------------------------------------------------------------*/
void reset_stats()
{
  if(cur_stats) {
    cur_stats->next = free_stats ;
    free_stats = cur_stats ;
  }
  cur_stats = new_stats() ; 
}
/*---------------------------------------------------------------------------*/
void log_lane_histogram(int stage) /* stage of butterfly only */
{
  int node_nr, chan_nr ;
  int nr_busy ;
  CHANNEL *channel ;

  for(node_nr = height*stage ; node_nr < height*(stage+1) ; node_nr++) {
    for(chan_nr = 0 ; chan_nr < k ; chan_nr++) {
      channel = &(nodes[node_nr].inputs[chan_nr]) ;
      nr_busy = nr_lanes_busy(channel) ;
      if(nr_busy < (MAX_LANES+1)) {
	cur_stats->lane_hist[nr_busy] += 1 ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void print_lane_hist()
{
  int i ;
  double total = 0.0 ;
  for(i=0;i<(MAX_LANES+1);i++) {
    total += cur_stats->lane_hist[i] ;
  }
  for(i=0;i<(MAX_LANES+1);i++) {
    printf("%d, %f\n", i, (double)(cur_stats->lane_hist[i])/total) ;
  }
}
/*---------------------------------------------------------------------------*/
void log_message_receipt(MESSAGE *message)
{
  int latency ;
  /* if logging is enabled */
  if(cur_stats && ((!vstat) || (message->priority < LOW_PRIORITY))) {
    latency = cur_time - message->start_time ;
    cur_stats->nr_messages_received++ ;
    cur_stats->total_latency += latency ;
    cur_stats->total_latency_sq += latency*latency ;
    if(latency < cur_stats->max_hist) {
      cur_stats->latency_histogram[latency]++ ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void log_message_injection(MESSAGE *message)
{
  if(cur_stats && ((!vstat) || (message->priority < LOW_PRIORITY))) {
    cur_stats->nr_messages_injected++ ;
    cur_stats->total_source_latency += (cur_time - message->start_time) ; 
  }
}
/*---------------------------------------------------------------------------*/
void log_message_creation(MESSAGE *message)
{
  if(cur_stats &&  ((!vstat) || (message->priority < LOW_PRIORITY))) {
    cur_stats->nr_messages_sent++ ;
  }
}
/*---------------------------------------------------------------------------*/
double flits_per_source_cycle(STATS *stats)
{
  return((double)(stats->nr_messages_received * slength)/
	 (double)((cur_time - stats->start_time) * nr_sources)) ;
}
/*---------------------------------------------------------------------------*/
double avg_net_latency(STATS *stats)
{
  int total_net_latency =
    stats->total_latency - stats->total_source_latency ;
  return((double)total_net_latency/
	 (double)stats->nr_messages_received) ;
}
/*---------------------------------------------------------------------------*/
double avg_total_latency(STATS *stats)
{
  return((double)stats->total_latency/
	 (double)stats->nr_messages_received) ;
}
/*---------------------------------------------------------------------------*/
double std_dev_latency(STATS *stats)
{
  double mean = avg_total_latency(stats) ;
  double var = (double)stats->total_latency_sq/
	       (double)stats->nr_messages_received ;
  double zmvar = var - (mean*mean) ;
  double stdev = (zmvar >= 0) ? sqrt(zmvar) : 0.0 ;
  /* printf("m %10.3g v %10.3g zmv %10.3g std %10.3g\n",
	 mean, var, zmvar, stdev) ;*/
  return(stdev) ;
}
/*---------------------------------------------------------------------------*/
void print_stats(STATS *stats)
{
  printf("STATISTICS from %d to %d\n", 
	 stats->start_time, cur_time) ;
  printf("MESSAGES %d sent %d injected %d received\n",
	 stats->nr_messages_sent, stats->nr_messages_injected,
	 stats->nr_messages_received) ;
  if(stats->nr_messages_received) {
    printf("AVG LATENCY %8.3g total %8.3g source %8.3g net \n", 
	   avg_total_latency(stats), 
	   (double)stats->total_source_latency/
	   (double)stats->nr_messages_injected,
	   avg_net_latency(stats) 
	   ) ;
    printf("LATENCY STD_DEV %8.3g\n",
	   std_dev_latency(stats)) ; 
  }
  if(cur_time - stats->start_time) {
    printf("MESSAGES PER CYCLE = %8.3g\n FLITS PER CYCLE %8.4g \
 FLITS PER SOURCE-CYCLE %8.4g\n",
	   (double)stats->nr_messages_received/
	   (double)(cur_time - stats->start_time),
	   (double)(stats->nr_messages_received * slength)/
	   (double)(cur_time - stats->start_time),
	   flits_per_source_cycle(cur_stats)
	   ) ;
  }
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
void print_latency_histogram(STATS *stats)
{
  int i, max = 0 ;

  /* find maximum latency */
  for(i=0;i<HIST_SIZE;i++) {
    if(stats->latency_histogram[i]) max = i ; 
  }
  
  /* for each bin, print a number */
  for(i=0;i<=max;i++) {
    printf("%d:%d ",i,stats->latency_histogram[i]) ;
    if((i % 15) == 14) printf("\n") ;
  }
  printf("\n") ;
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/

