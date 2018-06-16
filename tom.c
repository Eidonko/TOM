/*************************************************
 *
 *  TOM - a time-out manager
 *
 *  By Eidon (Eidon@tutanota.com)                                               *
 *
 *  V.1.3, Sat, Jun 16, 2018  4:11:38 PM
 *
 *  Module name: tom.c
 *  Contents: initialisation routine, TOM thread,
 *            private time-out management routines.
 *
 *************************************************/
#include <stdio.h>
#include "tom.h"

#ifdef EPX1_2
unsigned int TimeNow(void);
int LocalLink (LinkCB_t *[2]);
#endif


static TOM toms[MAX_TOMS];
static block_t blocks[MAX_TOMS][MAX_BLOCKS];


static int id;
private int private_tom_card(void) { return id; }
private int private_tom_inc(void) { return ++id; }
int tom_thread(void); /* had to remove `private' because of TEX... */

private int private_tom_pop(TOM*);
private int private_tom_enable(TOM*, timeout_t*);
private int private_tom_suspend(TOM*, timeout_t*);
private int private_tom_insert(TOM*, timeout_t*);
private int private_tom_delete(TOM*, timeout_t*);
private int private_tom_renew(TOM*, timeout_t*);
private int private_tom_close(TOM*);
private int private_tom_purge(TOM*);
private char *tom_print_code(int);
#ifdef DEBUG
char *DIRPrintCode(int);
#endif


static void PushAlarm(block_t*);
static block_t *PopAlarm(void);
static void RunAlarm(TOM*);
void BackDeallocate(block_t*);
void RenewCyclics(TOM*);

extern char *DIRPrintTimeout(int);

static int spawned;
extern int send_timeout_message(TOM*);
extern int IAlarm(TOM*);

#include <sys/sem.h>
Semaphore_t sem, *tom_sem = &sem;

public TOM *tom_init( int (*alarm_function)(TOM*) )
{
	TOM* tom;
	int i;
	int errors;
	int id;
	static first_time;

	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_init", "Wait... (ft==%d)",first_time);
	Wait (tom_sem);
	private_tom_inc();
	id = private_tom_card() - 1;

#ifdef EPX
	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_init", "starting (%d)... (alarm==%x)", id, alarm_function);
#endif

	Signal(tom_sem);
	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_init", "...Signal.");

	if (id >= MAX_TOMS) return NULL;

	tom = &toms[ id ];
	tom->default_alarm = (int(*)(struct TOM*)) alarm_function;  /* function to be called when a time-out occurs */
	tom->tom_id = id;
	tom->block_stack = blocks[id];
	tom->block_sp = 0;
	tom->top = NULL;
	LocalLink(tom->link);

	for (i=0; i<MAX_BLOCKS; i++)
		tom->block_stack[i].used = 0;

	if ( id == 0 && spawned == 0)
	{
		/* spawn a time-out manager thread */

		/*
		CreateThread(NULL, 0, tom_thread, &errors, NULL);
		*/
		StartThread(tom_thread, 65536, &errors, 0, NULL);
		spawned = 1;
#ifdef EPX
	/* if (GetRoot()==0) */
		LogError(EC_DEBUG, "tom_init", "tom_thread has been spawned.");
#endif
	}

	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_init", "ending (%d)...", id);
	return tom;
}

