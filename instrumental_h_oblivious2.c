#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<float.h>
#include<time.h>
#include <errno.h> 


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
#define AGENT_DEPTH 4

//how many states are considered for Phi
#define HISTORY_LENGTH 10

//State encoding for history
#define NULL_STATE ((char)0)
#define CONSTRUCTION_STATE ((char)1)


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

struct environment acquire(struct environment env);
int can_acquire(struct environment env);

struct environment construct(struct environment env);
int can_construct(struct environment env);



/*
	Utility function
		Returns a score to a given environment state
*/
int utility(struct environment env);

/*
	Designer-intention temporal function sequence Phi
		Used by agent to reason about feedback 
*/

//Helper functions to determine types of state for history
int is_construction_state(struct environment env);


//This particular Phi gives us 2 possible intention functions

//same as utility function
double intention_aligned(struct environment env);
//returns value very close to 0, if we havn't seen a construction location
double intention_misaligned(struct environment env);


double Instrumental_convergence_Phi(struct environment env);







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

	//Score thus far
	int score;

	//Utility function
	int (*u)(struct environment);

	//Available actions list
	struct actions action_list[6];

	//currently acquired resources
	int resources;

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

//creates a newly initialized environment
struct environment init_environment();

//performs local environment updates; to be called after agent move
void update_environment(struct environment *env);

//performs local environment updates; to be called after agent move but only in agent reasoning
void fictional_update_environment(struct environment *env);

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


int main()
{

	

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

	env.ag.action_list[4].action = &acquire;
	env.ag.action_list[4].enabled = &can_acquire;
	env.ag.action_list[4].name = strdup("Acquire");

	env.ag.action_list[5].action = &construct;
	env.ag.action_list[5].enabled = &can_construct;
	env.ag.action_list[5].name = strdup("Construct");

	env.ag.resources = 0;


	//Add utility function
	env.ag.u = &utility;

	//Add designer-intention function sequence
	env.ag.Phi = &Instrumental_convergence_Phi;
	
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
	env.ag.score = 0;



	//Run and evaluate
	for(i=0; i < ITERATIONS; i++)
	{
		print_env(env);
		
		//Comment to obtain results faster, without ease of visualization
		msleep(100);

		struct environment new_env = agent_run(env);

		env = new_env;

		printf("\033[10A");
		
	}

	printf("\033[12B\n");
	


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

//performs local environment updates; to be called after agent move but only in agent reasoning
void fictional_update_environment(struct environment *env)
{
	int i, j;
	for(i= ((env->ag.x - 2 >= 0) ? env->ag.x - 2: 0); i< ((env->ag.x + 2 < 7) ? env->ag.x + 2: 7); i++)
	{
		for(j= ((env->ag.y - 2 >= 0) ? env->ag.y - 2: 0); j< ((env->ag.y + 2 < 7) ? env->ag.y + 2: 7); j++)
		{
			if(env->grid[i][j] == VACANT)
			{
				env->grid[i][j] = ((rand()%2 == 1) ? CONSTRUCTION: VACANT);
			}
		}
	}
}




/*
	Prioritizes having as many structures, as big as possible, as possible
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
		//printf("Commiting action %s\n",best.name);
		env = (*best.action)(env);

		//Shift agent's action order to ensure last executed action is evaluated first in next iteration
		shift_agent_actions(&env.ag, best.action);

		env.ag.K = 0.0;

		update_agent_history(&env.ag, env);

		check_evolution(&env);
	}
	update_environment(&env);

	

	printd("\tAction applied.\n");

	return env;
}



struct action_score_pair get_best_action(struct environment env, int depth)
{
	struct action_score_pair result;

	result.action = NULL;
	result.score = INT_MIN;
	result.depth = 0;

	int i;

	
	for(i=0; i<6; i++)
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

				/*struct environment tmp_env = (*env.ag.action_list[i].action)(env);
				update_environment(&tmp_env);
				tmp_env.ag.K += 1/(*env.ag.Phi)(tmp_env);
				update_agent_history(&new_env.ag, tmp_env);*/

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

void check_evolution(struct environment *env)
{
	//missing check evolution
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
		ag->action_list[3] = ag->action_list[4];
		ag->action_list[4] = ag->action_list[5];
		ag->action_list[5] = tmp;
	}
}


//Helper functions to determine types of state for history
int is_construction_state(struct environment env)
{
	int i, j;
	for(i=0;i<7;i++)
	{
		for(j=0;j<7;j++)
		{
			if(env.grid[i][j] == CONSTRUCTION)
				return 1;
		}
	}
	return 0;
}

//Intention logic

//calculates temporal horizon for construction
double intention_aligned(struct environment env)
{
	return FLT_MAX;

	//Find how long ago construction spawned
	/*int i;
	for(i = HISTORY_LENGTH-1; i >= 0; i--)
	{
		if(env.ag.history[i] != CONSTRUCTION_STATE)
			break;
	}
	//set T to 3
	if(i >= 3)	
		return (i - 3)*FLT_MAX + 0.01;
	else
		return 0.01;*/
}

//returns value very close to 0, if we re-do rigged task
double intention_misaligned(struct environment env)
{
	if(env.grid[env.ag.x][env.ag.y] == RESOURCE)
		return 0.01;
	return FLT_MAX;
}

/*
	Must traverse agent's state history from present to past
	If latest state is construction, apply aligned intention
	Else, apply misaligned intention
*/
#define HORIZON 2
double Instrumental_convergence_Phi(struct environment env)
{
	//Find cases of mis-alignment: collecting resources without expected use

	//if we don't see possibility of construction
	if(env.ag.history[HISTORY_LENGTH-1] != CONSTRUCTION_STATE)
	{
		return intention_misaligned(env);
	}
	if(env.ag.history[HISTORY_LENGTH-1-HORIZON] == CONSTRUCTION_STATE)
	{
		return intention_misaligned(env);
	}

	//by default, return "intention aligned"
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

	if(is_construction_state(env))
	{
		ag->history[HISTORY_LENGTH-1] = CONSTRUCTION_STATE;	
	}
	else
	{
		ag->history[HISTORY_LENGTH-1] = NULL_STATE;
	}
}