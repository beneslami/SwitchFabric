/*---------------------------------------------------------------------------*/
/*
 |    network simulator - set variable utility
 |    (C) Bill Dally - 3-July-1989
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "defs.h"
#include "netsim.h"
/*---------------------------------------------------------------------------*/
struct set_rec {
  char *name ;			/* variable name */
  int  *var ;			/* pointer to integer variable */
  int  min ;			/* minimum value for var */
  int  max ;			/* maximum - cannot be set out of range */
};
typedef struct set_rec SET_REC ;
/*---------------------------------------------------------------------------*/
SET_REC set_table[] =
{
"debug", &debug, 0, 0xFFFF,
"depth", &depth, 1, MAX_DEPTH ,
"nr_lanes", &nr_lanes, 1, MAX_LANES ,
"stype", &stype, 0, MAX_SOURCE_TYPE ,
"slength", &slength, 1, MAX_SOURCE_LENGTH ,
"sinterval", &sinterval, 1, MAX_SOURCE_INTERVAL ,
"ssat", &ssat, 0, 1,
"spct", &spct, 0, 100,
"vstat", &vstat, 0, 1,
"chan_sched", &chan_sched, 0, MAX_CHAN_SCHED,
"nr_cycles", &nr_cycles, 1, MAX_NR_CYCLES,
"warm_cycles", &warm_cycles, 1, MAX_NR_CYCLES,
"unbuffered", &unbuffered, 1, MAX_NR_CYCLES,
"expsrc", &expsrc, 1, MAX_NR_CYCLES,
(char *)0,(int *)0, 0, 0
} ;
/*---------------------------------------------------------------------------*/
int updated = 1 ;
/*---------------------------------------------------------------------------*/
void do_set_var(char *s, int value)
{
  SET_REC *rec ;
  int i ;

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
  SET_REC *rec ;
  int i ;

  printf("\nVARIABLES: (updated = %d)\n", updated) ;
  for(i=0;set_table[i].name;i++) {
    rec = &(set_table[i]) ;
    printf("    %20s = %5d range [%d,%d]\n", 
	   rec->name, *(rec->var), rec->min, rec->max) ;
  }
}
/*---------------------------------------------------------------------------*/
