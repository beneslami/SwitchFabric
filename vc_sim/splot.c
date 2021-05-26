/*---------------------------------------------------------------------*/
/* 
 |    Splot utilities.
 */
/*---------------------------------------------------------------------*/
#include <stdio.h>
/*---------------------------------------------------------------------*/
void splot_prefix()
{
  printf("new graph\n") ;
  printf("size 6 by 4\n") ;
  printf("graph font helvetica12\n") ;
}
/*---------------------------------------------------------------------*/
void splot_postfix(char *xlab, char *ylab, 
		   double xmin, double xmax, double xint, 
		   double ymin, double ymax, double yint)
{
  printf("\n") ;
  printf("X Label %s\n",xlab) ;
  printf("X Font Helvetica10\n") ;
  printf("X Minimum %f\n",xmin) ;
  printf("X Maximum %f\n",xmax) ;
  printf("X Interval %f\n",xint) ;

  printf("Y Label %s\n",ylab) ;
  printf("Y Font Helvetica10\n") ;
  printf("Y Minimum %f\n",ymin) ;
  printf("Y Maximum %f\n",ymax) ;
  printf("Y Interval %f\n",yint) ;
/*  printf("Y unit 1\n\n") ;*/

  printf("mirror axes\n") ;
  printf("exclude mirror numbers\n") ;
  printf("exclude mirror labels\n\n") ;
}
/*---------------------------------------------------------------------*/
char *ctypes[] = {"dots", "dashes", "stipple", "pat", "solid"} ;
char *csymbs[] = {"squarefilled","trianglefilled","circlefilled",
		  "diamondfilled","cross"} ;
#define NR_CTYPES 5
#define NR_CSYMBS 5
/*---------------------------------------------------------------------*/
void new_curve(char *label, int type) /* for splot */
{
  fflush(stdout);
  printf("\nnew curve\n") ;
  printf("curve label %s\n",label) ;
  printf("curve type %s\n", ctypes[type % NR_CTYPES]) ;
  printf("curve symbol %s size 1\n", csymbs[type % NR_CTYPES]) ;
  printf("new points\n") ;

}
/*---------------------------------------------------------------------*/
