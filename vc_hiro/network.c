/*---------------------------------------------------------------------------*/
/*
 |    Network_sim.c - network simulator
 |     Hiromichi Aoki July 27, 1990
 |
 |    Network_sim simulates the performance of networks with arbitrary 
 |    toplogy of k-ary n-cube and adaptive or nonadaptive routing that 
 |    use virtual channel flow control.
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "netdef.h"
#include "kary_ncube.h"
#include "routing_func.h"
#include "event.h"
#include "stats.h"
#include "set.h"
#include "cmds.h"
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/* FUNCTIONS */
/*---------------------------------------------------------------------------*/
void init_net() ;
void init_paramater() ;
void cycle_network() ;
void selection_of_algorithms() ;
void print_menu() ;
void print_selecting_priorities() ;
void select_priorities() ;
void select_nr_virtual_channels() ;
void select_nr_special_lanes() ;
void select_VC_assignment_algorithm() ;
void set_special_lane() ;
void print_VC_assignment_algorithm() ;
void setting_VCs() ;
void print_VCs() ;
void select_timeout_scheme() ; 
void select_timeout_count() ;
int  slelect_priority_num() ;
int  select_routings() ;
/*---------------------------------------------------------------------------*/
/* GLOBALS */
/*---------------------------------------------------------------------------*/
NODE *nodes ;			/* array of nodes */
MESSAGE *free_messages ;	/* list of free message structures */
MESSAGE *active_messages ;	/* list of active message structures */
REQUEST *free_requests ;	/* list of free request structures */
CHANNEL *active_channels ;	/* list of channels with requests pending */
TIMESTEP *event_list ;		/* array of timesteps */
EVENT *free_events ;		/* free event list */
SOURCE *sources ;		/* array of sources */
PARA *para ;                    /* paramaters */
PARA_C *para_c ;                /* channel paramater */
SET_REC *set_table ;            /* set variable table */
INTARY **temp ;                 /* temporary array */
int cur_time ;			/* the current time */
int cur_index ;			/* the position of the current timestep in */
int nr_nodes ;			/* number of nodes */
int nr_sources ;                /* number of source nodes */
int debug = 0 ;			/* debugging bit mask */
int nr_cycles = 10000 ;		/* number of cycles to run a test */
int drain_messages = 0 ;	/* if true, sources disabled */
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
main(int argc, char *argv[])
{
  int done ;
  int k = 16, n = 2, exper = 0, traffic_type = 1, test_category = 0 ;
  int load_sel ;
  /* gobble up argument list */
  if(argc >= 3) {
    k = atoi(argv[1]) ; 
    n = atoi(argv[2]) ;
    if(argc >= 4) exper = atoi(argv[3]) ;
    if(argc >= 5) traffic_type = atoi(argv[4]) ;
    if(argc >= 6) test_category = atoi(argv[5]) ;
    if(argc >= 7) load_sel = atoi(argv[6]) ;
    printf("k=%darray n=%dcube   exper=%d traffic_type=%d\n", k,n,exper,
	   traffic_type );
    if(k==1) {
      printf("k=%d is prohibited.   select k >1\n", k) ; exit(1) ;
    }
  }
  /* initialize everything */
  init_net() ;	
  init_paramater(k,n) ; /* initialize paramater */
  if(!exper) {
    selection_of_algorithms() ; /* selection of algorithm */
  }
  new_kary_ncube(k,n) ;		/* make a k-array n-cube network */
  make_network_to_mesh_type() ; /* make network mesh */
  /* inject_test_message(0,8) ;	/* inject a test message */
  /* if batch experiment mode -- go do it */
  if(exper) {
    do_experiment(exper, traffic_type, test_category, load_sel) ;
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
  case 'D':
    scanf("%d",&i) ; 
    do_drain_messages_T(i) ;
    return(0) ;
  case 'e':
    scanf("%d %d",&i, &j) ;
    do_experiment(i, j) ;
    return(0) ;
  case 'h':
    do_show_vars() ; return(0) ;
  case 'i':
    do_reset() ; return(0) ;
  case 'p':
    print_stats(cur_stats) ; return(0) ;
  case 'm':
    scanf("%d %d", &i, &j) ;
    print_one_message(i, j) ; return(0) ;
  case 'L':
    print_VCs() ; return(0) ;
  case 'M':
    print_message_A() ; return(0) ;
  case 'r':
    scanf("%d",&i) ; do_run(i) ; return(0) ;
  case 's':
    scanf("%s %d",s,&i) ;
    do_set_var(s,i) ; return(0) ;
  case 'u':
    do_update() ; return(0) ;
  case 'N':
    scanf("%d", &i) ;
    print_channel_states(i) ;
    return(0) ;
  case 'S':
    scanf("%d", &i) ;
    print_source_dest(i) ;
    return(0) ;
  case '?':
    print_menu() ;
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
  request_next_lanes() ;	       /* active messages compete for lanes */
  allocate_next_lanes() ;              /* allocate lanes to messages */
  request_all_channels_of_messages() ; /* compete for physical channels */
  allocate_channels_for_flits() ;      /* channels allocate wires to lanes */
  advance_time() ;
  clean_messages() ;		/* log finished messages and return to free */
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
void init_paramater(int k, int n)
{
  int i ;
  para = NEW_STRUCT(PARA) ;      /* create paramater struct */
  para_c = NEW_STRUCT(PARA_C) ;  /* create paramater struct */
  create_set_table() ;           /* set up set table for variables */
  para->k_array = para->nr_ary = k ;
  para->n_cube = para->nr_dim = n ;
  para->nr_nodes = power(k,n) ;
  if(k == 2)  para->nr_chan = n ; 
  else        para->nr_chan = 2*n ; 
  init_nc_table() ;              /* set up next channel finding table */
  para->nr_lanes = 8 ;           /* default:   8 virtual channels */
  para->routing_scheme = 2 ;     /* default:   nonadaptive */
  para->chan_finding = 1 ;       /* default:   shortest path */
  para->depth = 2 ;              /* default:   each VC depth = 4 */
  para->slength = 20 ;           /* default:   message length = 20 */
  para->source_type = RANDOM ;   /* default:   destination = random */
  para->sinterval = 1000 ;       /* default:   traffic congestion = 1000 */
  para->ssat = NULL ;            /* default:   not saturation mode */
  para->chan_sched = CS_FIRST ;  /* default:   CS first */
  para->timeout_mode = 0 ;       /* default:   message can't wait */
  para->timeout_count = 0 ;      /* default:   0 */
  para->test_category = 0 ;      /* default:   traffic latency test */
  para->load_repetition = 1 ;    /* default:   one loop */
  para_c->nr_fixed_lanes = NULL ;/* default:   fixed VC not exist */
  para_c->nr_free_lanes = NULL ; /* default:   free VC not exist */
  para_c->nr_dynamic_lanes = NULL ; /* default:   dynamic VC not exist */
  para_c->throttling_boundary = NULL ; /* default:   no throttling */
  para_c->class_i_exist = NEW_ARRAY(INTARY, 20) ;
  para_c->lowest_VC_nr_class_i = NEW_ARRAY(INTARY, 20) ;
  para_c->highest_VC_nr_class_i = NEW_ARRAY(INTARY, 20) ;
  para_c->lane_types = NEW_ARRAY(PARA_LANE, MAX_LANES) ;
  for(i=0; i<MAX_LANES; i++) {
    para_c->class_i_exist[i].value = NULL ;
  }
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
int fact(int n)
{
  if(n == 1) return(1) ;
  else return(n*fact(n-1)) ;
}
/*---------------------------------------------------------------------------*/
/* 
 | selection_of_algorithms() determines the channel selection algorithm and 
 | the virtual channel assignment strategy.                                  
/*---------------------------------------------------------------------------*/
void selection_of_algorithms()
{
  int i, traffic, routing_scheme, chan_finding, VC_category ;
  int spriority_1, priority_2, priority_3, priority_4 ;

  printf("Select Traffic Scheme(Destination Decision Scheme)\n") ;
  printf("1.Random  2.Bit Reversal  3.Row Line  4.Reflection ?") ; 
  do {
    traffic = select_destination_scheme() ;
  } while(!traffic) ;
  printf("Adaptive(1) or Non-adaptive(2) ?") ;
  do {
    routing_scheme = select_routings() ;
  } while(!routing_scheme) ;
  if(routing_scheme == NONADAPTIVE) {
    printf("Select the number of virtual channels ?") ;
    select_nr_virtual_channels() ;
  }
  if(routing_scheme == ADAPTIVE) {
    printf("Select next channel finding algorithm for messages\n") ;
    printf("1.shortest paths only   2.any available channels ?") ;
    do {
      chan_finding = select_channel_finding_methods() ;  
    } while(!chan_finding) ;
    print_selecting_priorities() ;
    select_priorities() ;
    select_timeout_scheme() ;
    printf("Determine the number of virtual channels ?") ;
    select_nr_virtual_channels() ;
    printf("Determine the number of special lanes ?") ;
    select_nr_special_lanes() ;
    print_VC_assignment_algorithm() ;
    select_VC_assignment_algorithm() ;  
    setting_VCs() ; 
    print_VCs() ;
  }
  printf("OK..Algorithm selection completed. Input command(menu;'?').\n") ;
}
/*---------------------------------------------------------------------------*/
int select_nr_class_i_VCs(int i, int new_lane, int fixed_boundary) 
{
  int number, end=0 ;
  do {
    scanf("%d", &number) ;
    if(number > 0) end = (number+new_lane) > fixed_boundary ;
  } while(end) ;
  para_c->class_i_exist[i].value = TRUE ;
  para_c->lowest_VC_nr_class_i[i].value = new_lane ;
  para_c->highest_VC_nr_class_i[i].value = new_lane + number - 1 ;
  return(number) ;
}
/*---------------------------------------------------------------------------*/
void select_nr_virtual_channels() 
{
  int number ;
  do {
    scanf("%d", &number) ;
  } while(!number) ;
  para->nr_lanes = number ;
}
/*---------------------------------------------------------------------------*/
void select_nr_special_lanes() 
{
  int number ;
  do {
    scanf("%d", &number) ;
  } while(!number) ;
  para_c->nr_special_lanes = number ;
  para_c->special_min_lane_nr = para->nr_lanes - number ;
  para_c->special_max_lane_nr = para->nr_lanes -1 ;
}
/*---------------------------------------------------------------------------*/
void select_VC_assignment_algorithm()
{
  int i, VC_category, nr_special_lanes, new_lane, fixed_bdry, dynamic_bdry ;
  int throttling_bdry ;

  do {
    scanf("%d", &VC_category) ;
  } while(!VC_category) ;
  nr_special_lanes = para_c->nr_special_lanes ;
  if(VC_category == 1) {
    para_c->VC_category = 1 ;
    new_lane = 0 ;
    fixed_bdry = para->nr_lanes - nr_special_lanes;
    for(i=0; new_lane<fixed_bdry; i++) {
      printf("Determine the number of class(%d) VCs ?", i) ;
      new_lane = new_lane+select_nr_class_i_VCs(i, new_lane, fixed_bdry) ;
    }
    para_c->max_class = i-1 ;
    if(new_lane > 0) {
      para_c->nr_fixed_lanes = para->nr_lanes - nr_special_lanes ;
      para_c->fixed_min_lane_nr = 0 ;
      para_c->fixed_max_lane_nr = para->nr_lanes - nr_special_lanes -1 ;
    }
  }
  else if(VC_category == 2) {
    para_c->VC_category = 2 ;
    printf("Determine the number of fixed VCs ?") ;
    fixed_bdry = select_fixed_boundary() ;
    new_lane = 0 ;
    for(i=0; new_lane<fixed_bdry; i++) {
      printf("Determine the number of class(%d) VCs ?", i) ;
      new_lane = new_lane+select_nr_class_i_VCs(i, new_lane, fixed_bdry) ;
    }
    para_c->max_class = i-1 ;
    if(new_lane > NULL) {
      para_c->nr_fixed_lanes = fixed_bdry ;
      para_c->fixed_min_lane_nr = 0 ;
      para_c->fixed_max_lane_nr = fixed_bdry -1 ;
    }
    if(new_lane != para->nr_lanes - nr_special_lanes) {
      para_c->nr_free_lanes = para->nr_lanes - fixed_bdry ;
      para_c->free_min_lane_nr = fixed_bdry ;
      para_c->free_max_lane_nr = para->nr_lanes - nr_special_lanes -1 ;
    }
  }
  else if(VC_category == 3) {
    para_c->VC_category = 3 ;
    para_c->max_class = MAXINT-1 ;
    dynamic_bdry = para->nr_lanes - nr_special_lanes;
    para_c->nr_dynamic_lanes = para->nr_lanes - nr_special_lanes ;
    para_c->dynamic_min_lane_nr = 0 ;
    para_c->dynamic_max_lane_nr = para->nr_lanes - nr_special_lanes -1 ;
  }
  else if(VC_category == 4) {
    para_c->VC_category = 4 ;
    para_c->max_class = MAXINT-1 ;
    dynamic_bdry = para->nr_lanes - nr_special_lanes;
    para_c->nr_dynamic_lanes = para->nr_lanes - nr_special_lanes ;
    para_c->dynamic_min_lane_nr = 0 ;
    para_c->dynamic_max_lane_nr = para->nr_lanes - nr_special_lanes -1 ;
    printf("Select throttling boundary:") ;
    do {
      scanf("%d", &throttling_bdry) ;
    } while(!throttling_bdry && throttling_bdry > dynamic_bdry) ;
    para_c->throttling_boundary = throttling_bdry ;
  }
}
/*---------------------------------------------------------------------------*/
select_fixed_boundary()
{
  int number ;
  do {
    scanf("%d", &number) ;
  } while(!number) ;
  return(number) ;
}
/*---------------------------------------------------------------------------*/
int select_destination_scheme()
{
  char c ;
  c = getchar() ;
  switch(c) {
  case '1':
    para->source_type = RANDOM ;
    return(1) ;
  case '2':
    para->source_type = BIT_REVERSAL ;
    return(1) ;
  case '3':
    para->source_type = ROW_LINE ;
    return(1) ;
  case '4':
    para->source_type = REFLECTION ;
    return(1) ;
  default:
    return(0) ;
  }
}
/*---------------------------------------------------------------------------*/
void setting_VCs()
{
  int i, j, min, max, lowest, highest ;
  if(para_c->nr_fixed_lanes != NULL) {
    min = 0 ;
    max = para_c->fixed_max_lane_nr ;
    for(i=0; i<max+1; i++) {
      para_c->lane_types[i].category = FIXED ;
    }
    for(i=0; i<MAX_LANES; i++) {
      lowest = para_c->lowest_VC_nr_class_i[i].value ;
      highest = para_c->highest_VC_nr_class_i[i].value ;
      for(j=lowest; j<highest+1; j++) {
	if(para_c->class_i_exist[i].value) {
	  para_c->lane_types[j].class = i ;
	} 
      }
    }
  }
  if(para_c->nr_free_lanes != NULL) {
    min = para_c->free_min_lane_nr ;
    max = para_c->free_max_lane_nr ;
    for(i=min; i<max+1; i++) {
      para_c->lane_types[i].category = FREED ;
    }
  }
  if(para_c->nr_dynamic_lanes != NULL) {
    min = para_c->dynamic_min_lane_nr ;
    max = para_c->dynamic_max_lane_nr ;
    for(i=min; i<max+1; i++) {
      para_c->lane_types[i].category = DYNAMIC ;
    }
  }
  if(para_c->throttling_boundary) {
    min = para_c->throttling_boundary ;
    max = para_c->dynamic_max_lane_nr ;
    for(i=min; i<max+1; i++) {
      para_c->lane_types[i].category = DYNAMIC_UPPER ;
    }
  }
  min = para_c->special_min_lane_nr ;
  max = para_c->special_max_lane_nr ;
  for(i=min; i<max+1; i++) {
    para_c->lane_types[i].category = SPECIAL ;
  }
}
/*---------------------------------------------------------------------------*/
void print_VCs() 
{
  int i ;

  printf("\n") ;
  for(i=para->nr_lanes-1; i>-1; i--) {
    printf("    lane(%2d):   category:", i) ;
    if(para_c->lane_types[i].category == FIXED) {
      printf("FIXED     class:%d\n", para_c->lane_types[i].class) ;
    }
    else if(para_c->lane_types[i].category == FREED) {
      printf("FREE      class:#\n") ;
    }
    else if(para_c->lane_types[i].category == DYNAMIC) {
      printf("DYNAMIC   class:>=0\n") ;
    }
    else if(para_c->lane_types[i].category == DYNAMIC_UPPER) {
      printf("DYNAMIC   class:>=1\n") ;
    }
    else if(para_c->lane_types[i].category == SPECIAL) {
      printf("SPECIAL   class:$\n") ;
    }
  }
  printf("\n") ;
}
/*---------------------------------------------------------------------------*/
int select_routings()
{
  char c ;
  c = getchar() ;
  switch(c) {
  case '1':
    para->routing_scheme = ADAPTIVE ;
    return(1) ;
  case '2':
    para->routing_scheme = NONADAPTIVE ;
    return(2) ;
  default:
    return(0) ;
  }
}
/*---------------------------------------------------------------------------*/
void print_VC_assignment_algorithm() 
{
  printf("Determine virual channel assignment algorithm\n") ;
  printf("1. Static algorithm\n") ;
  printf("2. Static algorithm with Dynamic Buffer Pool\n") ;
  printf("3. Dynamic algorithm\n") ;
  printf("4. Dynamic algorithm with Throttling\n") ;
}
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int select_channel_finding_methods()
{
  char c ;
  c = getchar() ;
  switch(c) {
  case '1':
    para->chan_finding = 1 ;
    return(1) ;
  case '2':
    para->chan_finding = 2 ;
    return(2) ;
  default:
    return(0) ;
  }
}
/*---------------------------------------------------------------------------*/
void print_menu()
{
  printf(" c     ---- advance network one cycle\n") ;
  printf(" d     ---- drain messages in network\n") ;
  printf(" e x y ---- select experiment: x experiment_nr y source_type\n") ;
  printf(" h     ---- show variables\n") ;
  printf(" i     ---- initialize network\n") ;
  printf(" m x y ---- print active message with x src_nr y generated_time\n") ;
  printf(" p     ---- print statistics\n") ;
  printf(" q     ---- exit\n") ;
  printf(" r x   ---- advance network x cycles\n") ;
  printf(" s x y ---- set value y into variable x\n") ;
  printf(" u     ---- update network states\n") ;
  printf(" D x   ---- drain messages x cycles\n") ;
  printf(" L     ---- print lane category\n") ;
  printf(" M     ---- print all active messages\n") ;
  printf(" N x   ---- print output channel states of x node\n") ; 
  printf(" S x   ---- print source x and its destination\n") ; 
}
/*---------------------------------------------------------------------------*/
void set_special_lane()
{
  int i, j, k, nr_chan, nr_lanes;
  
  nr_lanes = para->nr_lanes ;
  if(para->k_array)  nr_chan = para->n_cube ; 
  else           nr_chan = 2*para->n_cube ; 
  for(i=0; i<nr_nodes; i++) {
    for(j=0; j<nr_chan; j++) {
      for(k=0; k<para->nr_lanes; k++) {
	if(k == para->nr_lanes-1 ) {
	(&nodes[i])->outputs[j]->lanes[k].type = SPECIAL ;
	}
	else (&nodes[i])->outputs[j]->lanes[k].type = NORMAL ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
void print_selecting_priorities() 
{
  printf("Determine priorities of the next channel selection.\n") ;
  printf("1. Free buffer threshold   2. Max delta of each dimension\n") ;
  printf("3. Minimize DR             4. Lowest dimension\n") ;
}
/*---------------------------------------------------------------------------*/
void select_priorities() 
{
  int i, done ;

  for(i=1; i<5; i++) {
    printf("Select priority(%d) ?", i) ;
    do {
      done = select_priority_num(i) ;
    } while(!done) ;
    if(done == 4) break ;
  }
}
/*---------------------------------------------------------------------------*/
int select_priority_num(int i) 
{
  int number ;
  scanf("%d", &number) ;
  if((number < 1) && (number > 5)) return(0) ;
  else {
    switch(i) {
    
    case 1:
      para->chan_sel_priority_1 = number ;
      return(number) ;
    case 2:
      para->chan_sel_priority_2 = number ;
      return(number) ;
    case 3:
      para->chan_sel_priority_3 = number ;
      return(number) ;
    case 4:
      para->chan_sel_priority_4 = number ;
      return(number) ;
    default:
      return(0) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
void select_timeout_scheme() 
{
  int ans ;
  char c ;
  printf("Do you employ Timeout Message Waiting scheme ?") ;
  do {
    ans = get_answer() ;
  } while(!ans) ;
  if(ans == 2) return ;
  else {
    if(para->chan_finding == SHORTEST_PATH) {
      para->timeout_mode = SHORTEST_PATH ;
      select_timeout_count() ;
      return ;
    }
    else if(para->chan_finding == ANY_CHANNEL) {
      printf("Timeout scheme within shortest paths ?") ;
      do {
	ans = get_answer() ;
      } while(!ans) ;
      if(ans == 1) {
	para->timeout_mode = SHORTEST_PATH ;
	select_timeout_count() ;
      }
      else {
	printf("Timeout scheme to any channels ?") ;
	do {
	  ans = get_answer() ;
	} while(!ans) ;
	if(ans == 1) {
	  para->timeout_mode = ANY_CHANNEL ;
	  select_timeout_count() ;
	}
	else return ;
      }
    }
  }
}
/*---------------------------------------------------------------------------*/
int get_answer() 
{
  char c ;
  c = getchar() ;
  switch(c) {
  case 'y':
    return(1) ;
  case 'n':
    return(2) ;
  default:
    return(0) ;
  }
}
/*---------------------------------------------------------------------------*/
void select_timeout_count() 
{
  int done, number ;
  printf("Input timeout counts ?") ;
  do {
    scanf("%d", &number) ;
    if(number > 0) done = 1 ;
    else done = 0 ;
  } while(!done) ;
  para->timeout_count = number ;
}
/*---------------------------------------------------------------------------*/
