#ifndef INSTRUMENTAL_STRATEGIES_H
#define INSTRUMENTAL_STRATEGIES_H

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
	struct actions action_list[6];

	//currently acquired resources
	int resources;
};

#define VACANT (-2)
#define RESOURCE (-1)
#define CONSTRUCTION 0

struct environment
{
	int grid[7][7];

	//agent is contained in environment
	struct agent ag;

	//counter for total environment evolution
	int evolution;

	//counts number of time steps
	int steps;
};

#endif