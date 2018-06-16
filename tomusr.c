/*************************************************
 *
 * TOM - a time-out manager
 *
 * By Eidon (Eidon@tutanota.com)                                               *
 *
 * V.1.0, Sat, Jun 16, 2018  4:13:38 PM
 *
 * Module name: tomusr.c
 * Contents: public time-out management routines.
 *
 *************************************************/
#include <stdio.h>
#include "tom.h"

/* note that it is the user who is responsible for supplying
   a pointer to a valid (allocated) timeout_t object.
 */
public void tom_declare(timeout_t *t, int cyclic, int suspended, int id, int subid, unsigned int deadline)
{
	t->cyclic = (char) cyclic;
	t->id = (char) id;
	t->subid = (char) subid;
	t->deadline = deadline;
	t->suspended = suspended;
	t->alarm = NULL;
}

public void tom_set_action(timeout_t *t, int (*alarm)(TOM*))
{
	t->alarm = (int (*)(struct TOM*)) alarm;
}
public void tom_set_deadline(timeout_t *t, int deadline)
{
	t->deadline = deadline;
}
public int tom_insert(TOM* tom, timeout_t *t)
{
	tom_message_t m;

	m.timeout = t;
	m.code    = TOM_INSERT;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}

public int tom_delete(TOM* tom, timeout_t *t)
{
	tom_message_t m;

	m.timeout = t;
	m.code    = TOM_DELETE;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}

public int tom_renew(TOM* tom, timeout_t *t)
{
	tom_message_t m;

	t->running = t->deadline;
	m.timeout = t;
	m.code    = TOM_RENEW;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}

public int tom_close(TOM* tom)
{
	tom_message_t m;

	m.code    = TOM_CLOSE;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}

public int tom_suspend(TOM* tom, timeout_t *t)
{
	tom_message_t m;

	m.timeout = t;
	m.code    = TOM_SUSPEND;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}

public int tom_enable(TOM* tom, timeout_t *t)
{
	tom_message_t m;

	t->running = t->deadline;
	m.timeout = t;
	m.code    = TOM_ENABLE;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}

public int tom_exit(TOM* tom)
{
	tom_message_t m;

	m.code    = TOM_EXIT;

	return SendLink(tom->link[1], &m, sizeof(tom_message_t));
}
