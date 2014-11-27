/*
 *
 *
 */
#ifndef USERDEVICE_H
#define USERDEVICE_H

int get_maxdatasize();
int open_device();
int close_device();
int init_device();
int wait_device();
int read_device(unsigned int *, int *, int *, int run_num);
int finalize_device();

#endif
