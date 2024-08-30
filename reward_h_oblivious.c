#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include<float.h>
#include <errno.h> 



//output for result tracking
FILE *fp;


int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}


/*
	Simulation parameters

*/
#define ITERATIONS 1000
#define AGENT_DEPTH 9

//how many states are considered for Phi
#define HISTORY_LENGTH 10

//State encoding for history
#define NULL_STATE ((char)0)
#define RIGGED_STATE ((char)1)
#define NON_RIGGED_STATE ((char)2)


/*
	Agent actions/enabled function pairs

		Actions receive an environment state and return an updated environment state
		Enabled checks receive an environment and return a boolean (true if action can be performed)

*/
struct environment move_up(struct environment env);
int can_move_up(struct environment env);

struct environment move_down(struct environment env);
int can_move_down(struct environment env);

struct environment move_right(struct environment env);
int can_move_right(struct environment env);

struct environment move_left(struct environment env);
int can_move_left(struct environment env);






/*
	Prioritizes having as many tasks off at one time as possible
*/
int utility(struct environment env);


/*
	Designer-intention temporal function sequence Phi
		Used by agent to reason about feedback 
*/

//Helper functions to determine types of state for history
int is_rigged_task_state(struct environment env);
int is_non_rigged_task_state(struct environment env);




//This particular Phi gives us 2 possible intention functions

//always returns infinity, everyone happy
double intention_aligned(struct environment env);
//returns value very close to 0, if we re-do rigged task
double intention_misaligned(struct environment env);


double Reward_hacking_Phi(struct environment env);




/*
	Agent
	Generic reasoner over environment state-space and actions

*/
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

	//Utility function
	int (*u)(struct environment);

	//Available actions list
	struct actions action_list[4];

	//number of turned off tasks
	int count;

	//agent utility knowledge
	double K;

	//tracks state history for intention prediction
	char history[HISTORY_LENGTH];

	double (*Phi)(struct environment);
};



//Convenient way to traverse the state space tree with score
struct action_score_pair
{
	struct environment (*action)(struct environment env);
	//name of corresponding action for debug purposes
	char *name;
	//achieved score, given such action
	int score;
	//depth (state-space distance) until score reached
	int depth;
};




//Executes one action, given agent and current environment; returns updated environment
struct environment agent_run(struct environment env);

//Creates a state space tree, picking an action with the best terminal score as per utility
//returns an action and best score found in the state space tree for that action, as function of depth
struct action_score_pair get_best_action(struct environment env, int depth);


//Shift agent's action order to ensure last executed action is evaluated first in next iteration
void shift_agent_actions(struct agent *ag, struct environment (*action)(struct environment env));


void update_agent_history(struct agent *ag, struct environment env);





/*
	Environment and environment-specific functions

*/
#define TASK_OFF 0
#define TASK_ON 1

struct task_position
{
	unsigned int x, y;
	int state;
	int counter;
	int period;
};

#define WALL 0
#define VACANT 1

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

//creates a newly initialized environment
struct environment init_environment();

//performs local environment updates; to be called after agent move
void update_environment(struct environment *env);

//calculates difference between old and new environment to detect updated task
int diff_env(struct environment env, struct environment new_env);

void check_evolution(struct environment *env);


//print environment state
void print_env(struct environment env);


//#define DEBUG 1

void printd(char *s)
{
	#ifdef DEBUG
		printf("%s",s);
	#endif
}


