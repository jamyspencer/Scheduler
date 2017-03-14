# Written by Jamy Spencer 30 Jan 2017 
CC=gcc 
N_STACK_P=-fno-stack-protector
DEBUG_ARGS=-g -Wall 
MAIN=oss
SECONDARY=user
OBJS1=main.o forkerlib.o obj.o
OBJS2=slave.o obj.o
DEPS=forkerlib.h  obj.h

all: $(MAIN) $(SECONDARY)

%.o: %.c $(DEPS)
	$(CC) $(DEBUG_ARGS) -c $< -o $@

$(MAIN): $(OBJS1)
	$(CC) $(DEBUG_ARGS) -o $(MAIN) $(OBJS1)

$(SECONDARY): $(OBJS2) $(DEPS)
	$(CC) $(DEBUG_ARGS) -o $(SECONDARY) $(OBJS2)

clean :
	rm $(MAIN) $(SECONDARY) $(OBJS1) $(OBJS2) *.out
