CFLAGS=-DTOM_DISCRETE_TIME_STEPS -DEPX -DEPX1_2

all:	tom.o tomusr.o tomtest.px

tom.o:	tom.c tom.h tomerr.h
	px ancc $(CFLAGS) -c tom.c

tomusr.o:	tomusr.c tom.h tomerr.h
	px ancc $(CFLAGS) -c tomusr.c

tomtest.px:	tom.o tomusr.o tomtest.c
	px ancc $(CFLAGS) -o tomtest.px tom.o tomusr.o tomtest.c

