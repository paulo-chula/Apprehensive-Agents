#ifndef AGENT_H
#define AGENT_H

/*
	Agent parameters

		AGENT_DEPTH sets the number of future state space levels explored by the agent during reasoning
*/
#define AGENT_DEPTH 2

struct agent;

/*
	Phi process
*/
//how many states are considered for Phi
#define HISTORY_LENGTH 10

//State encoding for history
#define NULL_STATE ((char)0)
#define RIGGED_STATE ((char)1)
#define NON_RIGGED_STATE ((char)2)

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

void update_agent_history(struct agent *ag, struct environment env);




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