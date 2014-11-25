/*
 *
 *
 */

#ifndef DUMMYDEVICE_
#define DUMMYDEVICE_

int open_dummy();
int close_dummy();
int init_dummy();
int wait_dummy();
int read_dummy(unsigned int *, int *, int *);

#endif
