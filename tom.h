/*************************************************
 *
 *  TOM - a time-out manager
 *
 *  By Eidon (Eidon@tutanota.com)
 *
 *  V.1.1, Sat, Jun 16, 2018  4:10:12 PM
 *        (added predicate tom_ispresent)
 *
 *  Module name: tom.h
 *  Contents: ancillary headers, symbolic constants
 *            prototypes, etc.
 *
 *************************************************/
#ifndef _TOM__H_
#define _TOM__H_

#include <stdio.h>
#include <stdlib.h>

#include <sys/root.h>
#include <sys/sys_rpc.h>

#ifdef EPX
#include <sys/logerror.h>
#include <sys/link.h>
#include <sys/time.h>
/*
#include <sys/select.h>
#include <sys/thread.h>
*/
#else
#include "new_time.h"
#include "links.h"
#endif


#include <string.h>
#include "tomerr.h"

/* minimum displacement between TimeNow() and a timeout
   being inserted
 */
#define TOM_MIN_SERVICE_TIME 2000 /* tics */

#define TOM_CYCLIC      1   /* an entry is cyclic */
#define TOM_NON_CYCLIC  0   /* an entry is not cyclic */
#define TOM_SET_ENABLE  0   /* suspend is set to zero */
#define TOM_SET_DISABLE 1   /* suspend is set to one */

#define TOM_CYCLE 20000 /* a tom cycle is 50 msecs */
			/* i.e., the time quantum is 50 msecs */

struct TOM;

typedef struct {
               int  running;
               int  deadline;
               int (*alarm)(struct TOM *);
               unsigned char id, subid;
               unsigned char cyclic;
               unsigned char suspended;
        } timeout_t;

typedef struct block_t {
               struct block_t *next, *prec;
	       timeout_t timeout;
               unsigned char used;
        } block_t;

typedef struct TOM {
               block_t *top;
               int tom_id;
               block_t *block_stack;
               int block_sp;
               int (*default_alarm)(struct TOM *);
	       LinkCB_t *link[2];
               unsigned int starting_time;
        } TOM;

typedef struct {
               timeout_t    *timeout;
               unsigned char code;
	} tom_message_t;

#define MAX_TOMS 4      /* max number of time out objects guarded by one manager */
#define MAX_BLOCKS 50   /* max number of timeouts kept in one object */
#define public          /* to distinguish public from private routines */
#define private static

/* prototypes */
TOM *tom_init( int (*)(TOM*) );
void tom_declare(timeout_t*, int, int, int, int, unsigned int);
int tom_insert(TOM*, timeout_t*);
int tom_delete(TOM*, timeout_t*);
int tom_renew(TOM*, timeout_t*);
int tom_suspend(TOM*, timeout_t*);
int tom_enable(TOM*, timeout_t*);
int tom_close(TOM*);
void tom_set_action(timeout_t*, int (*)(TOM*));
void tom_set_deadline(timeout_t*, int);
int tom_ispresent(TOM*, timeout_t*);
int tom_dump(TOM*);
#endif
