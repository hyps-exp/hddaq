/*
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "finessetype.h"


#define FREGQ_DEPTH 128

struct finesse_reg {
  int slot;
  int address;
  int val;
};

static unsigned int finesse_act = 0xf;
static struct finesse_reg fregque[FREGQ_DEPTH];
static int fregq_lp = 0;
static int fregq_rp = 0;

static char cval[128];
static int finesse_type[4] = {0, 0, 0, 0};

static char param_udphost[256];
static char param_tcphost[256];
static int param_udpport;
static int param_tcpport;

int fregque_pushback(int slot, int address, int val)
{

  if (fregq_lp >= FREGQ_DEPTH) return 0;

  fregque[fregq_lp].slot = slot;
  fregque[fregq_lp].address = address;
  fregque[fregq_lp].val = val;
  fregq_lp++;

  return 1;
}

int fregque_get(int *slot, int *address, int *val)
{
  if (fregq_rp >= fregq_lp) {
    *address = 0;
    *val = 0;
    return 0;
  }

  *slot = fregque[fregq_rp].slot;
  *address = fregque[fregq_rp].address;
  *val = fregque[fregq_rp].val;
  fregq_rp++;

  return 1;
}

int fregque_top()
{
  fregq_rp = 0;
  return 0;
}

int fregque_print()
{
  int i=0;

  for (i = 0 ; i < fregq_lp ; i++) {
    printf("%3d: slot: %2x addr:%2x %x\n", i,
	   fregque[i].slot, fregque[i].address, fregque[i].val);
  }

  return 0;
}

char * get_paramfilename()
{
  /*char *hostname;*/
  char *homedir=NULL;
  static char lhostname[128];
  char *filename = NULL;
  static char filename1[128], filename2[128];


  if (gethostname(lhostname, 128) != 0) {
    perror("gethostname err.");
    return NULL;
  }

  if (strlen(lhostname) >= 128) {
    fprintf(stderr, "too long hostname\n");
    return NULL;
  }

  {
    unsigned int i;
    for (i = 0 ; i < strlen(lhostname) ; i++) {
      if (lhostname[i] == '.') {
	lhostname[i] = '\0';
	break;
      }
    }
  }


  homedir = getenv("HOME");
  /*
    if ((homedir == NULL) || (homedir[0] == '\0')) {
    fprintf(stderr, "enviroment err. HOME.\n");
    }
  */
  /*fprintf(stderr, "#D:%s!!\n", homedir);*/

  sprintf(filename1, "%s/.copperlite_%s", homedir, lhostname);
  sprintf(filename2, "/etc/copperlite/%s", lhostname);
  {
    struct stat fs;
    int status;
    status = stat(filename1, &fs);
    if ((status == 0) && S_ISREG(fs.st_mode)) {
      filename = filename1;
    } else {
      status = stat(filename2, &fs);
      if ((status == 0) && S_ISREG(fs.st_mode)) {
	filename = filename2;
      }
    }
    if (filename == NULL) {
      fprintf(stderr, "No initialize file!!\n");
    }
  }

	
  return filename;
}


/* int get_finesse_param() */
int get_finesse_param(char *filename)
{
  FILE *fd;
  /* char *filename; */
  char oneline[256];
  int slot, addr, val;
  char sval[256];

  param_udphost[0] = 0;
  param_tcphost[0] = 0;
  param_udpport = 0;
  param_tcpport = 0;

  /* filename = get_paramfilename(); */

  fd = fopen(filename, "r");
  if (fd == NULL) {
    fprintf(stderr, "open fail %s !\n", filename);
    return 0;
  }

  while (fgets(oneline, 255, fd) != NULL) {
    oneline[255] = 0;
    if (feof(fd)) break;
    if (oneline[0] != '#') {
      if (sscanf(oneline, "finesse_act %x", &val) == 1) {
	finesse_act = val;
	printf("finesse_act %x\n", val);
      }
      if (sscanf(oneline, "reg %x %x %x",
		 &slot, &addr, &val) == 3) {
	fregque_pushback(slot, addr, val);
	printf("reg %x %x %x\n", slot, addr, val);
      }
      if (sscanf(oneline, "finesse_type %x %s",
		 &slot, cval) == 2) {
	printf("finesse_type %x %s\n", slot, cval);
	if (strncmp(cval, "ENCODER", 128) == 0) {
	  int i;
	  for (i = 0 ; i < 4 ; i++) {
	    if (slot & (0x1 << i)) {
	      finesse_type[i] = FT_ENCODER;
	    }
	  }
	}
	else if (strncmp(cval, "32PTDC", 128) == 0) {
	  int i;
	  for (i = 0 ; i < 4 ; i++) {
	    if (slot & (0x1 << i)) {
	      finesse_type[i] = FT_32PTDC;
	    }
	  }
	}
      }
      if (sscanf(oneline, "udp_port %d", &val) == 1) {
	param_udpport = val;
      }
      if (sscanf(oneline, "tcp_port %d", &val) == 1) {
	param_tcpport = val;
      }
      if (sscanf(oneline, "udp_host %s", sval) == 1) {
	strncpy(param_udphost, sval, 256);
      }
      if (sscanf(oneline, "tcp_host %s", sval) == 1) {
	strncpy(param_tcphost, sval, 256);
      }
    }
  }

  fclose(fd);
	
  return 1;
}


unsigned int get_finesse_act()
{
  return finesse_act;
}

int* get_finesse_type()
{
  return finesse_type;
}

int get_udpport()
{
  return param_udpport;
}

int get_tcpport()
{
  return param_tcpport;
}

char* get_udphost()
{
  return param_udphost;
}

char* get_tcphost()
{
  return param_tcphost;
}


#ifdef TESTMAIN
int main(int argc, char* argv[])
{
  char *filename;
  int slot, addr, val;
  int *ftype;

  /* filename = get_paramfilename(); */
  filename = argv[1];
  printf("filename: %s\n", filename);
  get_finesse_param(filename);
  printf("finesse_act: %x\n", get_finesse_act());
  ftype = get_finesse_type();
  printf("finesset_type: %d %d %d %d\n",
	 ftype[0], ftype[1], ftype[2], ftype[3]);
  printf("udp: %s:%d\n", get_udphost(), get_udpport());
  printf("tcp: %s:%d\n", get_tcphost(), get_tcpport());
  fregque_print();

  while(fregque_get(&slot, &addr, &val)) {
    printf("reg: %x %x %x\n", slot, addr, val);
  }

  return 0;
}
#endif
