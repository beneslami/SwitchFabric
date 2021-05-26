/*---------------------------------------------------------------------------*/
/*
 |    network simulator - set variable utility
 |    Hiromichi Aoki July 27 1990
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "netdef.h"
/*---------------------------------------------------------------------------*/
void create_set_table()
{
  set_table = NEW_ARRAY(SET_REC, 30) ;
  set_table[0].name = "debug" ; set_table[0].var = &debug; 
  set_table[0].min = 0; set_table[0].max = 0xFF; 
  set_table[1].name = "depth" ; set_table[1].var = &para->depth; 
  set_table[1].min = 1; set_table[1].max = MAX_DEPTH ; 
  set_table[2].name = "nr_lanes" ; set_table[2].var = &para->nr_lanes; 
  set_table[2].min = 1; set_table[2].max = MAX_LANES ; 
  set_table[3].name = "stype" ; set_table[3].var = &para->source_type; 
  set_table[3].min = 0; set_table[3].max = MAX_SOURCE_TYPE ; 
  set_table[4].name = "slength" ; set_table[4].var = &para->slength; 
  set_table[4].min = 1; set_table[4].max = MAX_SOURCE_LENGTH ; 
  set_table[5].name = "sinterval" ; set_table[5].var = &para->sinterval; 
  set_table[5].min = 1; set_table[5].max = MAX_SOURCE_INTERVAL ; 
  set_table[6].name = "ssat" ; set_table[6].var = &para->ssat; 
  set_table[6].min = 0; set_table[6].max = 1 ; 
  set_table[7].name = "chan_sched" ; set_table[7].var = &para->chan_sched; 
  set_table[7].min = 0; set_table[7].max = MAX_CHAN_SCHED ; 
  set_table[8].name = "nr_cycles" ; set_table[8].var = &nr_cycles; 
  set_table[8].min = 1; set_table[8].max = MAX_NR_CYCLES ; 

  set_table[9].name = "routing_scheme" ; 
  set_table[9].var = &para->routing_scheme ; 
  set_table[9].min = 1; set_table[9].max = 2 ; 
  set_table[10].name="chan_finding" ; 
  set_table[10].var=&para->chan_finding ; 
  set_table[10].min = 1; set_table[10].max = 2 ; 
  set_table[11].name="timeout_mode" ; 
  set_table[11].var=&para->timeout_mode ; 
  set_table[11].min = 0; set_table[11].max = 2 ; 
  set_table[12].name="timeout_counts" ; 
  set_table[12].var=&para->timeout_count ; 
  set_table[12].min = 0; set_table[12].max = 100 ; 
  set_table[13].name = "priority-1" ; 
  set_table[13].var = &para->chan_sel_priority_1 ; 
  set_table[13].min = 0; set_table[13].max = 4 ; 
  set_table[14].name = "priority-2" ; 
  set_table[14].var = &para->chan_sel_priority_2 ; 
  set_table[14].min = 0; set_table[14].max = 4 ; 
  set_table[15].name = "priority-3" ; 
  set_table[15].var = &para->chan_sel_priority_3 ; 
  set_table[15].min = 0; set_table[15].max = 4 ; 
  set_table[16].name = "priority-4" ; 
  set_table[16].var = &para->chan_sel_priority_4 ; 
  set_table[16].min = 0; set_table[16].max = 4 ; 
  set_table[17].name = (char *)0 ;
  set_table[17].var = (int *)0 ;
  set_table[17].min = 0; set_table[17].max = 0 ; 
}
/*---------------------------------------------------------------------------*/
int updated = 1 ;
/*---------------------------------------------------------------------------*/
void do_set_var(char *s, int value)
{
  int i ;
  SET_REC *rec ;
  
  for(i=0;set_table[i].name;i++) {
    rec = &(set_table[i]) ;
    if(!strcmp(rec->name,s)) {
      if((value >= rec->min) && (value <= rec->max)) {
	printf("SET: %s : %d -> %d\n",s,*(rec->var),value) ;
	*(rec->var) = value ;
	updated = 0 ;
      }
      else {
	printf("SET %s (currently %d) FAILED: %d is out of range [%d,%d]\n",
	       s, *(rec->var), value, rec->min, rec->max) ;
      }
      return ;
    }
  }
  printf("SET %s FAILED - no variable with this name\n",s) ;
}
/*---------------------------------------------------------------------------*/
void do_show_vars()
{
  int i ;
  SET_REC *rec ;

  printf("\nVARIABLES: (updated = %d)\n", updated) ;
  for(i=0;set_table[i].name;i++) {
    rec = &(set_table[i]) ;
    printf("    %20s = %5d range [%d,%d]\n", 
	   rec->name, *(rec->var), rec->min, rec->max) ;
  }
}
/*---------------------------------------------------------------------------*/
