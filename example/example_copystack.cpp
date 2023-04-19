#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include "../rt.h"

void* RoutineFunction(void* args) {
	rt_enable_hook_sys();
	int* routineid = (int*)args;
	while (true)
	{
		char sBuff[128];
		sprintf(sBuff, "from routineid %d stack addr %p\n", *routineid, sBuff);

		printf("%s", sBuff);
		poll(NULL, 0, 1000); //sleep 1s
	}
	return NULL;
}

int main()
{
	ShareStack* share_stack = rt_alloc_share_stack(1, 1024 * 128);
	RoutineAttr attr;
	attr.stack_size = 0;
	attr.share_stack = share_stack;

	Routine* rt[2];
	int routineid[2];
	for (int i = 0; i < 2; i++)
	{
		routineid[i] = i;
		rt_create(&rt[i], &attr, RoutineFunction, routineid + i);
		rt_resume(rt[i]);
	}
	rt_eventloop(get_thread_eventloop(), NULL, NULL);
	return 0;
}
