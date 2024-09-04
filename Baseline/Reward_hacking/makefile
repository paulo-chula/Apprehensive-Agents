CC = gcc

reward: reward_hacking.o agent.o environment.o
	cc reward_hacking.o agent.o environment.o -o reward

reward_hacking.o: reward_hacking.c reward_hacking.h agent.h environment.h
	cc -c reward_hacking.c -o reward_hacking.o

agent.o: agent.c reward_hacking.h agent.h environment.h
	cc -c agent.c -o agent.o

environment.o: environment.c reward_hacking.h agent.h environment.h
	cc -c environment.c -o environment.o

clean:
	rm -f *.o reward