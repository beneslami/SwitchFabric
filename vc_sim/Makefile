CC=gcc -g -O

SRCS = 	netsim.c \
	bfly.c \
	message.c \
	event.c \
	stats.c \
	source.c \
	set.c \
	cmds.c \
	mem.c \
	queue.c \
	splot.c

OBJS =	netsim.o \
	bfly.o \
	message.o \
	event.o \
	stats.o \
	source.o \
	set.o \
	cmds.o \
	mem.o \
	queue.o \
	splot.o

DEFS = 	defs.h \
	netsim.h \
	bfly.h \
	message.h \
	event.h \
	stats.h \
	source.h \
	queue.h \
	set.h \
	cmds.h \
	mem.h \
	Makefile

all:		netsim

netsim:		$(OBJS)
		$(CC) $(OBJS) -lm -o netsim

netsim.o:	netsim.c $(DEFS)
		$(CC) -c netsim.c

bfly.o:		bfly.c $(DEFS)
		$(CC) -c bfly.c

message.o:	message.c $(DEFS)
		$(CC) -c message.c

event.o:	event.c $(DEFS)
		$(CC) -c event.c

stats.o:	stats.c $(DEFS)
		$(CC) -c stats.c

source.o:	source.c $(DEFS)
		$(CC) -c source.c

queue.o:	queue.c $(DEFS)
		$(CC) -c queue.c

set.o:		set.c $(DEFS)
		$(CC) -c set.c

cmds.o:		cmds.c $(DEFS)
		$(CC) -c cmds.c

mem.o:		mem.c $(DEFS)
		$(CC) -c mem.c

splot.o:	splot.c $(DEFS)
		$(CC) -c splot.c
