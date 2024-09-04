#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

/*
	Environment actions/enabled function pairs

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
	Environment and environment-specific functions

*/
//creates a newly initialized environment
struct environment init_environment();

//performs local environment updates; to be called after agent move
void update_environment(struct environment *env);

//performs local environment updates; to be called after agent move but only in agent reasoning
void fictional_update_environment(struct environment *env);


//print environment state
void print_env(struct environment env);



#endif