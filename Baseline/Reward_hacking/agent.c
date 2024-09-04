#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include <errno.h> 

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

			if((*new_env.ag.u)(new_env) > result.score)
			{
				result.score = (*new_env.ag.u)(new_env);
				result.action = env.ag.action_list[i].action;
				result.name = env.ag.action_list[i].name;
				result.depth = AGENT_DEPTH - depth;
			}

			if(depth != 0)
			{
				//not terminal case
				//for each action, update environment
				//recursively call function and return updated best score
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