/* the time-out manager thread */
/* private */ int tom_thread()
{
	int i;
	int residual;
	tom_message_t message;
	Option_t option[2];
	int n, recv;
	block_t *b, *prec;

	spawned = 1;
	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_thread", "started.");

	while (1)
	{
		/* for each defined time-out object... */

		for (i=0; i<private_tom_card(); i++)
		{
		/*
		LogError(EC_DEBUG, "tom_thread", "checking tom %d", i);
		*/

			/* 1/2: Check whether a timeout is to be triggered */
			if (toms[i].top != NULL)
			{
				residual =  toms[i].top->timeout.running - (TimeNow() - toms[i].starting_time);

	/* if (GetRoot()==0) */
				if (residual < 0)
				{
				LogError(EC_DEBUG, "tom_thread",
				 "residual (%s, %d) == %d", 
				  DIRPrintTimeout(toms[i].top->timeout.id),
				  toms[i].top->timeout.subid,
				  residual);

					b = toms[i].top->next;
					prec = toms[i].top;
					while (residual<0 && b!= NULL)
					{
						PushAlarm(prec); /* push block "prec" on top of the alarm stack */
						residual += b->timeout.running;
						prec = b;
						b = b->next;
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"residual reduced to %d", residual);
					}

					/* not ( (residual<0) and   (b != NULL) )   ==
					   residual >= 0      or    b == NULL       which is verified when

					   (T^T):           (residual>=0) and (b == NULL)
					   (T^F):           (residual>=0) and (b != NULL)
					   (F^T):           (residual< 0) and (b == NULL)
					*/
					if (residual<0 && b==NULL)        /* F^T */
					{
					/* if (GetRoot()==0) */
					LogError(EC_DEBUG, "tom_thread",
					"1. Case residual<0 && b==NULL");
						PushAlarm(toms[i].top);
						RunAlarm(toms+i);
						toms[i].top->used = 0;
						toms[i].top = NULL;
						RenewCyclics(toms+i);
					}
					else
					if (residual>=0 && b==NULL)       /* T^T */
					{
					/* if (GetRoot()==0) */
					LogError(EC_DEBUG, "tom_thread",
					"2. Case residual>=0 (%d) && b==NULL", residual);
					/* if (GetRoot()==0) */
					LogError(EC_DEBUG, "tom_thread",
					"prec->timeout.running == %d", prec->timeout.running);
						residual -= prec->timeout.running; /* now residual<0 */
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"residual now is %d, should be less than 0.", residual);
						prec->timeout.running += residual + TimeNow() - toms[i].starting_time;

						/* PopAlarm(); */
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"About to run the alarms...");
						RunAlarm(toms+i); 
						BackDeallocate(prec->prec);
						toms[i].top = prec;
						prec->prec = NULL;
						RenewCyclics(toms+i);
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"Now one or more items should have been removed from the head of the list, check...");
				tom_dump(toms+i);
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"And now those who are cyclic should have been reentered, check...");
				tom_dump(toms+i);
					}
					else /* residual>=0 && b!=NULL */ /* T^F */
					{
					/* if (GetRoot()==0) */
					LogError(EC_DEBUG, "tom_thread",
					"3. Case residual>=0 (%d) && b!=NULL (in the middle)", residual);

					/* if (GetRoot()==0) */
					LogError(EC_DEBUG, "tom_thread",
					"prec->timeout.running == %d", prec->timeout.running);

						residual -= prec->timeout.running; /* now residual<0 */
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"residual now is %d, should be less than 0.", residual);
						prec->timeout.running += residual;
						toms[i].starting_time = TimeNow();
				/*		PopAlarm(); */
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"About to run the alarms...");
						RunAlarm(toms+i); 
						BackDeallocate(prec->prec);
						toms[i].top = prec;
						prec->prec = NULL;
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"Now one or more items should have been removed from the head of the list, check...");
				tom_dump(toms+i);
						RenewCyclics(toms+i);
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread",
				"And now those who are cyclic should have been reentered, check...");
				tom_dump(toms+i);
					}

				/* check again if an entry has become negative */
				/*
				residual =  toms[i].top->timeout.running - (TimeNow() - toms[i].starting_time);
				*/
				}

			}

		/*
		LogError(EC_DEBUG, "tom_thread", "checks for messages for tom %d", i);
		*/
			/* 2/2: Check whether there are incoming messages */
			option[0] = ReceiveOption(toms[i].link[0]);

#ifdef TOM_DISCRETE_TIME_STEPS
			option[1] = TimeAfterOption((time_t) TimeNow() + TOM_CYCLE);
			n = SelectList(2, option);
			if (n == 1)
				continue;
#else
			n = CondSelect(1, option[0]);
			if (n == -1)
				continue;
#endif

			if ( (recv = RecvLink(toms[i].link[0], (void*) &message, sizeof(message))) != 
			      sizeof(message) )
				return TOM_WRONG_SIZE;

#ifdef DEBUG
		LogError(EC_DEBUG, "tom_thread", 
		"incoming message for tom %d: %s(%s==%d,%d)",
		i, tom_print_code(message.code), 
		DIRPrintCode(message.timeout->id), 
		message.timeout->id,
		message.timeout->subid);
