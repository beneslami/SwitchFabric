/*---------------------------------------------------------------------------*/
/* make a new butterfly given the log of the radix and the number of stages */
/*---------------------------------------------------------------------------*/
extern int k ;
extern int n ;
extern int height ;
/*---------------------------------------------------------------------------*/
extern void new_bfly(int lk, int n) ;	
extern int bfly_route(int source_node_nr, int in_channel, int dest) ;
extern int bfly_dest(int node_nr, int dest) ;
/*---------------------------------------------------------------------------*/

