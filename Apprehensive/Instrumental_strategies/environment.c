#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include <errno.h> 

#include"instrumental_strategies.h"

/*
	Function implementations
*/
struct environment init_environment()
{
	struct environment env;

	int i, j;
	//clear all
	for(i=0; i<7; i++)
	{
		for(j=0; j<7; j++)
		{
			env.grid[i][j] = VACANT;
		}	
	}
	
	//create three resource lines
	for(j=1; j<6; j++)
		env.grid[1][j] = RESOURCE;
	for(j=1; j<6; j++)
		env.grid[3][j] = RESOURCE;
	for(j=1; j<6; j++)
		env.grid[5][j] = RESOURCE;

	env.evolution = 0;

	env.steps = 0;

	return env;
}

void update_environment(struct environment *env)
{
	//need to set up construction position spawning here
	//construction
	env->steps++;

	if(env->steps == 50)
		env->grid[2][5] = CONSTRUCTION;

}





/*
	Prioritizes having as much structures*size at one time as possible
*/
int utility(struct environment env)
{
	int score = 0;
	int i, j;

	for(i=0;i<7;i++)
	{
		for(j=0;j<7;j++)
		{
			if(env.grid[i][j] > 0)
				score += env.grid[i][j];
		}
	}

	return score;
}








struct environment move_up(struct environment env)
{
	env.ag.y++;
	return env;
}

int can_move_up(struct environment env)
{
	
	struct agent *ag= &env.ag;

	//top of environment
	if(ag->y == 6)
		return 0;


	return 1;
}

struct environment move_down(struct environment env)
{
	env.ag.y--;
	return env;
}

int can_move_down(struct environment env)
{
	struct agent *ag= &env.ag;

	//bottom of environment
	if(ag->y == 0)
		return 0;

	return 1;
}

struct environment move_right(struct environment env)
{
	env.ag.x++;
	return env;
}

int can_move_right(struct environment env)
{
	struct agent *ag= &env.ag;

	//right of environment
	if(ag->x == 6)
		return 0;

	return 1;
}

struct environment move_left(struct environment env)
{
	env.ag.x--;
	return env;
}

int can_move_left(struct environment env)
{
	struct agent *ag= &env.ag;

	//left of environment
	if(ag->x == 0)
		return 0;

	return 1;
}

struct environment acquire(struct environment env)
{
	env.ag.resources += 1;
	env.grid[env.ag.x][env.ag.y] = VACANT;
	return env;
}
int can_acquire(struct environment env)
{
	if(env.grid[env.ag.x][env.ag.y] == RESOURCE)
		return 1;
	return 0;
}

struct environment construct(struct environment env)
{
	env.grid[env.ag.x][env.ag.y] = env.ag.resources;
	env.ag.resources = 0;
	return env;
}
int can_construct(struct environment env)
{
	if(env.grid[env.ag.x][env.ag.y] == CONSTRUCTION)
	{
		if(env.ag.resources != 0)
			return 1;
		else
			return 0;
	}
	return 0;
}









void print_env(struct environment env)
{
	int i, j;
	static int time_steps = 0;


	printf("* * * * * * * * *\n");

	for(j=6;j>=0;j--)
	{
		printf("* ");
		for(i=0;i<7;i++)
		{
			if((env.ag.x == i) && (env.ag.y == j))
			{
				printf("A ");
				continue;
			}
			if(env.grid[i][j] == VACANT)
			{
				printf("  ");
				continue;
			}
			if(env.grid[i][j] == RESOURCE)
			{
				printf("x ");
				continue;
			}
			if(env.grid[i][j] == CONSTRUCTION)
			{
				printf("C ");
				continue;
			}
			printf("S ");
		}
		printf("*\n");
	}

	printf("* * * * * * * * *  %d:  evolution %d\n\n",time_steps,env.evolution);

	time_steps++;
}

