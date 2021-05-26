/*---------------------------------------------------------------------------*/
/*
 |    network simulator - quick and dirty memory allocator
 |    (C) Bill Dally - 11-Nov-1989
 */
/*---------------------------------------------------------------------------*/
/* INCLUDES */
/*---------------------------------------------------------------------------*/
#include <sys/types.h>
/*---------------------------------------------------------------------------*/
/* VARS */
/*---------------------------------------------------------------------------*/
static char *free_mem_ptr ;
static int free_size ;
static int alloc_size ;
/*---------------------------------------------------------------------------*/
void init_mem(int initial_size)
{
  free_mem_ptr = (caddr_t)sbrk(initial_size) ;
  if(free_mem_ptr == (caddr_t)-1) 
    error("Unable to allocate initial memory\n") ;
  free_size = alloc_size = initial_size ;
  printf("\nMEMORY INITIALIZED TO %d\n",alloc_size) ;
}
/*---------------------------------------------------------------------------*/
static void get_more_memory()
{
  char *ptr ;
  ptr = (caddr_t)sbrk(alloc_size) ;
  if(ptr == (caddr_t)-1) error("Unable to grow memory\n") ;
  printf("\nGROWING MEMORY BY %d\n",alloc_size) ;
  free_size += alloc_size ;
  alloc_size *= 2 ;	/* get twice as much next time */
}
/*---------------------------------------------------------------------------*/
char *my_alloc(int size)
{
  char *ptr ;
  while(size > free_size) {
    get_more_memory() ;
  }
  ptr = free_mem_ptr ;
  free_mem_ptr += size ;
  free_size -= size ;
  return(ptr) ;
}
/*---------------------------------------------------------------------------*/
char *my_realloc(char *old, int size)
{
  int i ;
  char *new = my_alloc(size) ;
  for(i=0;i<size;i++) new[i] = old[i] ;
  return(new) ;
}
/*---------------------------------------------------------------------------*/
void *my_free(char *ptr)
{
  /* does nothing */
}
/*---------------------------------------------------------------------------*/
