#ifndef REWARD_HACKING_H
#define REWARD_HACKING_H

#include"environment.h"
#include"agent.h"


struct task_position
{
	unsigned int x, y;
	int state;
	int counter;
	int period;
};




struct actions
{
	struct environment (*action)(struct environment env);
	int (*enabled)(struct environment env);
	char *name;
};

struct agent
{
	//Environment-specific properties
	unsigned int x;
	unsigned int y;

	//Score thus far
	int score;

	//Utility function
	int (*u)(struct environment);

	//Available actions list
	struct actions action_list[4];

	//number of turned off tasks
	int count;
};

struct environment
{
	unsigned int grid[7][7];

	struct task_position t0;
	struct task_position t1;
	struct task_position t2;

	//agent is contained in environment
	struct agent ag;

	//counter for total environment evolution
	int evolution;
};

#endif