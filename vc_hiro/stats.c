/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator - statistics functions
 |    Hiromichi Aoki July 27 1990
 */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include "netdef.h"
#include "defs.h"
#include "event.h"
/*---------------------------------------------------------------------------*/
extern double sqrt(double x) ;
/*---------------------------------------------------------------------------*/
#define HIST_SIZE (1<<12)
/*---------------------------------------------------------------------------*/
STATS *cur_stats ;
STATS *free_stats ;
/*---------------------------------------------------------------------------*/
STATS *new_stats()
{
  STATS *stats ;
  stats = NEW_STRUCT(STATS) ;
  stats->start_time = cur_time ;
  stats->nr_messages_sent = 0 ;
  stats->nr_messages_injected = 0;
  stats->nr_messages_received = 0 ;
  stats->total_latency = 0 ;
  stats->total_latency_sq = 0 ;
  stats->nr_misrouted_messages = 0 ;
  stats->max_hist = HIST_SIZE ;
  stats->latency_histogram = NEW_ARRAY(int,HIST_SIZE) ;
  stats->nr_received_class = NEW_ARRAY(int,100) ;
  stats->nr_scr_received_class = NEW_ARRAY(int,100) ;
  stats->nr_failure_next_lane_finding = 0 ;
  stats->nr_rejected_in_lane_allocation = 0 ;
  stats->nr_removed_deadlocked_messages = 0 ; 
  stats->nr_removed_livelocked_messages = 0 ; 
  stats->total_nr_occupied_lanes = 0 ;
  stats->nr_channel_requested =0 ;
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
  cur_stats = new_stats() ; 
}
/*---------------------------------------------------------------------------*/
void set_class_scratched_received(MESSAGE *message)
{
  int MDR ;

  MDR = message->M_DR ;
  if(message->face == SCRATCHED) {
    cur_stats->nr_scratched_messages++ ;
    switch(MDR) {
      
    case 0:
      cur_stats->nr_scr_received_class[0]++ ; return ; 
    case 1:
      cur_stats->nr_scr_received_class[1]++ ; return ;
    case 2:
      cur_stats->nr_scr_received_class[2]++ ; return ;
    case 3:
      cur_stats->nr_scr_received_class[3]++ ; return ;
    case 4:
      cur_stats->nr_scr_received_class[4]++ ; return ;
    case 5:
      cur_stats->nr_scr_received_class[5]++ ; return ;
    case 6:
      cur_stats->nr_scr_received_class[6]++ ; return ;
    case 7:
      cur_stats->nr_scr_received_class[7]++ ; return ;
    case 8:
      cur_stats->nr_scr_received_class[8]++ ; return ;
    case 9:
      cur_stats->nr_scr_received_class[9]++ ; return ;
    case 10:
      cur_stats->nr_scr_received_class[10]++ ; return ;
    case 11:
      cur_stats->nr_scr_received_class[11]++ ; return ;
    case 12:
      cur_stats->nr_scr_received_class[12]++ ; return ;
    case 13:
      cur_stats->nr_scr_received_class[13]++ ; return ;
    case 14:
      cur_stats->nr_scr_received_class[14]++ ; return ;
    case 15:
      cur_stats->nr_scr_received_class[15]++ ; return ;
    case 16:
      cur_stats->nr_scr_received_class[16]++ ; return ;
    case 17:
      cur_stats->nr_scr_received_class[17]++ ; return ;
    case 18:
      cur_stats->nr_scr_received_class[18]++ ; return ;
    case 19:
      cur_stats->nr_scr_received_class[19]++ ; return ;
    default:
      cur_stats->nr_scr_received_class[20]++ ; return ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void set_class_received(MESSAGE *message)
{
  int MDR ;

  MDR = message->M_DR ; 

  switch(MDR) {
      
  case 0:
    cur_stats->nr_received_class[0]++ ; return ; 
  case 1:
    cur_stats->nr_received_class[1]++ ; return ;
  case 2:
    cur_stats->nr_received_class[2]++ ; return ;
  case 3:
    cur_stats->nr_received_class[3]++ ; return ;
  case 4:
    cur_stats->nr_received_class[4]++ ; return ;
  case 5:
    cur_stats->nr_received_class[5]++ ; return ;
  case 6:
    cur_stats->nr_received_class[6]++ ; return ;
  case 7:
    cur_stats->nr_received_class[7]++ ; return ;
  case 8:
    cur_stats->nr_received_class[8]++ ; return ;
  case 9:
    cur_stats->nr_received_class[9]++ ; return ;
  case 10:
    cur_stats->nr_received_class[10]++ ; return ;
  case 11:
    cur_stats->nr_received_class[11]++ ; return ;
  case 12:
    cur_stats->nr_received_class[12]++ ; return ;
  case 13:
    cur_stats->nr_received_class[13]++ ; return ;
  case 14:
    cur_stats->nr_received_class[14]++ ; return ;
  case 15:
    cur_stats->nr_received_class[15]++ ; return ;
  case 16:
    cur_stats->nr_received_class[16]++ ; return ;
  case 17:
    cur_stats->nr_received_class[17]++ ; return ;
  case 18:
    cur_stats->nr_received_class[18]++ ; return ;
  case 19:
    cur_stats->nr_received_class[19]++ ; return ;
  default:
    cur_stats->nr_received_class[20]++ ; return ;
  }
}
/*---------------------------------------------------------------------------*/
void log_message_receipt(MESSAGE *message)
{
  int latency ;
  int MDR ;
  /* if logging is enabled */
  MDR = message->M_DR ;
  if(cur_stats) {
    latency = cur_time - message->start_time ;
    cur_stats->nr_messages_received++ ;
    cur_stats->nr_messages_received_interval++ ;
    cur_stats->total_latency += latency ;
    cur_stats->interval_latency += latency ;
    cur_stats->total_latency_sq += latency*latency ;
    set_class_received(message) ;
    set_class_scratched_received(message) ;
    if(message->misrouted) cur_stats->nr_misrouted_messages++ ;
    if(latency < cur_stats->max_hist) {
      cur_stats->latency_histogram[latency]++ ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void log_message_injection(MESSAGE *message)
{
  if(cur_stats) {
    cur_stats->nr_messages_injected++ ;
    cur_stats->total_source_latency += (cur_time - message->start_time) ; 
  }
}
/*---------------------------------------------------------------------------*/
void log_message_creation(MESSAGE *message)
{
  if(cur_stats) {
    cur_stats->nr_messages_sent++ ;
  }
}
/*---------------------------------------------------------------------------*/
double flits_per_source_cycle(STATS *stats)
{
  return((double)(stats->nr_messages_received * para->slength)/
	 (double)((cur_time - stats->start_time) * nr_sources)) ;
}
/*---------------------------------------------------------------------------*/
double received_per_injected(STATS *stats)
{
  return((double)(stats->nr_messages_received)/
	 (double)(stats->nr_messages_injected)) ;
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
double avg_interval_latency(STATS *stats)
{
  return((double)(stats->interval_latency)/
	 (double)(stats->nr_messages_received_interval)) ;
}
/*---------------------------------------------------------------------------*/
double std_dev_latency(STATS *stats)
{
  double mean = avg_total_latency(stats) ;
  double var = (double)stats->total_latency_sq/
	       (double)stats->nr_messages_received ;
  double zmvar = var - (mean*mean) ;
  double stdev = sqrt(zmvar) ;
  /* printf("m %10.3g v %10.3g zmv %10.3g std %10.3g\n",
	 mean, var, zmvar, stdev) ;*/
  return(stdev) ;
}
/*---------------------------------------------------------------------------*/
void print_received_class(STATS *stats) 
{
  int i, nr_received_i, nr_scratched ;
  double rate ;

  for(i=0; i<20; i++) {
    nr_received_i = stats->nr_received_class[i] ;
    rate = (double)(nr_received_i)*100.0/(double)(stats->nr_messages_received);
    if(nr_received_i) {
      printf("  class_%d   messages = %6d   rate = %4.2f\n",
	     i, nr_received_i, rate) ;
    }
  }
  nr_received_i = stats->nr_received_class[20] ;
  rate = (double)(nr_received_i)*100.0/(double)(stats->nr_messages_received) ;
  if(nr_received_i) {
    printf("  class >20 messages = %6d   rate = %4.2f\n", 
	   nr_received_i, rate) ;
  }
  nr_scratched = stats->nr_scratched_messages ;
  rate = (double)(nr_scratched)*100.0/(double)(stats->nr_messages_received) ;
  printf("  scratched messages = %6d   rate = %6.5f\n",nr_scratched,rate) ;
  for(i=0; i<20; i++) {
    nr_received_i = stats->nr_scr_received_class[i] ;
    rate =(double)(nr_received_i)*100.0/(double)(stats->nr_messages_received) ;
    if(nr_received_i) {
      printf("   class_%d scratched = %6d   rate = %6.5f\n",
	     i,nr_received_i,rate) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void print_misrouted_messages(STATS *stats) 
{
  int nr_misrouted, nr_messages_received ;

  nr_misrouted = stats->nr_misrouted_messages ;
  nr_messages_received = stats->nr_messages_received ;
  printf("  misrouted_messages = %6d   rate = %6.5f%\n", 
	 nr_misrouted,
	 (double)(nr_misrouted)*100.0/(double)(nr_messages_received)) ;
}
/*---------------------------------------------------------------------------*/
void print_stats(STATS *stats)
{
  double average_occupied_lanes ;

  printf("STATISTICS from %d to %d\n", 
	 stats->start_time, cur_time) ;
  printf("MESSAGES %d sent %d injected %d received\n",
	 stats->nr_messages_sent, stats->nr_messages_injected,
	 stats->nr_messages_received) ;
  if(stats->nr_messages_received) {
    printf("AVG LATENCY %8.3f total %8.3f source %8.3f net \n", 
	   avg_total_latency(stats), 
	   ((double)stats->total_source_latency)/
	   ((double)stats->nr_messages_injected),
	   avg_net_latency(stats) 
	   ) ;
    printf("LATENCY STD_DEV %8.3g\n",
	   std_dev_latency(stats)) ; 
  }
  if(cur_time - stats->start_time) {
    printf("MESSAGES PER CYCLE = %8.3g\n FLITS PER CYCLE %8.4g \
 RECEIVED PER INJECTED %8.4g\n\n",
	   (double)stats->nr_messages_received/
	   (double)(cur_time - stats->start_time),
	   (double)(stats->nr_messages_received * para->slength)/
	   (double)(cur_time - stats->start_time),
	   received_per_injected(cur_stats)
	   ) ;
  }
  printf("  Number of rejected counts in channel finding stage = %d\n", 
	 stats->nr_failure_next_lane_finding) ;
  printf("  Number of rejected counts in lane allocation stage = %d\n", 
	 stats->nr_rejected_in_lane_allocation) ;
  average_occupied_lanes = (double)(stats->total_nr_occupied_lanes)/
                                   (double)(stats->nr_channel_requested) ;
  printf("  total_nr_occupied_lanes = %d  nr_channel_requested = %d\n", 
	 stats->total_nr_occupied_lanes, stats->nr_channel_requested) ;
  printf("  Number of average occupied lanes in chan request stage = %5.3f\n",
	 average_occupied_lanes) ;
  print_received_class(cur_stats) ;
  print_misrouted_messages(cur_stats) ; 
  if(para->test_category == FAULT_TEST) {
    if(stats->nr_removed_deadlocked_messages != NULL) {
      print_deadlock_on_message_fault() ;
    }
    if(stats->nr_removed_livelocked_messages != NULL) {
      print_livelock_on_message_fault() ;
    }
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

