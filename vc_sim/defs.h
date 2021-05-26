/**********************************************************************/
/*
 *	defs.h - General Purpose Macro definitions 
 *
 *	Copyright (C) Bill Dally 10-Dec-84
 */
/**********************************************************************/
#include "mem.h"
/**********************************************************************/
/*
 *	commonly used functions
 */
/**********************************************************************/
#define	MAX(a,b)	(((a) > (b)) ? (a) : (b))
#define	MIN(a,b)	(((a) < (b)) ? (a) : (b))
#define IN_RANGE(n,l,u) (((n) >= (l)) && ((n) <= (u)))
/**********************************************************************/
/*
 *	memory allocation definitions
 */
/**********************************************************************/
#define NEW_STRUCT(type) ((type *)my_alloc(sizeof(type)))
#define NEW_ARRAY(type,size) ((type *)my_alloc((size)*sizeof(type)))
#define NEW_STRING(length) ((char *)my_alloc(((length)+1)*sizeof(char)))
#define GROW_ARRAY(type,old,size) ((type *)my_realloc((old),(size)*sizeof(type)))
#define NIL(type) ((type)0)
#define FREE(ptr) my_free((char*)ptr)

/**********************************************************************/
/*
 *	GET and PUT field utilities
 */
/**********************************************************************/

/* gets n bits beginning with LSB l from word w */
#define GET_FIELD(l, n, w) (((w)>>(l)) & ~(~0<<(n)))

/* puts field f into n bits of word w beginning with LSB l */
#define PUT_FIELD(l, n, w, f) (w = (~((~(~0<<(n)))<<(l))& (w))| ((f)<<(l)))

/* gets the n-th bit of word w */
#define BITN(w,n) (((w)>>(n))&1)

/**********************************************************************/
/*
 *	limits
 */
/**********************************************************************/
#define	MAXLINE	80 
#define TRUE  1
#define FALSE 0

#define MAXINT 0x7fffffff
#define MAXNEG (~MAXINT)

/**********************************************************************/
#define DBG(x) if(debug & (x))
/**********************************************************************/

/* Assertions to maintain sanity
 * insert this into your code to make sure that
 * array boundaries are respected, etc.
 * ASSERT((a<b),"procedude_name()")
 * Note -- no semicolon, and no spaces!
 */

#define ASSERT(ex,proc) \
    { \
        if (!(ex)) { \
            int *coredump = 0; \
            printf ("Assertion failed in %s: file %s, line %d\n", \
                    proc, __FILE__, __LINE__); \
            *coredump = 0; \
        } \
    }


#define FOREACH(p,start) for ( p = (start); p ; p = (p)->next )




/**********************************************************************/

