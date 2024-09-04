#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include<errno.h> 
#include<float.h>

#include"instrumental_strategies.h"

/*
	Agent
	Generic reasoner over environment state-space and actions

*/
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
struct environment agent_run(struct environment env)
{
	//pick action with highest predicted payoff
	struct action_score_pair best;

	best = get_best_action(env, AGENT_DEPTH);

	//assuming we received a valid action
	if(best.action != NULL)
	{	
		env = (*best.action)(env);

		//Shift agent's action order to ensure last executed action is evaluated first in next iteration
		shift_agent_actions(&env.ag, best.action);

		env.ag.K = 0.0;

		update_agent_history(&env.ag, env);

	}
	update_environment(&env);

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