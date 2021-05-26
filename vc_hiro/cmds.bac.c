/*---------------------------------------------------------------------------*/
/*
 |    network simulator commands
 |    Hiromichi Aoki April 20 1990
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include <math.h>
#include "defs.h"
#include "netdef.h"
#include "event.h"
#include "source.h"
#include "stats.h"
#include "set.h"
#include "cmds.h"
/*---------------------------------------------------------------------------*/
#define PRINT_INTERVAL 32
#define SCREEN_WIDTH   70
#define CYCLES_PER_TEST 10000
#define NR_INTERVALS 12
#define NR_TIMES 10
/*---------------------------------------------------------------------------*/
/* Function declaration */
/*---------------------------------------------------------------------------*/
void faulty_test(int nr_fail) ;
void print_disconnected_channels() ;
void channel_disconnect_function() ;
void recover_channel_fault() ;
/*---------------------------------------------------------------------------*/
int pct_load_s[] = {1,2,3,4,5,6,7,8,9,10} ;
int pct_load_l[] = {1,10,20,30,40,45,50,60,70,80,85,90,95} ;
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
    source->type = para->source_type ;
    source->interval = para->sinterval ;
    source->length = para->slength ;
    source->sat = para->ssat ;
  }

  /* update lanes */
  for(i=0;i<nr_nodes;i++) {
    node = &nodes[i] ;
    for(j=0;j<node->nr_inputs;j++) {
      channel = &(node->inputs[j]) ;
      channel->nr_lanes = para->nr_lanes ; /* update width of channel */
    }
  }
  updated = 1 ;
}
/*---------------------------------------------------------------------------*/
void do_reset()
{
  printf("Initializing Network--") ;
  do_clear_sources() ;
  do_drain_messages() ;
  reset_event_list() ;
  reset_stats() ;
  schedule_initial_source_events() ; /* start up sources */
  printf("Network Initialized\n") ;
}
/*---------------------------------------------------------------------------*/
#define MAX_CYCLES 100000
void do_run(int i)
{
  int j ;

  printf("Running for %d cycles\n", i) ;
  if((i>0) && (i<MAX_CYCLES)) {
    for(j=0;j<i;j++) {
      cycle_network() ;
      print_activity(j) ;
    }
  }
  printf("--Done\n") ;
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
  printf("Draining") ;
  while(active_messages) {
    cycle_network() ;
    print_activity(i++) ;
  }
  drain_messages = 0 ;
  check_messages_and_requests() ;
  printf("--Done\n") ;
}
/*---------------------------------------------------------------------------*/
void do_drain_messages_T(int i)
{
  int j = 0;
  drain_messages = 1 ;
  printf("Draining") ;
  if(i > 0) {
    for(j=0; j<i; j++) {
      cycle_network() ;
      print_activity(j) ;
    }
  }    
  drain_messages = 0 ;
  check_messages_and_requests() ;
  printf("--Done\n") ;
}
/*---------------------------------------------------------------------------*/
void do_run_test_cycle()
{
  do_update() ;
  do_reset() ;
  do_run(nr_cycles) ;
  do_show_vars() ;
  print_stats(cur_stats) ;
}
/*---------------------------------------------------------------------------*/
void reset_interval_var()
{
  cur_stats->interval_time = 0 ;
  cur_stats->nr_messages_received_interval = NULL ;
  cur_stats->interval_latency = NULL ;
  para->src_interval_1 = 0 ;
  para->src_interval_2 = 0 ;
  para->src_interval_3 = 0 ;
  para->src_interval_4 = 0 ;
}
/*---------------------------------------------------------------------------*/
void set_class_i(int class, int amount, int cure_nr)
{
  para_c->class_i_exist[class].value = TRUE ;      
  para_c->lowest_VC_nr_class_i[class].value = cure_nr ;
  para_c->highest_VC_nr_class_i[class].value = cure_nr + amount -1 ;    
}
/*---------------------------------------------------------------------------*/
void set_FIX_ONLY_boundary() 
  {
    para_c->special_min_lane_nr = para->nr_lanes - para_c->nr_special_lanes ;
    para_c->special_max_lane_nr = para->nr_lanes -1 ;
    para_c->fixed_min_lane_nr = 0 ;   
    para_c->fixed_max_lane_nr = para->nr_lanes - para_c->nr_special_lanes -1 ;
  }
