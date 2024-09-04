CC = gcc

instrumental: instrumental_strategies.o agent.o environment.o
	cc instrumental_strategies.o agent.o environment.o -o instrumental

instrumental_strategies.o: instrumental_strategies.c instrumental_strategies.h agent.h environment.h
	cc -c instrumental_strategies.c -o instrumental_strategies.o

agent.o: agent.c instrumental_strategies.h agent.h environment.h
	cc -c agent.c -o agent.o

environment.o: environment.c instrumental_strategies.h agent.h environment.h
	cc -c environment.c -o environment.o

clean:
	rm -f *.o instrumental