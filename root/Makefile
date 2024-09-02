CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

# Targets
TARGETS = master slave

# Object files
OBJS = validate.o

all: $(TARGETS)

master: master.o $(OBJS)
	$(CC) $(CFLAGS) -o master master.o $(OBJS)

slave: slave.o $(OBJS)
	$(CC) $(CFLAGS) -o slave slave.o $(OBJS)

validate.o: validate.c validate.h
	$(CC) $(CFLAGS) -c validate.c

master.o: master.c validate.h
	$(CC) $(CFLAGS) -c master.c

slave.o: slave.c validate.h
	$(CC) $(CFLAGS) -c slave.c

clean:
	rm -f $(TARGETS) *.o