/*---------------------------------------------------------------------------*/
/*
 |    network simulator commands -- cmds.c
 |    Hiromichi Aoki July 27 1990
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
/*---------------------------------------------------------------------------*/
/* Function declaration */
/*---------------------------------------------------------------------------*/
void fault_tolerence_latency_test() ;
void fault_tolerence_throughput_test() ;
void print_disconnected_channels() ;
void disconnect_channels(int nr_fail) ;
void clip_up_concave_regions() ;
void recover_channel_and_node_fault() ;
void disconnect_one_channel() ;
void clear_all_active_messages() ; 
void check_three_channels_failed() ;
void make_one_node_dead(NODE *node) ;
void check_two_adjacent_channels_failed() ;
void check_one_channel_failed() ;
void check_and_make_node_dead() ;
void make_normal_channels_alive() ;
void print_deadlock_on_message_fault() ;
void init_all_channel_and_lane_state() ;
void init_channel_and_lane_state() ;
void init_lane_state() ;
void test_disconnect_func() ; 
void traffic_latency_test() ;
void saturation_throughput_test() ;
void throughput_test_primitive(int repeat) ;
int  latency_test_primitive(int load, int repeat) ;
/*---------------------------------------------------------------------------*/
int fault_chan_nr[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15} ;
int pct_load_2[] = {20,22,24,26,28,30} ;
int pct_load_3[] = {30,32,34,36,38,40} ;
int pct_load_4[] = {40,42,44,46,48,50} ;
int pct_load_5[] = {50,52,54,56,58,60} ;
int pct_load_6[] = {60,62,64,66,68,70} ;
int pct_load_7[] = {70,72,74,76,78,80} ;
int pct_load_8[] = {80,82,84,86,88,90} ;
int pct_load_9[] = {90,92,94,96,98,100} ;
int pct_load_l[] = {1,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100} ;
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
  if(!(para->experiment)) { 
    printf("Running for %d cycles\n", i) ; 
    if((i>0) && (i<MAX_CYCLES)) {
      for(j=0;j<i;j++) {
	cycle_network() ;
	print_activity(j) ; 
      }
    }
    printf("--Done\n") ;
  }
  else {
    /* printf("Running for %d cycles\n", i) ; */
    if((i>0) && (i<MAX_CYCLES)) {
      for(j=0;j<i;j++) {
	cycle_network() ;
	print_activity(j) ; 
      }
    }
    /* printf("--Done\n") ;*/
  }
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
  print_stats(cur_stats) ;
}
/*---------------------------------------------------------------------------*/
void reset_interval_var()
{
  cur_stats->interval_time = 0 ;
  cur_stats->nr_messages_received_interval = NULL ;
  cur_stats->interval_latency = NULL ;
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
void traffic_latency_test()
{
  static int i, load, repeat ;
  int over, done ;

  do_show_vars() ;
  printf("TRAFFIC LATENCY TEST\n") ;
  repeat = 35 ; /* excecute 35000 cycles */
  for(i=0;i<para->load_repetition;i++) { 
    load = select_traffic_load(i, para->tload_mode) ;
    over = latency_test_primitive(load, repeat) ;
    /****** temporary checking *********/
    /*  do {                           */
    /*    done = process_command() ;   */
    /*	} while(!done) ;             */
    /***********************************/
    if(over) break ;
    printf("\n\n\n\n") ;
  }
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
void saturation_throughput_test()
{
  int i, j, k, load, nr_channels, nr_fail, done ;
  static int repeat ;
  do_show_vars() ;
  printf("THROUPUT SATURATION TEST\n") ;
  repeat = 10 ; /* execute 10000 cycles */
  throughput_test_primitive(repeat) ;
  /****** temporary checking *********/
  /* do {                              
  /* done = process_command() ;      
  /* } while(!done) ;                
  /***********************************/
  do_drain_messages() ;
}
/*---------------------------------------------------------------------------*/
int select_traffic_load(int i, int load_sel) 
{
  if(load_sel < 10) {
    switch(para->tload_mode) {
  
    case 2:
      para->load_repetition = 6 ; 
      return(pct_load_2[i]) ;
    case 3:
      para->load_repetition = 6 ; 
      return(pct_load_3[i]) ;
    case 4:
      para->load_repetition = 6 ; 
      return(pct_load_4[i]) ;
    case 5:
      para->load_repetition = 6 ; 
      return(pct_load_5[i]) ;
    case 6:
      para->load_repetition = 6 ; 
      return(pct_load_6[i]) ;
    case 7:
      para->load_repetition = 6 ; 
      return(pct_load_7[i]) ;
    case 8:
      para->load_repetition = 6 ; 
      return(pct_load_8[i]) ;
    case 9:
      para->load_repetition = 6 ; 
      return(pct_load_9[i]) ;
    default:
      para->load_repetition = 20 ; 
      return(pct_load_l[i]) ;
    }  
  }
  else {
    para->load_repetition = 1 ;
    return(load_sel) ;
  }
}
/*---------------------------------------------------------------------------*/
int latency_test_primitive(int load, int repeat)
{
  int j ;
  int over = 0 ;
  double latency_total[repeat+1] ;
  double latency_interval[repeat+1] ;
  double sum_latency ;
  double avg_latency ;
  double time_interval ;
  double interval ;
  interval = (double)((para->nr_ary)*(para->slength))/4.0 ;
  printf("For Traffic=%3.1f\n", (double)(load)) ;
  para->sinterval = (int)(100.0*interval/(double)(load)) ;
  do_update() ;
  do_reset() ;
  nr_cycles = CYCLES_PER_TEST/10 ;
  do_run(nr_cycles) ;
  cur_stats->interval_time = cur_time ;
  latency_total[0] = avg_total_latency(cur_stats) ;
  latency_interval[0] = avg_interval_latency(cur_stats) ;
  reset_interval_var() ;
  for(j=1; j<repeat; j++) {
    do_run(nr_cycles) ;
    cur_stats->interval_time = cur_time ;
    latency_total[j] = avg_total_latency(cur_stats) ;
    latency_interval[j] = avg_interval_latency(cur_stats) ;
    reset_interval_var() ;
    /*if(latency_interval[j] >600.0) {over = TRUE ; break ;}*/
  }
  printf("     %d,%6.2f\n", 1000, latency_interval[0]) ;
  sum_latency = 0.0 ;
  for(j=1; j<repeat; j++) {
    printf("     %d,%6.2f\n", (j+1)*1000,
	   latency_interval[j]) ;
    sum_latency = sum_latency + latency_interval[j] ;
    /*if(latency_interval[j] >600.0) break;*/
  }
  printf("\nTraffic = %3.1f%\n", (double)(load)) ;
  avg_latency = sum_latency/(double)(j-1) ;
  print_stats(cur_stats) ;
  printf("\n  Average interval latency = %6.3g\n", avg_latency) ;  
  return(over) ;
}
/*---------------------------------------------------------------------------*/
void throughput_test_primitive(int repeat)
{
  double thruput ;
  int    delivered ;
  
  para->ssat = 1 ;
  nr_cycles = repeat * 1000 ;
  do_run_test_cycle(nr_cycles) ;
  thruput = flits_per_source_cycle(cur_stats) ;
  delivered = cur_stats->nr_messages_received ;
  printf("THROUGHPUT    MESSAGES\n") ;
  printf("%10.4g  %8d\n", thruput, delivered) ;
  printf("\n") ;
  fflush(stdout) ;
}
/*---------------------------------------------------------------------------*/
void do_experiment(int exper_nr, int traffic_type, int test_catgry, int load_sel)
{
  int i, nr_fail, nr_channels, known, experiment_nr, algorithm_type, done ;

  printf("\nEXPERIMENT %d",exper_nr) ;
  if(traffic_type == RANDOM)  printf("  TRAFFIC TYPE: RANDOM") ;
  else if(traffic_type==BIT_REVERSAL)  printf("  TRAFFIC TYPE: BIT_REVERSAL") ;
  else if(traffic_type==REFLECTION)  printf("  TRAFFIC TYPE: REFLECTION") ;
  else if(traffic_type == ROW_LINE)  printf("  TRAFFIC TYPE: ROW_LINE") ;
  else { printf("  TRAFFIC TYPE: undefined") ; return ;}
  para->experiment = TRUE ;

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

  known = select_experiment(experiment_nr, algorithm_type, test_catgry) ;
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
  }
  else {error("unmatched experiment\n") ; return ;}
  para->tload_mode = load_sel ;
  if(test_catgry == 0 || test_catgry == 1) {
    traffic_latency_test() ;
    return ;
  }
  else if(test_catgry == 2) {
    saturation_throughput_test() ;
    return ;
  }
  else if(test_catgry == 3) {
    fault_tolerence_latency_test() ;
    return ;
  }
  else if(test_catgry == 4) {
    fault_tolerence_throughput_test() ;
    return ;
  }
}
/*---------------------------------------------------------------------------*/
void fault_tolerence_latency_test()
{
  int i, j, k, nr_channels, done ;
  int over = 0 ;
  static int load, repeat, nr_fail ;

  do_show_vars() ;
  nr_channels = (para->nr_nodes)*2 - (para->nr_ary)*(para->nr_dim) ;
  repeat = 25 ; /* execute 25000 cycles */
  for(i=8; i<9; i++) { /* traffic load 50 % */
    load = pct_load_l[i] ;
    for(j=4; j<15; j++) { /* disconnection percentage */
      for(k=0; k<20; k++) {
	nr_fail = nr_channels * fault_chan_nr[j]/100 ;
	printf("\n\n\n\n") ;
	printf("LATENCY TEST ON CHANNEL FAILURES\n") ;
	printf("Traffic Load = %2.1f,  ", (double)load) ;
	printf("Channel Failures = %d", nr_fail) ;
	printf("(Rate= %3.2f),  ",100.0*(double)nr_fail/(double)(nr_channels));
	printf("Repetition = %d\n", k+1) ;
	printf("The number of channels: %d --> %d\n", nr_channels, nr_channels
	       - nr_fail) ;
	disconnect_channels(nr_fail) ; 
	/* test_disconnect_func() ; */ 
	print_disconnected_channels() ;
	clip_up_concave_regions() ;
	printf("Clip up concave regions ---Done\n") ;
	print_disconnected_channels() ;
	over = latency_test_primitive(load, repeat) ;
	/****** temporary checking *********/
        /*do {                            
	/* done = process_command() ;    
	/*} while(!done) ;                
	/***********************************/
	/*if(check_deadlock_on_channel_fault()) {
	  clear_all_active_messages() ;
	  print_deadlock_on_message_fault() ;
	  init_all_channel_and_lane_state() ;
	}*/
	do_drain_messages() ;
	recover_channel_and_node_fault() ;
	if(over) break ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void fault_tolerence_throughput_test()
{
  int i, j, k, load, repeat, nr_channels, nr_fail, done ;
  do_show_vars() ;
  nr_channels = (para->nr_nodes)*2 - (para->nr_ary)*(para->nr_dim) ;
  repeat = 10 ; /* execute 10000 cycles */
  for(j=0; j<15; j++) {
    for(k=0; k<20; k++) {
      nr_fail = nr_channels * fault_chan_nr[j]/100 ;
      printf("\n\n\n\n") ;
      printf("THROUPUT SATURATION TEST ON CHANNEL FAILURES\n") ;
      printf("Channel Failures = %d", nr_fail) ;
      printf("(Rate= %3.2f),  ",100.0*(double)nr_fail/(double)(nr_channels));
      printf("Repetition = %d\n", k+1) ;
      printf("The number of channels: %d --> %d\n", nr_channels, nr_channels
	     - nr_fail) ;
      disconnect_channels(nr_fail) ; 
      /* test_disconnect_func() ; */ 
      print_disconnected_channels() ;
      clip_up_concave_regions() ;
      printf("Clip up concave regions---Done\n") ;
      print_disconnected_channels() ;
      throughput_test_primitive(repeat) ;
      /****** temporary checking *********/
      /* do {                              
      /* done = process_command() ;      
      /* } while(!done) ;                
      /***********************************/
      do_drain_messages() ;
      recover_channel_and_node_fault() ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void test_disconnect_func() 
{
  (&nodes[59])->outputs[1]->state = FAILED ;
  (&nodes[59])->outputs[2]->state = FAILED ;
  (&nodes[60])->outputs[2]->state = FAILED ;
  (&nodes[61])->outputs[0]->state = FAILED ;
  (&nodes[61])->outputs[2]->state = FAILED ;
}
/*---------------------------------------------------------------------------*/
void disconnect_channels(int nr_fail)
{
  int i ;
  for(i=0; i<nr_fail; i++) {
    disconnect_one_channel() ;
  }
}
/*---------------------------------------------------------------------------*/
void disconnect_one_channel()
{
  int node_nr, channel_nr, state ;

  /* Pick up a node randomly among nodes */
  node_nr = random()%para->nr_nodes ;
  /* Pick up a channel  randomly from output channels of a node */
  channel_nr = random()%para->nr_chan ;
  state = (&nodes[node_nr])->outputs[channel_nr]->state ;
  if(state == NORMAL || state == UNUSABLE) {
    (&nodes[node_nr])->outputs[channel_nr]->state = FAILED ;
    (&nodes[node_nr])->inputs[channel_nr].state = FAILED ;
  }
  else disconnect_one_channel() ;
  return ;
}
/*---------------------------------------------------------------------------*/
void clip_up_concave_regions()
{
  para->changed = TRUE ;
  while(para->changed) {
    para->changed = NULL ;
    check_three_channels_failed() ;
    check_two_adjacent_channels_failed() ;
    check_one_channel_failed() ;
    check_and_make_node_dead() ;
    check_three_channels_failed() ;
  }
  make_normal_channels_alive() ;
}
/*---------------------------------------------------------------------------*/
void check_three_channels_failed() 
{
  int i, j, state ;
  int fail_count = 0 ;
  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      state = (&nodes[i])->outputs[j]->state ;
      if(state == DISCON || state == FAILED) fail_count++ ;
    }
    if(fail_count >=3 && (&nodes[i])->state == ALIVE) {
      para->changed = TRUE ;
      make_one_node_dead(&nodes[i]) ;
    }
    fail_count = 0 ;
  }
}
/*---------------------------------------------------------------------------*/
void make_one_node_dead(NODE *node)
{
  int i, state ;
  node->state = DEAD ;
  for(i=0; i<para->nr_chan; i++) {
    state = node->outputs[i]->state ;
    if(state == NORMAL || state == UNUSABLE) {
      node->outputs[i]->state = FAILED ;
    }
    state = node->inputs[i].state ;
    if(state == NORMAL || state == UNUSABLE) {
      node->inputs[i].state = FAILED ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void check_two_adjacent_channels_failed() 
{
  int i, j, k, m, state ;
  int first_state, second_state  ;
  int fail_count = 0 ;

  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      state = (&nodes[i])->outputs[j]->state ;
      if(state == DISCON || state == FAILED) {
	fail_count++ ;
      }
    }
    if(fail_count == 2) {
      for(j=0; j<para->nr_chan; j++) {
	first_state = (&nodes[i])->outputs[j%para->nr_chan]->state ;
	if(j==0) m=2 ;
	else if(j==1) m=3 ;
	else if(j==2) m=1 ;
	else m=0 ;
	second_state = (&nodes[i])->outputs[m]->state ;
	if((first_state == DISCON || first_state == FAILED) &&
	   (second_state == DISCON || second_state == FAILED)) {
	  for(k=0; k<para->nr_chan; k++) {
	    if((&nodes[i])->outputs[k]->state == NORMAL) {
	      (&nodes[i])->outputs[k]->state = UNUSABLE ;
	    }
	  }
	}
      }
    }
    fail_count = 0 ;
  }
}
/*---------------------------------------------------------------------------*/
void check_one_channel_failed() 
{
  int i, j, k, state ;
  int fail_count = 0 ;

  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      state = (&nodes[i])->outputs[j]->state ;
      if(state == DISCON || state == FAILED) {
	fail_count++ ;
      }
    }
    if(fail_count == 1) {
      for(j=0; j<para->nr_chan; j++) {
	if((&nodes[i])->inputs[j].state == UNUSABLE) {
	  if(j==0) k=1 ;
	  else if(j==1) k=0 ;
	  else if(j==2) k=3 ;
	  else k=2 ;

	  if((&nodes[i])->outputs[k]->state == NORMAL) {
	    (&nodes[i])->outputs[k]->state = UNUSABLE ;
	  }
	}
      } 
    }
    fail_count = 0 ;
  }
}
/*---------------------------------------------------------------------------*/
void check_and_make_node_dead() 
{
  int i, j, k, m, state ;
  int first_state, second_state ;
  int fail_count = 0 ;
  int make_dead = 0 ;

  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      state = (&nodes[i])->outputs[j]->state ;
      if(state == FAILED) {
	fail_count++ ;
      }
    }
    if(fail_count == 2) {
      for(j=0; j<para->nr_chan; j++) {
	first_state = (&nodes[i])->outputs[j%para->nr_chan]->state ;
	if(j==0) m=2 ;
	else if(j==1) m=3 ;
	else if(j==2) m=1 ;
	else m=0 ;
	second_state = (&nodes[i])->outputs[m]->state ;
	if(first_state == FAILED && second_state == FAILED) {
	  for(k=0; k<para->nr_chan; k++) {
	    if((&nodes[i])->inputs[k].state == UNUSABLE) {
	      make_dead = 1 ;
	    }
	  }
	}
      }
    }
    if(make_dead && (&nodes[i])->state == ALIVE) {
      para->changed = TRUE ;
      make_one_node_dead((&nodes[i])) ;
    }
    make_dead = fail_count = 0 ;
  }
}
/*---------------------------------------------------------------------------*/
void make_normal_channels_alive() 
{
  int i, j ;
  CHANNEL *channel ;
  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      channel = (&nodes[i])->outputs[j] ;
      if(channel->state == UNUSABLE && channel->src->state == ALIVE &&
	 channel->dst->state == ALIVE) {
	channel->state = NORMAL ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void print_disconnected_channels()
{
  int i, j ;
  int src_node_nr, dst_node_nr, state ;
  int fail_count = 0 ;

  printf("Disconnected Channels: ") ;
  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      state = (&nodes[i])->outputs[j]->state ;
      if(state == FAILED) {
	src_node_nr = (&nodes[i])->id_seq ;
	dst_node_nr = (&nodes[i])->outputs[j]->dst->id_seq ;
	printf("%d-%d ", src_node_nr, dst_node_nr) ;
      }
    }
  }
  printf("\nDead nodes: ") ;
  for(i=0; i<para->nr_nodes; i++) {
    if((&nodes[i])->state == DEAD) {
      printf("%d  ", (&nodes[i])->id_seq ) ;
    }
  }
  for(i=0; i<para->nr_nodes; i++) {
    for(j=0; j<para->nr_chan; j++) {
      if((&nodes[i])->outputs[j]->state == FAILED)
	fail_count++ ;
    }
  }
  printf("\nChannel Failures=%d\n", fail_count/2) ;
  printf("\n") ;
}
/*---------------------------------------------------------------------------*/
int check_can_disconnect(int node_nr, int channel_nr)
{
  int i, count, input_node_nr, chan_state ;

  count = 0 ;
  /* check as to output channels */
  for(i=0; i<para->nr_chan; i++) {
    chan_state = (&nodes[node_nr])->outputs[i]->state ;
    if(chan_state == DISCON || chan_state == FAILED) {
      count++ ;
    }
  }
  if(count >= 2) return(NULL) ;
  /* chack as to input channels */
  count = 0 ;
  input_node_nr = (&nodes[node_nr])->outputs[channel_nr]->dst->id_seq ;
  for(i=0; i<para->nr_chan; i++) {
    chan_state = (&nodes[input_node_nr])->inputs[i].state ;
    if(chan_state == DISCON || chan_state == FAILED) {
      count++ ;
    }
  }
  if(count >= 2) return(NULL) ;
  else return(TRUE) ;
}
/*---------------------------------------------------------------------------*/
void recover_channel_and_node_fault() 
{
  int i, j ;
  for(i=0; i<para->nr_nodes; i++) {
    (&nodes[i])->state = ALIVE ;
    for(j=0; j<para->nr_chan; j++) {
      if((&nodes[i])->outputs[j]->state == FAILED) {
	(&nodes[i])->outputs[j]->state = NORMAL ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
int remove_infinite_looping_messages()
{
  MESSAGE *messages, *message ;
  CHANNEL *channel ;

  messages = active_messages ;
  while(messages) {
    message = messages ;
    messages = message->next ;
    if(message->M_DR >200) {
      eliminate_the_message(message) ;
      cur_stats->nr_removed_livelocked_messages++ ;
    }
  }
  return(NULL) ;
}
/*---------------------------------------------------------------------------*/
void clear_all_active_messages() 
{
  MESSAGE *messages, *message ;
  
  messages = active_messages ;
  active_messages = NULL_MESSAGE ;
  while(messages) {
    message = messages ;
    messages = message->next ;
    free_message(message) ;
  }
}
/*---------------------------------------------------------------------------*/
void init_all_channel_and_lane_state()
{
  int i, j ;
  CHANNEL *channel ;

  for(i=0 ;i<para->nr_nodes ;i++) {
    for(j=0; j<para->nr_chan; j++) {
      channel = (&nodes[i])->outputs[j] ;
      init_channel_and_lane_state(channel) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void init_channel_and_lane_state(CHANNEL *channel) 
{
  int i ;

  channel->nr_ocpied_lanes = 0 ; 
  /* initialize lanes */
  for(i=0;i<MAX_LANES;i++) {
    init_lane_state(&(channel->lanes[i])) ; 
  }
}
/*---------------------------------------------------------------------------*/
void init_lane_state(LANE *lane) 
{
  lane->message = NULL_MESSAGE ;  /* message occupying this lane */
  lane->first = 0 ;               /* header flit of the message in this lane */
  lane->nr_flits = 0 ;            /* number of flits in this lane */
  lane->prev = NULL_LANE ;        /* previous lane */
  lane->state = IDLE ;            /* set initial lane state */
  lane->init = 0 ;
}
/*---------------------------------------------------------------------------*/
void print_deadlock_on_message_fault() 
{
  printf("######################################################\n") ;
  printf("# A scratched message encountered a channel fault.   #\n") ;
  printf("# The message was eliminated from network            #\n") ;
  printf("######################################################\n") ;
  printf(" The number of eliminated scratched messages = %d\n", 
	 cur_stats->nr_removed_deadlocked_messages) ;
}
/*---------------------------------------------------------------------------*/
void print_livelock_on_message_fault() 
{
  printf("######################################################\n") ;
  printf("# The livelocked message was eliminated from network #\n") ;
  printf("######################################################\n") ;
  printf(" The number of eliminated livelocked messages = %d\n", 
	 cur_stats->nr_removed_livelocked_messages) ;
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
    if(state == DISCON || state == FAILED) {
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
	    printf("DYNAMIC class:>=0") ;
	  }
	  else if(para_c->lane_types[j].category == DYNAMIC_UPPER) {
	    printf("DYNAMIC class:>=1") ;
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
