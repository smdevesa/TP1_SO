COMPILER = gcc
CFLAGS = -Wall -pedantic -std=c99

all: master slave

master: master.o tad/slaveManagerADT.o
	$(COMPILER) $(CFLAGS) -o master master.o tad/slaveManagerADT.o

slave: slave.o
	$(COMPILER) $(CFLAGS) -o slave slave.o

master.o: master.c
	$(COMPILER) $(CFLAGS) -c master.c

slave.o: slave.c
	$(COMPILER) $(CFLAGS) -c slave.c

tad/slaveManagerADT.o: tad/slaveManagerADT.c
	$(COMPILER) $(CFLAGS) -c tad/slaveManagerADT.c -o tad/slaveManagerADT.o

clean:
	rm -f *.o master slave tad/*.o