#endif
			/* there's a message for me */
			switch(message.code)
			{
				case TOM_INSERT:
					n=private_tom_insert(toms+i, message.timeout);
	/* if (GetRoot()==0) */
					LogError(EC_DEBUG, "tom_thread",
					"tom_insert ended");
					break;
				case TOM_DELETE:
					n=private_tom_delete(toms+i, message.timeout);
					break;
				case TOM_SUSPEND:
					n=private_tom_suspend(toms+i, message.timeout);
					break;
				case TOM_ENABLE:
					n=private_tom_enable(toms+i, message.timeout);
					break;
				case TOM_RENEW:
					n=private_tom_renew(toms+i, message.timeout);
					break;
				case TOM_CLOSE:
					n=private_tom_close(toms+i);
					break;
				case TOM_EXIT:
					return spawned = 0;
			}

			if (n<0)
			{
				/*
				fprintf(stderr, "error %d detected\n", n);
				*/
	/* if (GetRoot()==0) */
				LogError(EC_DEBUG, "tom_thread", "error %d detected", n);
			}
		}
	}
}

private block_t *newblock(TOM *tom)
{
	block_t *b;
	int i;

	if (tom->block_sp < MAX_BLOCKS -1)
	{
		b = & tom->block_stack[tom->block_sp++];
		b->used = 1;
		return b;
	}
	else
	for (i=0; i<MAX_BLOCKS; i++)
	{
		if (tom->block_stack[i].used == 0)
		{
			b = & tom->block_stack[i];
			b->used = 1;
			return b;
		}
	}
	return NULL;
}