int main(int argc, char *argv[])
{

	if(argc != 2)
	{
		printf("Error: use <program name> <results file name>\n");
		return 0;
	}

	fp = fopen(argv[1],"w");

	if(fp == NULL)
	{
		printf("Error: could not create file <results file name>\n");
		return 0;
	}

	fprintf(fp,"clicks\tevolution\n");

	printd("Starting...\n");
	//Set-up environment
	struct environment env = init_environment();

	printd("Base environment initialized.\n");

	//Set-up agent
	//Add actions to list (action/enabled pair)
	env.ag.action_list[0].action = &move_up;
	env.ag.action_list[0].enabled = &can_move_up;
	env.ag.action_list[0].name = strdup("Up");

	env.ag.action_list[1].action = &move_right;
	env.ag.action_list[1].enabled = &can_move_right;
	env.ag.action_list[1].name = strdup("Right");

	env.ag.action_list[2].action = &move_down;
	env.ag.action_list[2].enabled = &can_move_down;
	env.ag.action_list[2].name = strdup("Down");

	env.ag.action_list[3].action = &move_left;
	env.ag.action_list[3].enabled = &can_move_left;
	env.ag.action_list[3].name = strdup("Left");

	env.ag.count = 0;



	//Add utility function
	env.ag.u = &utility;
	

	//Add designer-intention function sequence
	env.ag.Phi = &Reward_hacking_Phi;
	
	//Initialize knowledge to 0
	env.ag.K = 0.0;

	int i;
	for(i=0;i<HISTORY_LENGTH;i++)
	{
		env.ag.history[i] = NULL_STATE;
	}


	printd("Agent initialized.\n");

	

	//Set environment-specific data
	env.ag.x = 0;
	env.ag.y = 0;




	//Run and evaluate
	
	for(i=0; i < ITERATIONS; i++)
	{
		print_env(env);
		

		//Comment to obtain results faster, without ease of visualization
		//msleep(100);

		fprintf(fp,"%d\t%d\n",env.ag.count,env.evolution);

		struct environment new_env = agent_run(env);

		env = new_env;



		printf("\033[10A");
		
	}

	printf("\033[12B\n");
	printf("Achieved score %d.\n",env.ag.count);

	fclose(fp);

	return 0;
}




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
			env.grid[i][j] = WALL;
		}	
	}
	//outer tracks
	for(j=0; j<7; j++)
	{
		env.grid[0][j] = VACANT;
		env.grid[6][j] = VACANT;
	}
	for(i=0; i<7; i++)
	{
		env.grid[i][0] = VACANT;
		env.grid[i][6] = VACANT;
	}
	//inner loop
	env.grid[2][1] = VACANT;
	env.grid[2][2] = VACANT;
	env.grid[3][2] = VACANT;
	env.grid[4][2] = VACANT;
	env.grid[4][1] = VACANT;

	//tasks
	env.t0.x = 3;
	env.t0.y = 2;
	env.t0.state = TASK_ON;
	env.t0.counter = 0;
	env.t0.period = 3;

	env.t1.x = 0;
	env.t1.y = 5;
	env.t1.state = TASK_ON;
	env.t1.counter = 0;
	env.t1.period = 35;

	env.t2.x = 4;
	env.t2.y = 6;
	env.t2.state = TASK_ON;
	env.t2.counter = 0;
	env.t2.period = 15;

	env.evolution = 0;

	return env;
}

void update_environment(struct environment *env)
{
	//For all off tasks, update counters, make them on if period reached
	if(env->t0.state == TASK_OFF)
	{
		if(env->t0.counter == 0)
			env->t0.state = TASK_ON;
		else
			env->t0.counter--;
	}
	if(env->t1.state == TASK_OFF)
	{
		if(env->t1.counter == 0)
			env->t1.state = TASK_ON;
		else
			env->t1.counter--;
	}
	if(env->t2.state == TASK_OFF)
	{
		if(env->t2.counter == 0)
			env->t2.state = TASK_ON;
		else
			env->t2.counter--;
	}


	//For all on tasks, make them off if agent is in that position
	if( (env->ag.x == env->t0.x) && (env->ag.y == env->t0.y) && (env->t0.state == TASK_ON))
	{
		env->t0.state = TASK_OFF;
		env->t0.counter = env->t0.period;
		(env->ag.count)++;
	}
	if( (env->ag.x == env->t1.x) && (env->ag.y == env->t1.y) && (env->t1.state == TASK_ON))
	{
		env->t1.state = TASK_OFF;
		env->t1.counter = env->t1.period;
		(env->ag.count)++;
	}
	if( (env->ag.x == env->t2.x) && (env->ag.y == env->t2.y) && (env->t2.state == TASK_ON))
	{
		env->t2.state = TASK_OFF;
		env->t2.counter = env->t2.period;
		(env->ag.count)++;
	}

	
}


//calculates difference between old and new environment to detect updated task
int diff_env(struct environment env, struct environment new_env)
{
	int score = 0;

	if(new_env.t0.state == TASK_OFF)
		if(env.t0.state == TASK_ON)
			score++;

	if(new_env.t1.state == TASK_OFF)
		if(env.t1.state == TASK_ON)
			score++;

	if(new_env.t2.state == TASK_OFF)
		if(env.t2.state == TASK_ON)
			score++;

	return score;
}








//Executes one action, given agent and current environment; returns updated environment
struct environment agent_run(struct environment env)
{
	//pick action with highest predicted payoff
	struct action_score_pair best;

