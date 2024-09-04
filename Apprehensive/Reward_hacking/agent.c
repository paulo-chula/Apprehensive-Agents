#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include <errno.h> 
#include<float.h>

#include"reward_hacking.h"

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

		update_agent_history(&env.ag, env);

		env.ag.K = 0.0;

		check_evolution(&env);
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