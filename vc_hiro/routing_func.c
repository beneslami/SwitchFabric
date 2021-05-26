/*---------------------------------------------------------------------------*/
/*
 |    Network Simulator knetsim - "routing_func.c"
 |    Routing functions
 |    Hiromichi Aoki,  July 27 1990
 */
/*---------------------------------------------------------------------------*/
#include <math.h>
#include "defs.h"
#include "netdef.h"
#include "routing_func.h"
/*-------------------------------------------------------------------------*/
/*  Function Declaration */
/*-------------------------------------------------------------------------*/
   int reduce_lane_on_DR() ; 
   int reduce_in_FBT() ; 
   int reduce_in_max_delta() ;
   int reduce_in_minimize_DR() ;
   int reduce_in_lowest_dimension() ;
   int chain_in_shortest() ;
   int chain_in_any_channel() ;
   void search_in_priority() ;
   void message_scratched() ;
   void print_nc_table() ; 
   LANE *check_empty_free_lane() ; 
   LANE *check_in_fixed() ; 
   LANE *check_in_dynamic() ;
   LANE *check_in_dynamic_with_throttling() ;
   LANE *empty_free_lane() ; 
   LANE *find_next_lane_nonadaptive_mesh() ;
   LANE *find_next_lane_adaptive() ;
   LANE *find_next_lane_source_nonadaptive_mesh() ;
   LANE *find_next_lane_source_adaptive() ;
   LANE *select_lane() ;
   LANE *special_lane_in_channel() ;
   LANE *special_lane_in_shortest() ;
   LANE *find_special_lane() ;
   LANE *select_lane() ;
   CHANNEL *next_in_shortest() ;
   CHANNEL *find_channel_for_scratched_message() ;
   NC_TABLE *free_one_from_nc_table() ;
/*-------------------------------------------------------------------------*/
void init_nc_table()
{
  int i ;

  para_c->nc_table = NEW_ARRAY(NC_TABLE, 2*(para->nr_chan)) ;
  para_c->first_nc_table = para_c->nc_table ;
  for(i=0; i<2*(para->nr_chan); i++) {
    para_c->nc_table[i].channel = NULL_CHANNEL ;
    para_c->nc_table[i].next = NULL_NC_TABLE ;
    para_c->nc_table[i].prev = NULL_NC_TABLE ;
    para_c->nc_table[i].lane = NULL_LANE ;
    para_c->nc_table[i].seq_nr = NULL ;
  }
}
/*-------------------------------------------------------------------------*/
LANE *find_next_lane(MESSAGE *message)
{
  LANE *lane ;
  int routing_scheme, chan_finding ;

  routing_scheme = para->routing_scheme ;
  if(routing_scheme == NONADAPTIVE) {
    lane = find_next_lane_nonadaptive_mesh(message) ;
    if(lane == NULL_LANE) {
      if(cur_stats) {
	cur_stats->nr_failure_next_lane_finding ++ ;
      }
    }
    return(lane) ;
  }
  else if(routing_scheme == ADAPTIVE) {
    para_c->nc_table = para_c->first_nc_table ;
    lane = find_next_lane_adaptive(message) ;
    if(lane == NULL_LANE) {
      if(cur_stats) {
	cur_stats->nr_failure_next_lane_finding ++ ;
      }
    }
    return(lane) ;
  }
  else {error("undefined value in routing_scheme'\n"); exit(1) ;} 
}
/*-------------------------------------------------------------------------*/
/*    Function 'find_next_lane_nonadaptive' is for
 |    non-adaptive routing.
/*-------------------------------------------------------------------------*/
LANE *find_next_lane_nonadaptive_mesh(MESSAGE *message)
{
  int select_dim, select_dire, i, dest_p, cure_p, lane_id, count, 
      seq_chan_id, lowest_dim, message_dim, N_OPY ;
  CHANNEL *selected_channel = NULL_CHANNEL ;
  LANE *lane ;
  NODE *dest_node, *cure_node ;

  /* start from the lowest dimension */
  dest_node = message->dest_node ;
  if(message->head_lane) {
    message_dim = message->head_lane->channel->dim ;
    cure_node = message->head_lane->channel->dst ;
  }
  else {
    message_dim = 0 ;
    cure_node = message->src_node ;
  }
  for(i=0; i<para->nr_dim; i++) {
    dest_p = dest_node->id_dimensions[i].value ;
    cure_p = cure_node->id_dimensions[i].value ;
    if(dest_p!=cure_p) {
      lowest_dim = i ;
      break ;
    }
  }
  select_dim = lowest_dim ;
  if(dest_p > cure_p) select_dire = POS ;
  else select_dire = NEG ;
  seq_chan_id = 2*select_dim + select_dire ;
  selected_channel = cure_node->outputs[seq_chan_id] ;
  if(selected_channel->state != DISCON && 
     selected_channel->state != FAILED && N_OPY != para->nr_lanes) {
    message->state = MESSAGE_FOUND_LANE ;
    return(first_free_lane(selected_channel)) ;
  }
  else {
    message->state = MESSAGE_FAILED_LANE ;
    return(NULL_LANE) ;
  }
}
/*-------------------------------------------------------------------------*/
/* 
 |     Function 'find_next_lane_adaptive' 
 */