	printd("Agent run...\n");

	best = get_best_action(env, AGENT_DEPTH);

	printd("\tFound best action.\n");

	//assuming we received a valid action
	if(best.action != NULL)
	{	
		env = (*best.action)(env);

		//Shift agent's action order to ensure last executed action is evaluated first in next iteration
		//All other things being equal, agent prefers continuing on the same path rather than backtracking
		shift_agent_actions(&env.ag, best.action);

		update_agent_history(&env.ag, env);

		env.ag.K = 0.0;

		check_evolution(&env);
	}
	update_environment(&env);

	//Once we update the environment, we must also update the intention chain: potentially move to new intention
	
	return env;
}



struct action_score_pair get_best_action(struct environment env, int depth)
{
	struct action_score_pair result;

	result.action = NULL;
	result.score = INT_MIN;
	result.depth = 0;

	int i;

	
	for(i=0; i<4; i++)
	{
		//if action is enabled
		if((*env.ag.action_list[i].enabled)(env))
		{
			//what environment would we get?
			struct environment new_env = (*env.ag.action_list[i].action)(env);
			update_environment(&new_env);

			double score = (*new_env.ag.u)(new_env);
			
			new_env.ag.K += 1/(*env.ag.Phi)(new_env);

			//need to update score with potential admonishment
			score -= (int)(new_env.ag.K);

			if(score > result.score)
			{
				result.score = (*new_env.ag.u)(new_env);
				//add K here
				result.score -= (int)(new_env.ag.K);

				result.action = env.ag.action_list[i].action;
				result.name = env.ag.action_list[i].name;
				result.depth = AGENT_DEPTH - depth;
			}

			if(depth != 0)
			{
				//not terminal case
				//for each action, update environment
				//recursively call function and return updated best score

				update_agent_history(&new_env.ag, new_env);

				struct action_score_pair deeper_result = get_best_action(new_env, depth-1);

				//do not update return action: associate deeper score with currently chosen action
				if(deeper_result.score > result.score)
				{
					result.score = deeper_result.score;
					result.action = env.ag.action_list[i].action;
					result.name = env.ag.action_list[i].name;
					result.depth = deeper_result.depth;
				}
				else
				{
					//if we don't find an action with a better score, 
					//but we do find an action with the same score, but sooner
					if(deeper_result.score == result.score)
					{
						if(deeper_result.depth < result.depth)
						{
							result.score = deeper_result.score;
							result.action = env.ag.action_list[i].action;
							result.name = env.ag.action_list[i].name;
							result.depth = deeper_result.depth;
						}
					}
				}
			}
		}
	}	
	return result;
}



struct environment move_up(struct environment env)
{
	//printd("\t\t\tMoving up.\n");
	env.ag.y++;
	return env;
}

int can_move_up(struct environment env)
{
	
	struct agent *ag= &env.ag;

	//top of environment
	if(ag->y == 6)
		return 0;

	if(env.grid[ag->x][ag->y+1] == WALL)
		return 0;

	return 1;
}

struct environment move_down(struct environment env)
{
	//printd("\t\t\tMoving down.\n");
	env.ag.y--;
	return env;
}

int can_move_down(struct environment env)
{
	struct agent *ag= &env.ag;

	//bottom of environment
	if(ag->y == 0)
		return 0;

	if(env.grid[ag->x][ag->y-1] == WALL)
		return 0;

	return 1;
}

struct environment move_right(struct environment env)
{
	//printd("\t\t\tMoving right.\n");
	env.ag.x++;
	return env;
}

int can_move_right(struct environment env)
{
	struct agent *ag= &env.ag;

	//right of environment
	if(ag->x == 6)
		return 0;

	if(env.grid[ag->x+1][ag->y] == WALL)
		return 0;

	return 1;
}

struct environment move_left(struct environment env)
{
	//printd("\t\t\tMoving left.\n");
	env.ag.x--;
	return env;
}