private int private_tom_insert(TOM* tom, timeout_t *t)
{
	block_t *b, *prec, *temp;
	int residual;

	if (tom == NULL || t == NULL)
		return TOM_NULL_PTRS;

	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_insert", "starting to insert timeout %s, subid %d, deadline=%d", DIRPrintTimeout(t->id), t->subid, t->deadline);
	tom_dump(tom);

	/*
	if (t->deadline < TOM_MIN_SERVICE_TIME)
		return TOM_REFUSED;
	LogError(EC_DEBUG, "tom_insert", "is the timeout present??");
	*/

	if (tom_ispresent(tom, t))
		return TOM_ALREADY_PRESENT_ENTRY;

	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_insert", "timeout %s, subid %d, is not present", DIRPrintTimeout(t->id), t->subid);

#ifdef DEBUG
	printf("%d: about to insert a timeout (id==%s, subid==%d, deadline==%d)\n", 
		GetRoot(), 
		DIRPrintTimeout(t->id), t->subid, t->deadline);
#endif
	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_insert",
	"about to insert a timeout (id==%s, subid==%d, deadline==%d)", 
		DIRPrintTimeout(t->id), t->subid, t->deadline);

			/******************/
			/* 1. top is NULL */
			/******************/

	if (tom->top == NULL)
	{
		b = newblock(tom);
		/* every time we turn a NULL list into a
		   non-NULL one, starting_time is set to
		   the time now
		 */
		tom->starting_time = TimeNow();
		b->next = b->prec = NULL;
		b->used = 1;
	        memcpy((void*) &b->timeout, t, sizeof(timeout_t));
		/* running is initially set to deadline */
		b->timeout.running = ( tom->starting_time - TimeNow() ) + t->deadline;
#ifdef DEBUG
		printf("Tom-insert: added one entry at the top of the list");
#endif
		tom->top = b;

	/* if (GetRoot()==0) */
	LogError(EC_DEBUG, "tom_insert",
		"Tom-insert: added one entry at the top of the list");
		tom_dump(tom);

		return 0;
	}

			/**********************/
			/* 2. top is not NULL */
			/**********************/

	/* residual time = absolute time from the alarm - spent time */
	residual = tom->top->timeout.running - ( TimeNow() - tom->starting_time );

	/* the new block goes on top */
	if (t->deadline < residual)
	{
		temp = newblock(tom);

		temp->next = tom->top;
		tom->top->prec = temp;

		temp->used = 1;
	        memcpy((void*) &temp->timeout, t, sizeof(timeout_t));
		temp->timeout.running = t->deadline + TimeNow() - tom->starting_time;

		temp->next->timeout.running = residual - t->deadline;
		tom->top = temp;
#ifdef DEBUG
		printf("Tom-insert: added one entry AT THE TOP of the list\n");
		tom_dump(tom);
#endif
	/* if (GetRoot()==0) */
		LogError(EC_DEBUG, "tom_insert",
			"Tom-insert: added one entry AT THE TOP of the list");
		tom_dump(tom);
		return 0;
	}

	/* the new block has to be inserted after the top */
	/* let's find a proper place */

	/* residual is set to top's hoard */
	residual = ( tom->starting_time - TimeNow() ) + tom->top->timeout.running;
	/* at first, b is next to top */
	b = tom->top->next;
	prec = tom->top;

	while ( residual < t->deadline   &&   b != NULL )
	{
		residual += b->timeout.running;
		prec = b;
		b = b->next;
	}

	/* not ( (residual < t->deadline)  and  (b != NULL) ) :
	   not (residual < t->deadline)  or   not(b != NULL)  :
	   (residual >= t->deadline)  or  (b == NULL) :

	 1. (residual <  t->deadline) && (b == NULL)     i.e.,   .F.  .T.
	 2. (residual >= t->deadline) && (b == NULL)     i.e.,   .T.  .T.
	 3. (residual >= t->deadline) && (b != NULL)     i.e.,   .T.  .F.
	 */

	if  (residual < t->deadline   &&  b == NULL)           /* 1. */
	{
		/* the block goes at the end */
		temp = newblock(tom);
		prec->next = temp;
		temp->next = NULL;
		temp->prec = prec;

		temp->used = 1;
	        memcpy((void*) &temp->timeout, t, sizeof(timeout_t));
	/* if (GetRoot()==0) */
		LogError(EC_DEBUG, "tom_insert",
		 "id==%s, subid==%d. Deadline == %d, Residual == %d, D-R==%d",
		 DIRPrintTimeout(t->id), t->subid, t->deadline,
		 residual, t->deadline - residual);
		temp->timeout.running = t->deadline - residual;

	/* if (GetRoot()==0) */
		LogError(EC_DEBUG, "tom_insert",
			"Tom-insert: added one entry AT THE END (1) of the list");
		tom_dump(tom);
		return 0;
	}

	if  (residual >= t->deadline   &&  b == NULL)          /* 2. */
	{
		residual -= prec->timeout.running;
		b = prec; 
		prec = prec->prec;

		/* the block goes at the end */
		temp = newblock(tom);
		prec->next = temp;

		temp->next = b; 
		b->prec = temp;

		temp->prec = prec;

		temp->used = 1;
	        memcpy((void*) &temp->timeout, t, sizeof(timeout_t));

	/* if (GetRoot()==0) */
		LogError(EC_DEBUG, "tom_insert",
		  "id==%s, subid==%d. Deadline == %d, Residual == %d, D-R==%d",
		  DIRPrintTimeout(t->id), t->subid, t->deadline, residual,
		  t->deadline - residual);
		temp->timeout.running = t->deadline - residual;
		b->timeout.running -= temp->timeout.running;

	/* if (GetRoot()==0) */
		LogError(EC_DEBUG, "tom_insert",
			"Tom-insert: added one entry AT THE END BUT ONE entry of the list");
		tom_dump(tom);
		return 0;
	}

	/* 3. (residual >= t->deadline) && (b != NULL) */
	residual -= prec->timeout.running;
 b = prec; 
	prec = prec->prec;

	temp = newblock(tom);
	prec->next = temp;
	temp->next = b;
	b->prec = temp;
	temp->used = 1;
	temp->prec = prec;
	memcpy((void*) &temp->timeout, t, sizeof(timeout_t));
	LogError(EC_DEBUG, "tom_insert",
	 "id==%s, subid==%d. Deadline == %d, Residual == %d, D-R==%d",
	 DIRPrintTimeout(t->id), t->subid, t->deadline, residual, 
	 t->deadline - residual);
	temp->timeout.running = t->deadline - residual;

	b->timeout.running -= temp->timeout.running;


	LogError(EC_DEBUG, "tom_insert",
		"Tom-insert: added one entry IN THE MIDDLE of the list");
	tom_dump(tom);
	return 0;
}

