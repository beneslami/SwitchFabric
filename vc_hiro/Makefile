CC=gcc -g -O

SRCS = 	network.c \
	kary_ncube.c \
	mem.c \
	queue.c \
	source.c \
	event.c \
	message_route.c \
	routing_func.c \
	cmds.c \
	exper.c \
        stats.c \
	set.c

OBJS =	network.o \
	kary_ncube.o \
	mem.o \
	queue.o \
	source.o \
	event.o \
	message_route.o \
	routing_func.o \
	cmds.o \
	exper.o \
        stats.o \
	set.o

DEFS = 	defs.h \
	netdef.h \
	network.h \
	kary_ncube.h \
	queue.h \
	source.h \
	event.h \
	message_route.h \
	routing_func.h \
	cmds.h \
	exper.h \
        stats.h \
	set.h \
	Makefile		

all:		knetsim

knetsim:	$(OBJS)
		$(CC) $(OBJS) -lm -o knetsim

network.o:	network.c $(DEFS)
		$(CC) -c network.c

kary_ncube.o:	kary_ncube.c $(DEFS)
		$(CC) -c kary_ncube.c

queue.o:	queue.c $(DEFS)
		$(CC) -c queue.c

mem.o:		mem.c $(DEFS)
		$(CC) -c mem.c
	
source.o:	    source.c $(DEFS)
		    $(CC) -c source.c

event.o:	     event.c $(DEFS)
		     $(CC) -c event.c

message_route.o:     message_route.c $(DEFS)
		     $(CC) -c message_route.c

routing_func.o:      routing_func.c $(DEFS)
		     $(CC) -c routing_func.c 

cmds.o:	 	     cmds.c $(DEFS)
		     $(CC) -c cmds.c

exper.o:	     exper.c $(DEFS)
		     $(CC) -c exper.c

stats.o:             stats.c $(DEFS)
		     $(CC) -c stats.c

set.o: 	             set.c $(DEFS)
		     $(CC) -c set.c

