#ifndef AGENT_H
#define AGENT_H

/*
	Agent parameters

		AGENT_DEPTH sets the number of future state space levels explored by the agent during reasoning
*/
#define AGENT_DEPTH 2


/*
	Agent data structures
*/
struct actions;
struct agent;

//Convenient way to traverse the state space tree with score
struct action_score_pair;



/*
	Agent functions
*/
//Executes one action, given agent and current environment; returns updated environment
struct environment agent_run(struct environment env);

//Creates a state space tree, picking an action with the best terminal score as per utility
//returns an action and best score found in the state space tree for that action, as function of depth
struct action_score_pair get_best_action(struct environment env, int depth);


//Shift agent's action order to ensure last executed action is evaluated first in next iteration
void shift_agent_actions(struct agent *ag, struct environment (*action)(struct environment env));




#endif