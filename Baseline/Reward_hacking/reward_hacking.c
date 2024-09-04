#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>
#include<time.h>
#include <errno.h> 


#include"reward_hacking.h"
#include"agent.h"
#include"environment.h"




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




int main(int argc, char *argv[])
{
	//Set-up environment
	struct environment env = init_environment();


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


	//Set environment-specific data
	env.ag.x = 0;
	env.ag.y = 0;
	env.ag.score = 0;



	//Run and evaluate
	int i;
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
	//printf("Achieved score %d.\n",env.ag.score);



	return 0;
}