/*-------------------------------------------------------------------------*/
LANE *find_next_lane_adaptive(MESSAGE *message)
{
  int M_DR, c_dim, c_dire, timeout_mode, chan_exist, lane_exist ; 
  int selected_lane, can_wait ;
  NODE *c_node, *d_node ;
  CHANNEL *next_channel =NULL_CHANNEL ;
  LANE *lane ;

  if(message->face == SCRATCHED) {
    lane = find_special_lane(message) ;
    return(lane) ;
  }
  else {
    c_dim = message->head_lane->channel->dim ;
    c_dire = message->head_lane->channel->dire ;
    c_node = message->head_lane->channel->dst ;
    d_node = message->dest_node ;
    M_DR = message->M_DR ;
    timeout_mode = para->timeout_mode ;
    /* wring the next nchannel */
    if(para->chan_finding == SHORTEST_PATH) {
      chan_exist = chain_in_shortest(c_node, d_node, c_dim, c_dire) ;
      if(chan_exist) {
	can_wait = check_message_can_wait(message) ;
	lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
      } 
      else {
	can_wait = NULL ;
	lane_exist = NULL ;
      }
      if(!lane_exist) {
/* printf("###########  M_DR=%d  can_wait=%d cur_time=%d\n", M_DR, can_wait, cur_time);
printf("message %x[ src:%d t:%d head_dst=%d\n", 
        message, message->src_node->id_seq, message->start_time, c_node->id_seq) ;
print_nc_table() ;
print_channel_states(c_node->id_seq) ;*/
	if(chan_exist && can_wait || !chan_exist) {
	  return(NULL_LANE) ;
	}
	else {
	  if(timeout_mode == SHORTEST_PATH || timeout_mode == BOTH) {
	    if(message->wait_count < para->timeout_count) {
	      message->wait_count++ ;
	      return(NULL_LANE) ;
	    }
	    else {
	      message_scratched(message) ;
	      lane = find_special_lane(message) ;
	      return(lane) ;
	    }
	  }
	  else {
	    message_scratched(message) ;
	    lane =find_special_lane(message) ;
	    return(lane) ;
	  }
	}
      }      
      else {
	search_in_priority(c_node, d_node, c_dim, c_dire, M_DR) ;
	lane = select_lane(para_c->nc_table) ;
	return(lane) ;
      }
    }
    else if(para->chan_finding == ANY_CHANNEL) {
      chan_exist = chain_in_shortest(c_node, d_node, c_dim, c_dire) ;
      if(chan_exist) {
	can_wait = check_message_can_wait(message) ;
	lane_exist = reduce_lane_on_DR(c_dim, M_DR) ; 
      }
      else {
	can_wait = NULL ;
	lane_exist = NULL ;
      }
      if(!lane_exist) {
	if(chan_exist && can_wait){
	  return(NULL_LANE) ;
	}
	else if(chan_exist && !can_wait) {
	  if(timeout_mode == SHORTEST_PATH || timeout_mode == BOTH) {
	    if(message->wait_count < para->timeout_count) {
	      message->wait_count++ ;
	      return(NULL_LANE) ;
	    }
	    else {
	      para_c->nc_table = para_c->first_nc_table ;
	      chan_exist = chain_in_any_channel(c_node, d_node, c_dim, c_dire);
	      if(chan_exist) {
		can_wait = check_message_can_wait(message) ;
		lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
	      } 
	      else {
		can_wait = NULL ;
		lane_exist = NULL ;
	      }
	      if(!lane_exist) {
		if(chan_exist && can_wait || !chan_exist) {
                  return(NULL_LANE) ;
                }
		else {
		  if(timeout_mode == BOTH) {
		    if(message->wait_count < para->timeout_count) {
		      message->wait_count++ ;
		      return(NULL_LANE) ;
		    }
		    else {
		      message_scratched(message) ;
		      lane = find_special_lane(message) ;
		      return(lane) ;
		    }
		  }
		  else {
		    message_scratched(message) ;
		    lane = find_special_lane(message) ;
		    return(lane) ;
		  }
		}
	      }	    
	      else {
		search_in_priority(c_node, d_node, c_dim, c_dire, M_DR) ;
		lane = select_lane(para_c->nc_table) ;
		return(lane) ;
	      }
	    }
	  }
	  else {  /* timeout_mode == ANY_CHANNEL or NULL */
	    para_c->nc_table = para_c->first_nc_table ;
	    chan_exist = chain_in_any_channel(c_node, d_node, c_dim, c_dire) ;
	    if(chan_exist) {
	      can_wait = check_message_can_wait(message) ;
	      lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
	    } 
	    else {
	      can_wait = NULL ;
	      lane_exist = NULL ;
	    }
	    if(!lane_exist) {
	      if(chan_exist && can_wait || !chan_exist) {
		return(NULL_LANE) ;
	      }
	      else {
		if(timeout_mode == ANY_CHANNEL) {
		  if(message->wait_count < para->timeout_count) {
		    message->wait_count++ ;
		    return(NULL_LANE) ;
		  }
		  else {
		    message_scratched(message) ;
		    lane = find_special_lane(message) ;
		    return(lane) ;
		  }
		}
		else {
		  message_scratched(message) ;
		  lane = find_special_lane(message) ;
		  return(lane) ;
		}
	      }
	    }	    
	    else {
	      search_in_priority(c_node, d_node, c_dim, c_dire, M_DR) ;
	      if(para_c->nc_table->next == NULL_NC_TABLE) {
		lane = select_lane(para_c->nc_table) ;
		return(lane) ;
	      }
	      else {
		print_nc_table() ;
		printf("time=%d\n", cur_time) ;
		error("nc_table reduction failed\n") ; exit(3) ;
	      }
	    }
	  }
	}
	else { /* !chan_exist */
	  para_c->nc_table = para_c->first_nc_table ;
	  chan_exist = chain_in_any_channel(c_node, d_node, c_dim, c_dire) ;
	  if(chan_exist) {
	    can_wait = check_message_can_wait(message) ;
	    lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
	  } 
	  else {
	    can_wait = NULL ;
	    lane_exist = NULL ;
	  }
	  if(!lane_exist) {
	    if(chan_exist && can_wait || !chan_exist) {
	      return(NULL_LANE) ;
	    }
	    else {
	      if(timeout_mode == BOTH) {
		if(message->wait_count < para->timeout_count) {
		  message->wait_count++ ;
		  return(NULL_LANE) ;
		}
		else {
		  message_scratched(message) ;
		  lane = find_special_lane(message) ;
		  return(lane) ;
		}
	      }
	      else {
		message_scratched(message) ;
		lane = find_special_lane(message) ;
		return(lane) ;
	      }
	    }
	  }	    
	  else {
	    search_in_priority(c_node, d_node, c_dim, c_dire, M_DR) ;
	    lane = select_lane(para_c->nc_table) ;
	    return(lane) ;
	  }
	}
      }
      else { /* lane_exist */
	search_in_priority(c_node, d_node, c_dim, c_dire, M_DR) ;
	lane = select_lane(para_c->nc_table) ;
	return(lane) ;
      }
    }
    else {error("undefined value in chan_finding") ; exit(1) ;}
  }
}
/*-------------------------------------------------------------------------*/
int check_message_can_wait(MESSAGE *message) 
{
  int message_dim, max_DR, table_ch_dim, M_DR ;
  NC_TABLE *nc_table ;

  nc_table = para_c->nc_table ;
  message_dim = message->head_lane->channel->dim ;
  while(nc_table) {
    max_DR = nc_table->channel->max_DR ;
    M_DR = message->M_DR ;
    table_ch_dim = nc_table->channel->dim ;
    if(table_ch_dim < message_dim) M_DR++ ;
    if(max_DR >= M_DR) return(TRUE) ;
    nc_table = nc_table->next ;
  }
  return(NULL) ;
}
/*-------------------------------------------------------------------------*/
void search_in_priority(NODE *c_node, NODE *d_node, int c_dim, int M_DR)
{
  int priority_1, priority_2, priority_3, priority_4, exist ;
 
  priority_1 = para->chan_sel_priority_1 ;
  priority_2 = para->chan_sel_priority_2 ; 
  priority_3 = para->chan_sel_priority_3 ; 
  priority_4 = para->chan_sel_priority_4 ;
  if(priority_1) {
    if(priority_1 == 1) {
      exist = reduce_in_FBT() ;  /* free buffer threshold */
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
      }
    else if(priority_1 == 2) {
      exist = reduce_in_max_delta(c_node, d_node) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_1 == 3) {
      exist = reduce_in_minimize_DR(c_dim, M_DR) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_1 == 4) {
      exist = reduce_in_lowest_dimension() ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else {error("undefined value in priority_1") ; exit(1) ;}
  }
  if(priority_2) {
    if(priority_2 == 1) {
      exist = reduce_in_FBT() ;  /* free buffer threshold */
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
      }
    else if(priority_2 == 2) {
      exist = reduce_in_max_delta(c_node, d_node) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_2 == 3) {
      exist = reduce_in_minimize_DR(c_dim, M_DR) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_2 == 4) {
      exist = reduce_in_lowest_dimension() ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else {error("undefined value in priority_1") ; exit(1) ;}
  }
  if(priority_3) {
    if(priority_3 == 1) {
      exist = reduce_in_FBT() ;  /* free buffer threshold */
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
      }
    else if(priority_3 == 2) {
      exist = reduce_in_max_delta(c_node, d_node) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_3 == 3) {
      exist = reduce_in_minimize_DR(c_dim, M_DR) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_3 == 4) {
      exist = reduce_in_lowest_dimension() ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else {error("undefined value in priority_1") ; exit(1) ;}
  }
  if(priority_4) {
    if(priority_4 == 1) {
      exist = reduce_in_FBT() ;  /* free buffer threshold */
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
      }
    else if(priority_4 == 2) {
      exist = reduce_in_max_delta(c_node, d_node) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_4 == 3) {
      exist = reduce_in_minimize_DR(c_dim, M_DR) ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else if(priority_4 == 4) {
      exist = reduce_in_lowest_dimension() ;
      if(!exist) {error("error at reduce_in_FBT()\n") ; exit(1) ;}
    }
    else {error("undefined value in priority_1") ; exit(1) ;}
  }
}
/*-------------------------------------------------------------------------*/
int chain_in_shortest(NODE *c_node, NODE *d_node, int c_dim, int c_dire) 
{
  int i, j, cure_p, dest_p, select_dim, select_dire, seq_chan_id ;
  CHANNEL *n_channel ;

  j = 0 ;
  for(i=0; i<(para->nr_dim); i++) {
    cure_p = c_node->id_dimensions[i].value ;
    dest_p = d_node->id_dimensions[i].value ;
    if(cure_p != dest_p) {
      select_dim = i ;
      if(dest_p > cure_p) select_dire = POS ;
      else select_dire = NEG ;
      if((select_dim != c_dim) || !select_dire != c_dire) {
	seq_chan_id = 2*select_dim + select_dire ;
	n_channel = c_node->outputs[seq_chan_id] ;
	if(n_channel->state != DISCON && n_channel->state != FAILED){
	  para_c->nc_table[j].channel = n_channel ;
	  para_c->nc_table[j].seq_nr = j ;
	  para_c->nc_table[j].channel->misrouted = NULL ;
	  if(j!=0) para_c->nc_table[j].prev =&(para_c->nc_table[j-1]) ;
	  para_c->nc_table[j].next = &(para_c->nc_table[j+1]) ;
	  j++ ;
	}
      }
    }
  }
  para_c->nc_table[0].prev = NULL_NC_TABLE ;
  if(j!=0) para_c->nc_table[j-1].next = NULL_NC_TABLE ;
  return(j) ;
}
/*-------------------------------------------------------------------------*/
int chain_in_any_channel(NODE *c_node, NODE *d_node, int c_dim, int c_dire) 
{
  int i, j, cure_p, dest_p, select_dim, select_dire, seq_chan_id ;
  CHANNEL *n_channel ;

  j = 0 ;
  for(i=0; i<(para->nr_chan); i++) {
    select_dim = (i/2)%(para->nr_dim) ;
    select_dire = i%2 ;
    if((select_dim != c_dim) || !select_dire != c_dire) { 
      seq_chan_id = i ;
      n_channel = c_node->outputs[seq_chan_id] ;
      if(n_channel->state != DISCON && n_channel->state != FAILED) {
	para_c->nc_table[j].channel = n_channel ;
	para_c->nc_table[j].seq_nr = j ;
	para_c->nc_table[j].channel->misrouted = 1 ;
	if(j!=0) para_c->nc_table[j].prev =&(para_c->nc_table[j-1]) ;
	para_c->nc_table[j].next = &(para_c->nc_table[j+1]) ;
	j++ ;
      }
    }
  }
  para_c->nc_table[0].prev = NULL_NC_TABLE ;
  if(j!=0) para_c->nc_table[j-1].next = NULL_NC_TABLE ;
  return(j) ;
}
/*-------------------------------------------------------------------------*/
void message_scratched(MESSAGE *message)
{
  if(para->test_category == FAULT_TEST) { /* fault test */
    /* print_one_message(message->src_node->id_seq, message->start_time) ;
    print_channel_states(message->head_lane->channel->dst->id_seq) ; */
  }
  message->face = SCRATCHED ;
}
/*-------------------------------------------------------------------------*/
LANE *find_special_lane(MESSAGE *message) 
{
  CHANNEL *channel ;
  LANE *lane ;
  channel = find_channel_for_scratched_message(message) ;
  if(channel->state != DISCON && channel->state != FAILED) {
    lane = special_lane_in_channel(channel) ;
    return(lane) ;
  }
  else return(NULL_LANE) ;
}
/*-------------------------------------------------------------------------*/
CHANNEL *find_channel_for_scratched_message(MESSAGE *message)
{
  int i, selected_dim, selected_dire, seq_chan_id ;
  int cure_p, dest_p, lowest_dim, nr_lane ; 
  NODE *c_node, *d_node ;
  CHANNEL *n_channel ;
   
  c_node = message->head_lane->channel->dst ;
  d_node = message->dest_node ;
  nr_lane = para->nr_dim ;
  for(i=0; i<nr_lane; i++) {
    cure_p = c_node->id_dimensions[i].value ;
    dest_p = d_node->id_dimensions[i].value ;
    if(cure_p != dest_p) {
      lowest_dim = i ;
      break ;
    }
  }
  selected_dim = lowest_dim ;
  if(dest_p > cure_p) selected_dire = POS ;
  else selected_dire = NEG ;
  seq_chan_id = 2*selected_dim + selected_dire ;
  n_channel = c_node->outputs[seq_chan_id] ;
  return(n_channel) ;
}
/*-------------------------------------------------------------------------*/
LANE *special_lane_in_channel(CHANNEL *channel)
{
  int i, min, max ;
  min = para_c->special_min_lane_nr ;
  max = para_c->special_max_lane_nr ;
  for(i=min; i<max+1; i++) {
    if(channel->lanes[i].state == IDLE) return(&(channel->lanes[i])) ;
  }
  return(NULL_LANE) ;
}
/*-------------------------------------------------------------------------*/
int reduce_lane_on_DR(int message_dim, int M_DR)
{
  int M_DR_c, cure_dim ;
  NC_TABLE *nc_table ;

  nc_table = para_c->nc_table ;
  if(nc_table == NULL_NC_TABLE) return(NULL) ;
  else {
    while(nc_table) {
      cure_dim = nc_table->channel->dim ;
      M_DR_c = M_DR ;
      if(cure_dim < message_dim) M_DR_c++ ;
      if(!check_DR_in_channel(nc_table, M_DR_c)) {
	nc_table = free_one_from_nc_table(nc_table) ;
      }
      else nc_table = nc_table->next ;
    }
    if(para_c->nc_table == NULL_NC_TABLE) return(NULL) ;
    else return(TRUE) ;
  }
}
/*-------------------------------------------------------------------------*/
NC_TABLE *free_one_from_nc_table(NC_TABLE *c_nc_table) 
{
  if(c_nc_table->prev != NULL_NC_TABLE) {
    c_nc_table->prev->next = c_nc_table->next ;
    if(c_nc_table->next != NULL_NC_TABLE) {
      c_nc_table->next->prev = c_nc_table->prev ;
    }
  }
  if(c_nc_table == para_c->nc_table) {
    para_c->nc_table = c_nc_table->next ;
  }
  return(c_nc_table->next) ;
}
/*-------------------------------------------------------------------------*/
int check_DR_in_channel(NC_TABLE *nc_table, int M_DR) 
{
  int free_OK ;
  CHANNEL *channel ;
  LANE *lane ;
  int misrouted ;

  channel = nc_table->channel ;
  misrouted = channel->misrouted ;
  if(para_c->VC_category == 1) {
    lane = check_in_fixed(channel, M_DR) ;
    if(lane != NULL_LANE) {
      nc_table->lane = lane ;
      nc_table->lane->misrouted = misrouted ;
      return(TRUE) ;
    }
    else return(NULL) ;
  }
  else if(para_c->VC_category == 2) {
    lane = check_in_fixed(channel, M_DR) ;
    if(lane != NULL_LANE) {
      nc_table->lane = lane ;
      nc_table->lane->misrouted = misrouted ;
      return(TRUE) ;
    }
    else {
      free_OK = MDR_exist_in_fixed(channel, M_DR) ;
      if(free_OK) {
	lane = empty_free_lane(channel) ;
	if(lane != NULL_LANE) {
	  nc_table->lane = lane ;
	  nc_table->lane->misrouted = misrouted ;
	  return(TRUE) ;
	}
	else return(NULL) ;
      }
      else return(NULL) ;
    }
  }
  else if(para_c->VC_category == 3) {
    lane = check_in_dynamic(channel, M_DR) ;
    if(lane != NULL_LANE) {
      nc_table->lane = lane ;
      nc_table->lane->misrouted = misrouted ;
      return(TRUE) ;
    }
    else {
      return(NULL) ;
    }
  }
  else if(para_c->VC_category == 4) {
    lane = check_in_dynamic_with_throttling(channel, M_DR) ;
    if(lane != NULL_LANE) {
      nc_table->lane = lane ;
      nc_table->lane->misrouted = misrouted ;
      return(TRUE) ;
    }
    else {
      return(NULL) ;
    }
  }
}  
/*-------------------------------------------------------------------------*/
LANE *empty_free_lane(CHANNEL *channel) 
{
  int i, MIN, max ;
  LANE *lane ;

  MIN = para_c->free_min_lane_nr ;
  max = para_c->free_max_lane_nr ;
  for(i=MIN; i<max+1; i++) {
    lane = &(channel->lanes[i]) ;
    if(lane->state == IDLE) return(lane) ;
  }
  return(NULL_LANE) ;
}
/*-------------------------------------------------------------------------*/
int MDR_exist_in_fixed(CHANNEL *channel, int M_DR) 
{
  if((M_DR < para_c->max_class) && (channel->state != DISCON) &&
     channel->state != FAILED) {
    if(para_c->class_i_exist[M_DR].value == TRUE) return(TRUE) ;
    else return(NULL) ;
  }
  else return(NULL) ;
}
/*-------------------------------------------------------------------------*/
LANE *check_in_fixed(CHANNEL *channel, int M_DR) 
{
  int i, lowest_VC, highest_VC ;
  LANE *lane ;

  if(M_DR < para_c->max_class+1) {
    if(para_c->class_i_exist[M_DR].value) {
      lowest_VC = para_c->lowest_VC_nr_class_i[M_DR].value ;
      highest_VC = para_c->highest_VC_nr_class_i[M_DR].value ;
      for(i=lowest_VC; i<highest_VC+1; i++) {
	lane = &(channel->lanes[i]) ;
	if(lane->state == IDLE)  return(lane) ;
      }
      return(NULL_LANE) ;
    }
    else return(NULL_LANE) ;
  }
  else return(NULL_LANE) ;
}
/*-------------------------------------------------------------------------*/
LANE *check_in_dynamic(CHANNEL *channel, int M_DR) 
{
  int i, min, max ;
  LANE *lane ;

  min = para_c->dynamic_min_lane_nr ;
  max = para_c->dynamic_max_lane_nr ;
  for(i=min; i<max+1; i++) {
    lane = &(channel->lanes[i]) ;
    if(lane->state == IDLE) return(lane) ;
  }
  return(NULL_LANE) ;
}
/*---------------------------------------------------------------------------*/
LANE *check_in_dynamic_with_throttling(CHANNEL *channel, int M_DR) 
{
  int i, min, max ;
  LANE *lane ;

  if(M_DR == 0) {
    min = para_c->dynamic_min_lane_nr ;
    max = para_c->throttling_boundary -1 ;
    for(i=min; i<max+1; i++) {
      lane = &(channel->lanes[i]) ;
      if(lane->state == IDLE) return(lane) ;
    }
    return(NULL_LANE) ;
  }
  else {
    min = para_c->throttling_boundary ;
    max = para_c->dynamic_max_lane_nr ;
    for(i=min; i<max+1; i++) {
      lane = &(channel->lanes[i]) ;
      if(lane->state == IDLE) return(lane) ;
    }
    min = para_c->dynamic_min_lane_nr ;
    max = para_c->throttling_boundary -1 ;
    for(i=min; i<max+1; i++) {
      lane = &(channel->lanes[i]) ;
      if(lane->state == IDLE) return(lane) ;
    }
    return(NULL_LANE) ;
  }  
}
/*---------------------------------------------------------------------------*/
LANE *find_next_lane_at_source(SOURCE *source)
{
  LANE *lane ;
  int routing_scheme, chan_finding ;

  routing_scheme = para->routing_scheme ;
  chan_finding = para->chan_finding ;
  if(routing_scheme ==NONADAPTIVE) {
    lane = find_next_lane_source_nonadaptive_mesh(source) ;
    return(lane) ;
  }
  else {
    if(routing_scheme == ADAPTIVE) {
      para_c->nc_table = para_c->first_nc_table ;
      lane = find_next_lane_source_adaptive(source) ;
      return(lane) ;
    }
    else {error("undefined value in routing_scheme\n"); exit(1) ;} 
  }
}
/*-------------------------------------------------------------------------*/
/*  
 |    Function 'find_next_lane_nonadaptive' is for
 |    non-adaptive routing.
 |    This function is used in message injection stage.
 */
/*-------------------------------------------------------------------------*/
LANE *find_next_lane_source_nonadaptive_mesh(SOURCE *source)
{
  int select_dim, lowest_dim, select_dire, i, dest_p, cure_p, seq_chan_id ;
  CHANNEL *selected_channel = NULL_CHANNEL ;
  LANE *lane ;
  NODE *dest_node, *cure_node ;

  /* start from the lowest dimension */
  dest_node = source->dest_node ;
  cure_node = source->node ;
  for(i=0; i<para->nr_dim; i++) {
    dest_p = dest_node->id_dimensions[i].value ;
    cure_p = cure_node->id_dimensions[i].value ;
    if(dest_p!=cure_p) {
      lowest_dim = i ;
      break ;
    }
  }
  select_dim = lowest_dim ;
  if(dest_p > cure_p) select_dire = POS ;
  else select_dire = NEG ;
  seq_chan_id = 2*select_dim + select_dire ;
  selected_channel = cure_node->outputs[seq_chan_id] ;
  if(selected_channel->state != DISCON &&
     selected_channel->state != FAILED &&
     selected_channel->nr_ocpied_lanes != para->nr_lanes) {
    return(first_free_lane(selected_channel)) ;
  }
  else return(NULL_LANE) ;
}
/*-------------------------------------------------------------------------*/
/* 
 |     Function 'find_next_lane_source_adaptive_NO1' 
 */
/*-------------------------------------------------------------------------*/
LANE *find_next_lane_source_adaptive(SOURCE *source)
{
  int chan_exist, lane_exist, c_dim, c_dire, M_DR ;
  LANE *lane ;
  NODE *c_node, *d_node ;

  c_node = source->node ;
  d_node = source->dest_node ;
  c_dim = M_DR = 0 ;
  c_dire = 3 ; 
  if(para->chan_finding == SHORTEST_PATH) {
    chan_exist = chain_in_shortest(c_node, d_node, c_dim, c_dire) ;
    if(chan_exist) {
      lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
    } 
    else {
      lane_exist = NULL ;
    }
    if(!lane_exist) {
      return(NULL_LANE) ;
    }
    else { /* lane_exist */
      search_in_priority(c_node, d_node, c_dim, M_DR) ;
      lane = select_lane(para_c->nc_table) ;
      return(lane) ;
    }
  }
  else if(para->chan_finding == ANY_CHANNEL) {
    chan_exist = chain_in_shortest(c_node, d_node, c_dim, c_dire) ;
    if(chan_exist) {
      lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
    } 
    else {
      lane_exist = NULL ;
    }
    if(!lane_exist) {
      para_c->nc_table = para_c->first_nc_table ;
      chan_exist = chain_in_any_channel(c_node, d_node, c_dim, c_dire) ;
      if(chan_exist) {
	lane_exist = reduce_lane_on_DR(c_dim, M_DR) ;
      } 
      else {
	lane_exist = NULL ;
      }
      if(!lane_exist) {
	return(NULL_LANE) ;
      }
      else { /* lane_exist in any channel */
	search_in_priority(c_node, d_node, c_dim, M_DR) ;
	lane = select_lane(para_c->nc_table) ;
	return(lane) ;
      }
    }
    else { /* lane_exist in shortest */
      search_in_priority(c_node, d_node, c_dim, M_DR) ;
      lane = select_lane(para_c->nc_table) ;
      return(lane) ;
    }
  }
  else {error("undefined value of routing_scheme\n") ; exit(1) ;}
}
/*-------------------------------------------------------------------------*/
LANE *select_lane(NC_TABLE *nc_table)
{
  return(nc_table->lane) ;
}
/*---------------------------------------------------------------------------*/
/* find first free lane - return lane address if found, return null value
/* if not found 
/*---------------------------------------------------------------------------*/
LANE *first_free_lane(CHANNEL *channel) 
{
  int i ;
  LANE *lane, *flane ;
  lane = channel->lanes ;
  for(i=0; i<(channel->nr_lanes); i++) {
    if((&lane[i])->state == IDLE) {
      flane = &(lane[i]) ;
      break ;
    }
    else flane = NULL_LANE ;
  }
  return(flane) ;
}
/*---------------------------------------------------------------------------*/
/* find first free lane except for a special lane  - return lane address 
/* if found, return null value if not found */
/*---------------------------------------------------------------------------*/
LANE *first_empty_lane_in_free_lanes(CHANNEL *channel) 
{
  int i ;
  LANE *lane, *flane ;
  lane = channel->lanes ;
  for(i=0; i<(channel->nr_lanes); i++) {
    if((&lane[i])->state == IDLE) {
      flane = &(lane[i]) ;
      break ;
    }
    else flane = NULL_LANE ;
  }
  return(flane) ;
}
/*---------------------------------------------------------------------------*/
int check_new_message_can_enter(LANE *lane) 
{
  int i, cure_dim, cure_dire, cure_p, dest_p, selected_dim, selected_dire ;
  int lowest_dim, direction, routing_scheme, chan_finding, lane_nr ;
  int P_distance, N_distance ;
  SOURCE *source ;
  NODE *cure_node, *dest_node ;
  CHANNEL *selected_channel ;

  routing_scheme = para->routing_scheme ;
  chan_finding = para->chan_finding ;
  cure_dim = lane->channel->dim ;
  cure_dire = lane->channel->dire ;
  source = lane->channel->src->source ;
  if(para->source_type == RANDOM) {
    new_source_dest(source) ;
  }
  dest_node = source->dest_node ;
  cure_node = source->node ;
  if(routing_scheme == NONADAPTIVE) {
    for(i=0; i<para->nr_dim; i++) {
      dest_p = dest_node->id_dimensions[i].value ;
      cure_p = cure_node->id_dimensions[i].value ;
      if(dest_p!=cure_p) {
	lowest_dim = i ;
	break ;
      }
    }
    selected_dim = lowest_dim ;
    if(selected_dim == cure_dim) {
      if(dest_p > cure_p) selected_dire = POS ;
      else if(dest_p < cure_p) selected_dire = NEG ;
      else return(NULL) ;
      selected_channel = lane->channel ;
      if(selected_dire == cure_dire) {
	if(selected_channel->state != DISCON &&
	   selected_channel->state != FAILED &&
	   selected_channel->nr_ocpied_lanes != para->nr_lanes) {
	  return(TRUE) ;
	}
	else return(NULL) ;
      }
      else return(NULL) ;
    }
    else return(NULL) ;
  }
  else if(routing_scheme == ADAPTIVE) {
    if(chan_finding ==SHORTEST_PATH) {
      cure_p = lane->channel->src->id_dimensions[cure_dim].value ;
      dest_p = source->dest_node->id_dimensions[cure_dim].value ;
      if(cure_p < dest_p) direction = POS ;
      else if(cure_p > dest_p) direction = NEG ;
      else return(NULL) ;
      if(cure_dire != direction) {
	return(NULL) ;
      }
    }
    if(lane->channel->state != DISCON && 
                                  lane->channel->state != FAILED) {
      lane_nr = lane->lane_id_nr ;
      if(para_c->VC_category == STATIC || para_c->VC_category == STATIC_DBP) {
	if(para_c->lane_types[lane_nr].category == FIXED) {
	  if(para_c->lane_types[lane_nr].class==0) {
	    return(TRUE) ;
	  }
	  else return(NULL) ;
	}
	else return(NULL) ;
      }
      else if(para_c->VC_category == DYNAMIC) {
	if(para_c->lane_types[lane_nr].category == DYNAMIC) {
	  return(TRUE) ;
	}
        else return(NULL) ;
      }
      else if(para_c->VC_category == HYBRID) {
	if(para_c->lane_types[lane_nr].category == DYNAMIC) {
	  if(lane_nr < para_c->throttling_boundary) {
	    return(TRUE) ;
	  }
	  else return(NULL) ;
	}
	else return(NULL) ;
      }
      else error("undefined VC category\n") ;
    }
    else return(NULL) ;
  }
  else error("undefined value at routing_scheme\n") ;
}
/*---------------------------------------------------------------------------*/
int reduce_in_FBT() 
{
  int c_nr_filled_lanes, n_nr_filled_lanes ;
  NC_TABLE *cure_table, *next_table ;

  cure_table = para_c->nc_table ;
  if(cure_table == NULL_NC_TABLE) return(NULL) ;
  else {
    while(cure_table) {
      next_table = cure_table->next ;
      if(next_table != NULL_NC_TABLE) {
	c_nr_filled_lanes = cure_table->channel->nr_ocpied_lanes ;
	n_nr_filled_lanes = next_table->channel->nr_ocpied_lanes ;
	if(c_nr_filled_lanes < n_nr_filled_lanes) {
	  cure_table = free_one_from_nc_table(next_table) ;
	}
	else if(c_nr_filled_lanes == n_nr_filled_lanes) {
	  cure_table = cure_table->next ;
	}
	else {
	  cure_table = free_one_from_nc_table(cure_table) ;
	}
      }
      else break ;
    }
    return(TRUE) ;
  }
}
/*---------------------------------------------------------------------------*/
int reduce_in_max_delta(NODE *c_node, NODE *d_node)
{
  int cure_dim, c_cure_p, c_dest_p, c_delta, next_dim, n_cure_p ;
  int n_dest_p, n_delta ;
  NC_TABLE *cure_table ;
  NC_TABLE *next_table ;

  cure_table = para_c->nc_table ;
  if(cure_table == NULL_NC_TABLE) return(NULL) ;
  else {
    while(cure_table) {
      cure_dim = cure_table->channel->dim ;
      c_cure_p = c_node->id_dimensions[cure_dim].value ;
      c_dest_p = d_node->id_dimensions[cure_dim].value ;
      c_delta = abs(c_cure_p - c_dest_p) ;
      next_table = cure_table->next ;
      if(next_table != NULL_NC_TABLE) {
	next_dim = next_table->channel->dim ;
	n_cure_p = c_node->id_dimensions[next_dim].value ;
	n_dest_p = d_node->id_dimensions[next_dim].value ;
	n_delta = abs(n_cure_p - n_dest_p) ;
	if(c_delta < n_delta) {
	  cure_table = free_one_from_nc_table(cure_table) ;
	}
	else if(c_delta == n_delta) {
	  cure_table = cure_table->next ;
	}
	else {
	  cure_table = free_one_from_nc_table(next_table) ;
	}
      }
      else break ;
    }
    return(TRUE) ;
  }
}
/*---------------------------------------------------------------------------*/
int reduce_in_minimize_DR(int message_dim, int M_DR)
{
  int cure_dim ; 
  NC_TABLE *cure_table ;
  int exist_DR_increasing, exist_DR_keeping ; 

  exist_DR_increasing = exist_DR_keeping = 0 ;
  cure_table = para_c->nc_table ;
  if(cure_table == NULL_NC_TABLE) return(NULL) ;
  else {
    while(cure_table) {
      cure_dim = cure_table->channel->dim ;
      if(cure_dim < message_dim) {
	exist_DR_increasing++ ;
	cure_table = cure_table->next ;
      }
      else {
	exist_DR_keeping++ ;
	cure_table = cure_table->next ;
      }
    }    
    if(exist_DR_increasing && !exist_DR_keeping) return(TRUE) ;
    else if(!exist_DR_increasing && exist_DR_keeping) return(TRUE) ;
    else {
      cure_table = para_c->nc_table ;
      while(cure_table) {
	cure_dim = cure_table->channel->dim ;
	  if(cure_dim < message_dim) {
	    cure_table = free_one_from_nc_table(cure_table) ;
	  }
	  else cure_table = cure_table->next ;
      }
      return(TRUE) ;
    }
  }
}
/*---------------------------------------------------------------------------*/
int reduce_in_lowest_dimension()
{
  int cure_dim, next_dim ;
  NC_TABLE *cure_table, *next_table ;

  cure_table = para_c->nc_table ;
  if(cure_table == NULL_NC_TABLE) return(NULL) ;
  else {
    while(cure_table) {
      cure_dim = cure_table->channel->dim ;
      next_table = cure_table->next ;
      if(next_table != NULL_NC_TABLE) { 
	next_dim = next_table->channel->dim ;
	if(cure_dim <= next_dim) {
	  cure_table = free_one_from_nc_table(next_table) ;
	}
	else cure_table = free_one_from_nc_table(cure_table) ;
      }
      else break ;
    }
    return(TRUE) ;
  }
}
/*---------------------------------------------------------------------------*/
void print_nc_table() 
{
  int init ;
  NC_TABLE *cure_table ;

  cure_table = para_c->nc_table ;
  while(cure_table) {
    printf("nc_tabel[%d]= %x\n", cure_table->seq_nr, cure_table) ;
    printf("    seq_nr= %d\n", cure_table->seq_nr) ;
    printf("    prev= %x\n", cure_table->prev) ;
    printf("    next= %x\n", cure_table->next) ;
    printf("    channel= %x ", cure_table->channel) ;
    if(cure_table->channel != NULL_CHANNEL) {
      printf("%d-%d\n", cure_table->channel->src->id_seq,
	     cure_table->channel->dst->id_seq) ;
    }
    printf("    lane= %x\n", cure_table->lane) ;
    if(cure_table->channel != NULL_CHANNEL) {
      printf("    dim= %d\n", cure_table->channel->dim) ;
    }
    cure_table = cure_table->next ;
  }
}
/*---------------------------------------------------------------------------*/
/* END
/*---------------------------------------------------------------------------*/