private int private_tom_delete(TOM* tom, timeout_t *t)
{
	block_t *b, *prec;
	unsigned int residual;

	residual = TimeNow();

	if (tom == NULL || t == NULL)
		return TOM_NULL_PTRS;

	b = tom->top;
	prec = NULL;
	while ( b != NULL )
	{
		if (b->timeout.id == t->id &&
		    b->timeout.subid == t->subid)
			break;

		prec = b;
		b = b->next;
	}

	/* (1) we reached the end w/o finding that entry */
	if (b == NULL)
		return TOM_ENTRY_NOT_FOUND;


	/* (2) the matching block was at the very beginning of the list */
	if (prec == NULL)
	{
		b->used = 0;  /* mark the block as `free' */
		tom->top = b->next;  /* set the new top */

		/* check whether the list is empty */
		if (tom->top == NULL)
			return TOM_EMPTY_LIST;

		tom->top->prec = NULL;

		tom->top->timeout.running += b->timeout.running;

		/* if (temp < 0) tom->top->timeout.running = TimeNow() - tom->starting_time; */

		tom_dump(tom);
		/* tom->starting_time = TimeNow(); / * added */
		return 0;
	}

	/* (3) matching block was in the middle of the list */

	b->used = 0;  /* mark the block as `free' */
	prec->next = b->next; /* skip the block in between prec and next */

	/* now  the previous order, i.e., prec < b < next
	   has been turned into           prec < next
	   and b is the deleted entry. */

	/* is `next' the last one? Then that's all */
	if (b->next == NULL)
		return 0;

	b->next->prec = prec;

	/* if `next' is a valid entry, it must inherit
	   the possibly non-null hoard of its deleted ancestor
	 */
	b->next->timeout.running += b->timeout.running;

	return 0;
}

private int private_tom_renew(TOM* tom, timeout_t *t)
{
	       private_tom_delete(tom, t);
	return private_tom_insert(tom, t);
}

private int private_tom_close(TOM* tom)
{
	if (tom == NULL)
		return TOM_NULL_PTRS;

	while (tom->top != NULL)
	{
		tom->top->used = 0;
		tom->top = tom->top->next;
	}
	return 0;
}

private int private_tom_purge(TOM* tom)
{
	if (tom == NULL)
		return TOM_NULL_PTRS;

	if (tom->top == NULL)
		return 0;

	tom->top->used =0;
	tom->top = tom->top->next;

	tom->starting_time = TimeNow();
	return 0;
}

private int private_tom_pop(TOM* tom)
{
	block_t *b;
	unsigned int residual, cmp;

	residual = TimeNow();

	if (tom == NULL)
		return TOM_NULL_PTRS;

	b = tom->top;
	b->used = 0;  /* mark the block as `free' */
	tom->top = b->next;

	/* check whether the list is empty */
	if (tom->top == NULL)
		return TOM_EMPTY_LIST;

	cmp = b->timeout.deadline - TimeNow();
	if (cmp > 0)
	{
		/* in a sense, the new top `inherits' the hoard
		   of its direct ancestor */
		tom->top->timeout.deadline += cmp;
	}
	else
	{
		/* contrarywise, we take the time spent herein
		   into account and we subtract it from the
		   new top's deadline */
		tom->top->timeout.deadline -= ( TimeNow() - residual );
	}
	return 0;
}

private int private_tom_suspend(TOM* tom, timeout_t* t)
{
	block_t *b;

	b = tom->top;
	while ( b != NULL )
	{
		if (b->timeout.id == t->id &&
		    b->timeout.subid == t->subid)
			break;

		b = b->next;
	}

	/* we reached the end w/o finding that entry */
	if (b == NULL)
		return TOM_ENTRY_NOT_FOUND;

	/* we found the entry */
	b->timeout.suspended = 1;
	t->suspended = 1;
	return 0;
}
private int private_tom_enable(TOM* tom, timeout_t* t)
{
	block_t *b;

	b = tom->top;
	while ( b != NULL )
	{
		if (b->timeout.id == t->id &&
		    b->timeout.subid == t->subid)
			break;

		b = b->next;
	}

	/* we reached the end w/o finding that entry */
	if (b == NULL)
		return TOM_ENTRY_NOT_FOUND;

	/* we found the entry */
	b->timeout.suspended = 0;
	t->suspended = 0;
	t->running = t->deadline;
	private_tom_renew(tom, t);
	return 0;
}

