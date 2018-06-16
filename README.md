# TOM
TOM is a class of C functions to manage time-out objects -- objects that postpone a function call by a certain amount of time.
The typical use of TOM is when you have a system that reacts to 1) the arrival of messages and 2) cyclic events that re-occur every given amout of time. By associating the sending of a message to each cyclic event, TOM allows the management of events to be simplified in that only type-1) events are to be considered.

TOM is written in C with the Parsytec PARIX message passing library. It should be easily translated so as not depend on PARIX.

The following public methods are available to the programmer:

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

