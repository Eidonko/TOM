/*************************************************
 *
 * TOM - a time-out manager
 *
 * By Eidon (Eidon@tutanota.com)                                               *
 *
 * V.1.0, Sat, Jun 16, 2018  4:12:58 PM
 *
 * Module name: tomtest.c
 * Contents: a test routine.
 *
 *************************************************/
#include "tom.h"

#define MY_TIMEOUT_ID1          10
#define MY_TIMEOUT_SUBID1       1
#define MY_TIMEOUT_DEADLINE1    (int)(2.7*CLK_TCK_HIGH)

#define MY_TIMEOUT_ID2          20
#define MY_TIMEOUT_SUBID2       2
#define MY_TIMEOUT_DEADLINE2    (int)(5.9*CLK_TCK_HIGH)

#define MY_TIMEOUT_ID3          30
#define MY_TIMEOUT_SUBID3       3
#define MY_TIMEOUT_DEADLINE3    (int)(10*CLK_TCK_HIGH)

#define MY_TIMEOUT_ID4          40
#define MY_TIMEOUT_SUBID4       4
#define MY_TIMEOUT_DEADLINE4    (int)(20*CLK_TCK_HIGH)

#include <sys/sem.h>

main()
{
	TOM *tom;
	int alarm(struct TOM *);
	timeout_t t1, t2;
	timeout_t t3, t4;
	extern Semaphore_t sem, *tom_sem;

	if (GET_ROOT()->ProcRoot->MyProcID != 0)
		return;
	InitSem(tom_sem, 1);
	tom = tom_init( alarm );

	tom_declare(&t1, TOM_CYCLIC, TOM_SET_ENABLE, MY_TIMEOUT_ID1, MY_TIMEOUT_SUBID1, MY_TIMEOUT_DEADLINE1);
	tom_insert(tom, &t1);

	tom_declare(&t2, TOM_CYCLIC, TOM_SET_ENABLE, MY_TIMEOUT_ID2, MY_TIMEOUT_SUBID2, MY_TIMEOUT_DEADLINE2);
	tom_insert(tom, &t2);

	tom_declare(&t3, TOM_CYCLIC, TOM_SET_ENABLE, MY_TIMEOUT_ID3, MY_TIMEOUT_SUBID3, MY_TIMEOUT_DEADLINE3);
	tom_insert(tom, &t3);

	tom_declare(&t4, TOM_CYCLIC, TOM_SET_ENABLE, MY_TIMEOUT_ID4, MY_TIMEOUT_SUBID4, MY_TIMEOUT_DEADLINE4);
	tom_insert(tom, &t4);

	/*
	printf("suspending timeout 3\n");
	tom_suspend(tom, &t3);
	TimeWaitHigh(TimeNowHigh()+3*CLK_TCK_HIGH);
	printf("enabling timeout 3\n");
	tom_enable(tom, &t3);
	printf("suspending timeout 4\n");
	tom_suspend(tom, &t4);
	*/
	TimeWaitHigh(TimeNowHigh()+300*CLK_TCK_HIGH);
	printf("OK, closing...\n");
	tom_close(tom);
	tom_exit(tom);
}

int alarm(struct TOM *tom)
{
	static int i;
	unsigned char id;
	unsigned char subid;

	tom_dump(tom);
}
char *DIRPrintTimeout(int i)
{
	switch (i) {
	case MY_TIMEOUT_ID1 : return "Timeout 1";
	case MY_TIMEOUT_ID2 : return "Timeout 2";
	case MY_TIMEOUT_ID3 : return "Timeout 3";
	case MY_TIMEOUT_ID4 : return "Timeout 4";
	}
	return NULL;
}
GetRoot()
{
	return GET_ROOT()->ProcRoot->MyProcID;
}