int tom_dump(TOM* tom)
{
	block_t *b;
	int i;
	char s[1024];

	sprintf(s, "Dump: entry | cyclic |          id          | subid |  deadline\n");
	b = tom->top;
	i=0;
	while ( b != NULL )
	{
		sprintf(s+strlen(s), "%12d", i); /* entry */
		sprintf(s+strlen(s), " | %6s", b->timeout.cyclic? "yes":"no"); /* cyclic */
		sprintf(s+strlen(s), " | %20s", DIRPrintTimeout(b->timeout.id)); /* id */
		sprintf(s+strlen(s), " | %5d", b->timeout.subid); /* subid */
		if (i==0)
		sprintf(s+strlen(s), " | %d\n", (int) tom->starting_time + b->timeout.running - TimeNow()); /* running */
		else
		sprintf(s+strlen(s), " | %d\n", (int) b->timeout.running); /* running */

		b = b->next;
		i++;
	}
	if (i==0)
		sprintf(s+strlen(s), "<e m p t y>");
#ifdef EPX
	LogError(EC_ERROR, "dump", "%s", s);
#else
	printf("%s\n", s);
#endif
	return 0;
}
public int tom_ispresent(TOM* tom, timeout_t *t)
{
	block_t *b;

	if (tom == NULL || t == NULL)
		return TOM_NULL_PTRS;

	b = tom->top;
	while ( b != NULL )
	{
		if (b->timeout.id == t->id &&
		    b->timeout.subid == t->subid)
			return 1;

		b = b->next;
	}

	/* we reached the end w/o finding that entry */
	return 0;
}
#ifdef EPX1_2
#include <sys/time.h>
unsigned int TimeNow(void) { return TimeNowHigh(); }
#endif
private char *tom_print_code(int code)
{
	switch(code)
	{
		case TOM_INSERT: return "TOM_INSERT";
		case TOM_DELETE: return "TOM_DELETE";
		case TOM_SUSPEND: return "TOM_SUSPEND";
		case TOM_ENABLE: return "TOM_ENABLE";
		case TOM_RENEW: return "TOM_RENEW";
		case TOM_CLOSE: return "TOM_CLOSE";
		case TOM_EXIT: return "TOM_EXIT";
		default: return "<unknown>";
	}
}

static block_t *bstack[MAX_BLOCKS];
static int     bsp;
/* push block on top of the alarm stack */
static void PushAlarm(block_t *block)
{
	bstack[bsp++] = block;
}
static block_t *PopAlarm()
{
/*
	if (bsp>0)
	LogError(EC_DEBUG, "PopAlarm", "entry %d popped off", bsp-1);
	else
	LogError(EC_DEBUG, "PopAlarm", "stack is over");
*/
	if (bsp>0) return bstack[--bsp];
	return NULL;
}
static timeout_t *tstack[MAX_BLOCKS];
static int     tsp;
/* push timeout on top of the timeout stack */
static void PushTimeout(timeout_t *t)
{
	LogError(EC_DEBUG, "PushTimeout", "entry %d (%s) pushed in",
	tsp, DIRPrintTimeout(t->id));
	tstack[tsp++] = t;
}
static timeout_t *PopTimeout()
{
	/*
	if (tsp>0)
	LogError(EC_DEBUG, "PopTimeout", "entry %d popped off", tsp-1);
	else
	LogError(EC_DEBUG, "PopTimeout", "stack is over");
	*/

	if (tsp>0) return tstack[--tsp];
	return NULL;
}
static void ZeroTimeOutStack()
{
	tsp = 0;
}
static void ZeroAlarmStack()
{
	bsp = 0;
}
/* this function runs the alarms kept in the alarm stack
   and stores those timeouts that are cyclic in the timeout stack
 */
static void RunAlarm(TOM* tom)
{
	block_t *b;

	ZeroTimeOutStack();

	/* for the time being, just run an alarm after the other;
	   in next version, clusterize 'n bufferize */
	while ( (b=PopAlarm()) != NULL )
	{
		if (! b->timeout.suspended)
			if (b->timeout.alarm != NULL)
				b->timeout.alarm((struct TOM*) tom);
			else
				tom->default_alarm((struct TOM*) tom);
		if (b->timeout.cyclic)
			PushTimeout(&b->timeout);
	}
}
void BackDeallocate(block_t *b)
{
	while (b != NULL)
	{
		b->used = 0;
		b = b->prec;
	}
}
void RenewCyclics(TOM* tom)
{
	timeout_t *t;

	while ( (t=PopTimeout()) != NULL )
	{
		private_tom_insert(tom, t);
	}
}
/* eof tom.c */
