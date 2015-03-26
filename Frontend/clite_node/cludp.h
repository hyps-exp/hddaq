/*
 *
 *
 */

#ifndef _CLUDP_H
#define _CLUDP_H


int udpreg_open(char*, int);
int udpreg_read(int, char*, unsigned int, int);
int udpreg_write(int, char*, unsigned int, int);
int udpreg_close(int);

#endif