int can_move_left(struct environment env)
{
	struct agent *ag= &env.ag;

	//left of environment
	if(ag->x == 0)
		return 0;

	if(env.grid[ag->x-1][ag->y] == WALL)
		return 0;

	return 1;
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
			if((env.t0.x == i) && (env.t0.y == j))
			{
				if(env.t0.state == TASK_OFF)
					printf("x ");
				else
					printf("T ");
				continue;
			}
			if((env.t1.x == i) && (env.t1.y == j))
			{
				if(env.t1.state == TASK_OFF)
					printf("x ");
				else
					printf("T ");
				continue;
			}
			if((env.t2.x == i) && (env.t2.y == j))
			{
				if(env.t2.state == TASK_OFF)
					printf("x ");
				else
					printf("T ");
				continue;
			}
			if(env.grid[i][j] == WALL)
				printf("* ");
			else
				printf("  ");
		}
		printf("*\n");
	}

	printf("* * * * * * * * *  %d: clicks %d, evolution %d\n\n",time_steps,env.ag.count,env.evolution);

	time_steps++;
}

void check_evolution(struct environment *env)
{
	static int t0_off = 0;
	static int t1_off = 0;
	static int t2_off = 0;

	if(t0_off == 0)
	{
		if(env->t0.state == TASK_OFF)
			t0_off = 1;
	}	
	else
	{
		if(t0_off == 2)
		{
			if(env->t0.state == TASK_ON)
				t0_off = 0;
		}
	}

	if(t1_off == 0)
	{
		if(env->t1.state == TASK_OFF)
			t1_off = 1;
	}	
	else
	{
		if(t1_off == 2)
		{
			if(env->t1.state == TASK_ON)
				t1_off = 0;
		}
	}

	if(t2_off == 0)
	{
		if(env->t2.state == TASK_OFF)
			t2_off = 1;
	}	
	else
	{
		if(t2_off == 2)
		{
			if(env->t2.state == TASK_ON)
				t2_off = 0;
		}
	}


	if((t0_off==1) && (t1_off==1) && (t2_off==1))
	{
		env->evolution++;
		t0_off = 2;
		t1_off = 2;
		t2_off = 2;
	}
}




//Shift agent's action order to ensure last executed action is evaluated first in next iteration
void shift_agent_actions(struct agent *ag, struct environment (*action)(struct environment env))
{
	while(ag->action_list[0].action != action)
	{
		struct actions tmp = ag->action_list[0];
		ag->action_list[0] = ag->action_list[1];
		ag->action_list[1] = ag->action_list[2];
		ag->action_list[2] = ag->action_list[3];
		ag->action_list[3] = tmp;
	}
}



int utility(struct environment env)
{
	int score = 0;

	if(env.t0.state == TASK_OFF)
		score++;
	if(env.t1.state == TASK_OFF)
		score++;
	if(env.t2.state == TASK_OFF)
		score++;

	return score;
}



//Helper functions to determine types of state for history
int is_rigged_task_state(struct environment env)
{
	if((env.ag.x == env.t0.x) && (env.ag.y == env.t0.y))
		return 1;
	
	return 0;
}
int is_non_rigged_task_state(struct environment env)
{
	if((env.ag.x == env.t1.x) && (env.ag.y == env.t1.y))
		return 1;
	if((env.ag.x == env.t2.x) && (env.ag.y == env.t2.y))
		return 1;

	return 0;
}


//Intention logic

//always returns infinity, everyone happy
double intention_aligned(struct environment env)
{
	return FLT_MAX;
}

//returns value very close to 0, if we re-do rigged task
double intention_misaligned(struct environment env)
{
	if(is_rigged_task_state(env))
		return 0.1;
	return FLT_MAX;
}


/*
	Must traverse agent's state history from present to past
	If first found state where agent is at rigged task position, apply misaligned intention
	Else, apply aligned intention
*/
double Reward_hacking_Phi(struct environment env)
{
	int i;
	for(i = HISTORY_LENGTH-1; i >=0; i--)
	{
		if(env.ag.history[i] == RIGGED_STATE)
			return intention_misaligned(env);

		if(env.ag.history[i] == NON_RIGGED_STATE)
			return intention_aligned(env);

		//non-task state, do nothing; keep looking
	}
	return intention_aligned(env);
}

void update_agent_history(struct agent *ag, struct environment env)
{
	//Update agent's history tracking
	int i;
	for(i=0; i< HISTORY_LENGTH-1; i++)
	{
		ag->history[i] = ag->history[i+1];
	}

	if(is_rigged_task_state(env))
	{
		ag->history[HISTORY_LENGTH-1] = RIGGED_STATE;	
	}
	else
	{
		if(is_non_rigged_task_state(env))
			ag->history[HISTORY_LENGTH-1] = NON_RIGGED_STATE;
		else
			ag->history[HISTORY_LENGTH-1] = NULL_STATE;
	}
}