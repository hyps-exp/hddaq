/*
 *
 *
 */

#ifndef _CLTDP_H
#define _CLTDP_H

int cltcp_open(char*, int);
int cltcp_read(int, char*, int);
int cltcp_write(int, char*, int);
int cltcp_close(int);
int cltcp_drop_event(int);

int cltcp_set_finesse_act(int);
int cltcp_get_finesse_act();
int cltcp_get_nfinesse();

#endif
