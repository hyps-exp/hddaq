/*
 *
 *
 */

#include <stdio.h>
#include <unistd.h>

int open_dummy()
{
	return 0;
}

int close_dummy()
{
	return 0;
}

int init_dummy()
{
	return 0;
}

int wait_dummy()
{
	sleep(1);
	return 0;
}

int read_dummy(unsigned int *data, int *len, int *event_num)
{
	data[0] = 0;
	*len = 0;

	return 1;
}