/*---------------------------------------------------------------------------*/
void set_FIX_FREE_boundary() 
  {
    para_c->special_min_lane_nr = para->nr_lanes - para_c->nr_special_lanes ;
    para_c->special_max_lane_nr = para->nr_lanes -1 ;
    para_c->free_min_lane_nr =  para_c->special_min_lane_nr 
                                         - para_c->nr_free_lanes ;
    para_c->free_max_lane_nr = para_c->special_min_lane_nr -1 ;
    para_c->fixed_min_lane_nr = 0 ;   
    para_c->fixed_max_lane_nr = para_c->nr_fixed_lanes -1 ;
  }
/*---------------------------------------------------------------------------*/
void set_DYNAMIC_boundary() 
  {
    para_c->special_min_lane_nr = para->nr_lanes - para_c->nr_special_lanes ;
    para_c->special_max_lane_nr = para->nr_lanes -1 ;
    para_c->dynamic_min_lane_nr = 0 ;   
    para_c->dynamic_max_lane_nr = para->nr_lanes - para_c->nr_special_lanes -1;
  }
/*---------------------------------------------------------------------------*/
void trafic_latency_test()
{
  int i,j,k,n ;
  double latency_total[(MAX_LANES)*NR_INTERVALS] ;
  double latency_interval[(MAX_LANES)*NR_INTERVALS] ;
  double sum_latency ;
  double avg_latency ;
  double time_interval ;
  double interval ;
  int    delivered[MAX_LANES+1] ;
  int done ;
  k = para->nr_ary ;
  n = para->nr_dim ;
  interval = (double)(k*(para->slength))/4.0 ;
  for(i=0;i<=NR_INTERVALS;i++) { 
    printf("\n\n\n\n") ;
    /****** temporary checking *****/
    /*    do {
    /*	  done = process_command() ;
    /*	  } while(!done) ; */
    /*******************************/
    if(para->tload_mode == 1) {
      printf("For Traffic=%3.1f\n", pct_load_s[i]) ;
      para->sinterval = (int)(100.0*interval/(double)(pct_load_s[i])) ;
    }
    else {
      printf("For Traffic=%3.1f%\n", (double)pct_load_l[i]) ;
      para->sinterval = (int)(100.0*interval/(double)(pct_load_l[i])) ;
    }
    do_update() ;
    do_reset() ;
    do_show_vars() ;
    nr_cycles = CYCLES_PER_TEST/10 ;
    do_run(nr_cycles) ;
    cur_stats->interval_time = cur_time ;
    latency_total[i] = avg_total_latency(cur_stats) ;
    latency_interval[i] = avg_interval_latency(cur_stats) ;
    reset_interval_var() ;
    for(j=1;j<4*NR_TIMES;j++) {
      do_run(nr_cycles) ;
      cur_stats->interval_time = cur_time ;
      latency_total[i*NR_INTERVALS+j] = avg_total_latency(cur_stats) ;
      latency_interval[i*NR_INTERVALS+j] = avg_interval_latency(cur_stats) ;
      reset_interval_var() ;
      if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
    }
    printf("     %d,%6.2f\n", 1000, latency_interval[0]) ;
    sum_latency = 0.0 ;
    for(j=1;j<4*NR_TIMES;j++) {
      printf("     %d,%6.2f\n", (j+1)*1000,
	     latency_interval[i*NR_INTERVALS+j]) ;
      sum_latency = sum_latency + latency_interval[i*NR_INTERVALS+j] ;
      if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
    }
    if(para->tload_mode == 1) {
      printf("\nTraffic = %3.1f\n", pct_load_s[i]) ;
    }
    else {
      printf("\nTraffic = %3.1f%\n", (double)pct_load_l[i]) ;
    }
    avg_latency = sum_latency/(double)(j-1) ;
    print_stats(cur_stats) ;
    printf("\n  Average interval latency = %6.3g\n", avg_latency) ;  
    if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
  }
  printf("\n") ;
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
void latency_test_primitive()
{
  int i,j,k,n ;
  double latency_total[(MAX_LANES)*NR_INTERVALS] ;
  double latency_interval[(MAX_LANES)*NR_INTERVALS] ;
  double sum_latency ;
  double avg_latency ;
  double time_interval ;
  double interval ;
  int    delivered[MAX_LANES+1] ;
  int done ;
  k = para->nr_ary ;
  n = para->nr_dim ;
  interval = (double)(k*(para->slength))/4.0 ;
  for(i=0;i<=NR_INTERVALS;i++) { 
    printf("\n\n\n\n") ;
    /****** temporary checking *****/
    /*    do {
    /*	  done = process_command() ;
    /*	  } while(!done) ; */
    /*******************************/
    if(para->tload_mode == 1) {
      printf("For Traffic=%3.1f\n", pct_load_s[i]) ;
      para->sinterval = (int)(100.0*interval/(double)(pct_load_s[i])) ;
    }
    else {
      printf("For Traffic=%3.1f%\n", (double)pct_load_l[i]) ;
      para->sinterval = (int)(100.0*interval/(double)(pct_load_l[i])) ;
    }
    do_update() ;
    do_reset() ;
    do_show_vars() ;
    nr_cycles = CYCLES_PER_TEST/10 ;
    do_run(nr_cycles) ;
    cur_stats->interval_time = cur_time ;
    latency_total[i] = avg_total_latency(cur_stats) ;
    latency_interval[i] = avg_interval_latency(cur_stats) ;
    reset_interval_var() ;
    for(j=1;j<4*NR_TIMES;j++) {
      do_run(nr_cycles) ;
      cur_stats->interval_time = cur_time ;
      latency_total[i*NR_INTERVALS+j] = avg_total_latency(cur_stats) ;
      latency_interval[i*NR_INTERVALS+j] = avg_interval_latency(cur_stats) ;
      reset_interval_var() ;
      if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
    }
    printf("     %d,%6.2f\n", 1000, latency_interval[0]) ;
    sum_latency = 0.0 ;
    for(j=1;j<4*NR_TIMES;j++) {
      printf("     %d,%6.2f\n", (j+1)*1000,
	     latency_interval[i*NR_INTERVALS+j]) ;
      sum_latency = sum_latency + latency_interval[i*NR_INTERVALS+j] ;
      if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
    }
    if(para->tload_mode == 1) {
      printf("\nTraffic = %3.1f\n", pct_load_s[i]) ;
    }
    else {
      printf("\nTraffic = %3.1f%\n", (double)pct_load_l[i]) ;
    }
    avg_latency = sum_latency/(double)(j-1) ;
    print_stats(cur_stats) ;
    printf("\n  Average interval latency = %6.3g\n", avg_latency) ;  
    if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
  }
  printf("\n") ;
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
void do_experiment(int exper_nr, int traffic_type)
{
  int i, nr_fail, nr_channels, known, experiment_nr, algorithm_type, done ;

  printf("\nEXPERIMENT %d",exper_nr) ;
  if(traffic_type == RANDOM)  printf("  TRAFFIC TYPE: RANDOM") ;
  else if(traffic_type==BIT_REVERSAL)  printf("  TRAFFIC TYPE: BIT_REVERSAL") ;
  else if(traffic_type==REFLECTION)  printf("  TRAFFIC TYPE: REFLECTION") ;
  else if(traffic_type == ROW_LINE)  printf("  TRAFFIC TYPE: ROW_LINE") ;
  else { printf("  TRAFFIC TYPE: undefined") ; return ;}

  if(exper_nr<1000) {
    algorithm_type = DETERMINISTIC;
    experiment_nr = exper_nr ;
  }
  else if(exper_nr<2000) {
    algorithm_type = STATIC ;
    experiment_nr = exper_nr ;
  }
  else if(exper_nr <3000) {
    experiment_nr = exper_nr - 1000 ;
    algorithm_type = STATIC_DBP ;
  }
  else if(exper_nr <4000) {
    experiment_nr = exper_nr - 2000 ;
    algorithm_type = DYNAMIC ;
  }
  else if(exper_nr <5000) {
    experiment_nr = exper_nr - 3000 ;
    algorithm_type = HYBRID ;
  }
  if(algorithm_type==DETERMINISTIC) printf("  ALGORITHM TYPE: DETERMINISTIC") ;
  else if(algorithm_type==STATIC)  printf("  ALGORITHM TYPE: STATIC") ;
  else if(algorithm_type==STATIC_DBP) printf("  ALGORITHM TYPE: STATIC_DBP\n");
  else if(algorithm_type==DYNAMIC)  printf("  ALGORITHM TYPE: DYNAMIC\n") ;
  else if(algorithm_type==HYBRID)  printf("  ALGORITHM TYPE: HYBRID\n") ;
  else {printf("undefined algorithm number\n") ; return ;}

  if(exper_nr <5000) {
    known = select_experiment(experiment_nr, algorithm_type) ;
    if(known) {
      if(traffic_type == RANDOM) {
	para->source_type = RANDOM ;  /* destination: Random */
      }
      else if(traffic_type == BIT_REVERSAL) {
	para->source_type = BIT_REVERSAL ;  /* destination: Bit Reversal */
	init_source_destinations(sources) ;
      }
      else if(traffic_type == REFLECTION) {
	para->source_type = REFLECTION ;  /* destination: Reflection */
	init_source_destinations(sources) ;
      }
      else if(traffic_type == ROW_LINE) {
	para->source_type = ROW_LINE ;  /* destination: Row Line */
	init_source_destinations(sources) ;
      }
      else {error("undefined traffic type. It must be 1~4.\n"); return;}
      latency_test_primitive() ;
    }
    else return ;
  }
  else if(exper_nr <6000) { /* Fault Tolerence test */
    nr_channels = (para->nr_nodes)*4 - (para->nr_ary)*(para->nr_dim)*2 ;
    printf("The number of channels = %d\n", nr_channels) ;
    for(i=0; i<10; i++) {
      nr_fail = nr_channels * pct_load_s[i] /100 ;
      printf("\n\nChannel failure rate= %d   ", pct_load_s[i]) ;
      printf("(The number of failed channels = %d)\n", nr_fail) ;
      faulty_test(nr_fail) ;
    }
  }
  else {error("undefined experiment number\n") ; exit(8) ;}
}
/*---------------------------------------------------------------------------*/
void faulty_test(int nr_fail)
{
  int i ;
  for(i=0; i<nr_fail; i++) {
    channel_disconnect_function() ;
  }
  print_disconnected_channels() ;
  recover_channel_fault() ;
  interval = (double)(para->nr_ary)*(para->slength))/4.0 ;
  para->sinterval = (int)(2.0*interval) ; /*--- traffic 50 % ---*/
    do_update() ;
    do_reset() ;
    do_show_vars() ;
    nr_cycles = CYCLES_PER_TEST/10 ;
    do_run(nr_cycles) ;
    cur_stats->interval_time = cur_time ;
    latency_total[i] = avg_total_latency(cur_stats) ;
    latency_interval[i] = avg_interval_latency(cur_stats) ;
    reset_interval_var() ;
    for(j=1;j<4*NR_TIMES;j++) {
      do_run(nr_cycles) ;
      cur_stats->interval_time = cur_time ;
      latency_total[i*NR_INTERVALS+j] = avg_total_latency(cur_stats) ;
      latency_interval[i*NR_INTERVALS+j] = avg_interval_latency(cur_stats) ;
      reset_interval_var() ;
      if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
    }
    printf("     %d,%6.2f\n", 1000, latency_interval[0]) ;
    sum_latency = 0.0 ;
    for(j=1;j<4*NR_TIMES;j++) {
      printf("     %d,%6.2f\n", (j+1)*1000,
	     latency_interval[i*NR_INTERVALS+j]) ;
      sum_latency = sum_latency + latency_interval[i*NR_INTERVALS+j] ;
      if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
    }
    if(para->tload_mode == 1) {
      printf("\nTraffic = %3.1f\n", pct_load_s[i]) ;
    }
    else {
      printf("\nTraffic = %3.1f%\n", (double)pct_load_l[i]) ;
    }
    avg_latency = sum_latency/(double)(j-1) ;
    print_stats(cur_stats) ;
    printf("\n  Average interval latency = %6.3g\n", avg_latency) ;  
    if( latency_interval[i*NR_INTERVALS+j] >600.0) break;
  }
  printf("\n") ;
  fflush(stdout) ;

}
/*---------------------------------------------------------------------------*/
void print_disconnected_channels()
{
  int i, j ;
  int src_node_nr, dst_node_nr ;

  printf("Disconnected channels:\n") ;
  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      if((&nodes[i])->outputs[j]->state == CHAN_FAIL) {
	src_node_nr = (&nodes[i])->id_seq ;
	dst_node_nr = (&nodes[i])->outputs[j]->dst->id_seq ;
	printf("%d-%d ", src_node_nr, dst_node_nr) ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void channel_disconnect_function()
{
  int node_nr, channel_nr, OK ;

  /* Pick up a node randomly among nodes */
  node_nr = random()%para->nr_nodes ;
  /* Pick up a channel  randomly from output channels of a node */
  channel_nr = random()%para->nr_chan ;
  OK = check_can_disconnect(node_nr, channel_nr) ;
  if(OK) {
    (&nodes[node_nr])->outputs[channel_nr]->state = CHAN_FAIL ;
    return ;
  }
  else channel_disconnect_function() ;
}
/*---------------------------------------------------------------------------*/
int check_can_disconnect(int node_nr, int channel_nr)
{
  int i, count, input_node_nr, chan_state ;

  count = 0 ;
  /* check as to output channels */
  for(i=0; i<para->nr_chan; i++) {
    chan_state = (&nodes[node_nr])->outputs[i]->state ;
    if(chan_state == DISCONNECTION || chan_state == CHAN_FAIL) {
      count++ ;
    }
  }
  if(count >= 2) return(NULL) ;
  /* chack as to input channels */
  count = 0 ;
  input_node_nr = (&nodes[node_nr])->outputs[channel_nr]->dst->id_seq ;
  for(i=0; i<para->nr_chan; i++) {
    chan_state = (&nodes[input_node_nr])->inputs[i].state ;
    if(chan_state == DISCONNECTION || chan_state == CHAN_FAIL) {
      count++ ;
    }
  }
  if(count >= 2) return(NULL) ;
  else return(TRUE) ;
}
/*---------------------------------------------------------------------------*/
void recover_channel_fault() 
{
  int i, j ;
  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      if((&nodes[i])->outputs[j]->state == CHAN_FAIL) {
	(&nodes[i])->outputs[j]->state = NULL ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void print_channel_states(int n) 
{
  int i, j ;
  int state, nr_ocupied_lanes, max_DR, nr_chan, nr_lanes, src_id, dst_id ;
  CHANNEL *channel ;
  LANE *lane ;

  if(n > para->nr_nodes-1) {
    printf("Argument Too large\n") ;
    return ;
  }
  nr_chan = para->nr_chan ;
  for(i=0; i<nr_chan; i++) {
    channel = (&nodes[n])->outputs[i] ;
    max_DR = channel->max_DR ;
    nr_lanes =  para->nr_lanes ;
    nr_ocupied_lanes = channel->nr_ocpied_lanes ;
    state = channel->state ;
    src_id = n ;
    dst_id = channel->dst->id_seq ;
    if(state == DISCONNECTION || state == CHAN_FAIL) {
      printf("channel:%d-%d DISCONNECTED\n", src_id, dst_id) ;
    }
    else {
      printf("channel:%d-%d [max_DR=%d, nr_occupied_lanes=%d]\n", src_id, 
	     dst_id, max_DR, nr_ocupied_lanes) ;
    }
    if(nr_ocupied_lanes != NULL) {
      for(j=0; j<nr_lanes; j++) {
	lane = &(channel->lanes[j]) ;
	if(lane->state == OCCUPIED) {
	  printf("   lane:%d category:", j) ;
	  if(para_c->lane_types[j].category == FIXED) {
	    printf("FIXED   class:%d", para_c->lane_types[j].class) ;
	  }
	  else if(para_c->lane_types[j].category == DYNAMIC) {
	    printf("DYNAMIC class:*") ;
	  }
	  else if(para_c->lane_types[j].category == FREED) {
	    printf("FREE    class:#") ;
	  }
	  else if(para_c->lane_types[j].category == SPECIAL) {
	    printf("SPECIAL  class:&") ;
	  }
	  printf(" message(src_%d)(t%d):(DR=%d)\n",
		 lane->message->src_node->id_seq,
		 lane->message->start_time, lane->DR) ; 
	}	
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void print_source_dest(int n)
{
  int i, A ;

  if(n > para->nr_nodes-1) {
    printf("Argument Too large\n") ;
    return ;
  }
  printf("Source_%d[",n) ;
  for(i=0; i<para->nr_dim; i++) {
    A = (&sources[n])->node->id_dimensions[i].value ;
    printf("%d",A) ;
    if(i != para->nr_dim-1) printf(",") ;
  }
  printf("]") ;
  printf(" dest_node:%d[", (&sources[n])->dest_node->id_seq) ;
  for(i=0; i<para->nr_dim; i++) {
    A = (&sources[n])->dest_node->id_dimensions[i].value ;
    printf("%d",A) ;
    if(i != para->nr_dim-1) printf(",") ;
  }
  printf("]\n") ;
}
/*---------------------------------------------------------------------------*/
void print_message_A() {
  MESSAGE *message, *messages ;
  LANE *lane ;
  messages = active_messages ;
  while(messages) {
    message = messages ;
    messages = message->next ;
    lane = message->head_lane ;
    printf("Message:src_%d(t%d) lane: ", message->src_node->id_seq,
	   message->start_time) ;
    while(lane) {
      printf("%d-%d(%d(%d)) flits=%d ", lane->channel->src->id_seq,
	     lane->channel->dst->id_seq,
	     lane->lane_id_nr, lane->DR, lane->nr_flits) ; 
      lane = lane->prev ;
    }
    if(message->face == SCRATCHED) printf("SCRACHED") ;
    printf("Dest_node:%d\n", message->dest_node->id_seq) ;
    printf("\n") ;
  }
}   
/*---------------------------------------------------------------------------*/
void print_message_B() {
  MESSAGE *message, *messages ;

  printf("Active messages: ") ;
  messages = active_messages ;
  while(messages) {
    message = messages ;
    messages = message->next ;
    printf("%d(%d) ", message->src_node->id_seq, message->start_time );
  }
  printf("\n") ;
}
/*---------------------------------------------------------------------------*/
void print_one_message(int src_nr, int generated_time) {
  MESSAGE *message, *messages ;
  LANE *lane ;
  messages = active_messages ;
  while(messages) {
    message = messages ;
    messages = message->next ;
    if(message->src_node->id_seq == src_nr &&
       message->start_time == generated_time) {
      lane = message->head_lane ;
      printf("Message:src_%d(t%d) M_DR:%d lane: ", message->src_node->id_seq,
	     message->start_time,
	     message->M_DR) ;
      while(lane) {
	printf("%d-%d(%d(%d)) flits=%d(%d) ", lane->channel->src->id_seq,
	       lane->channel->dst->id_seq,
	       lane->lane_id_nr, lane->DR, lane->nr_flits, lane->first) ; 
	lane = lane->prev ;
      }
      if(message->face == SCRATCHED) printf("SCRATCHED") ;
      printf("Dest_node:%d\n", message->dest_node->id_seq) ;
      printf("\n") ;
    }
  }   
}
/*---------------------------------------------------------------------------*/
