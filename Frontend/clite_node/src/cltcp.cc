/*
 *
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <assert.h>
#include <sys/time.h>

#include "MessageHelper.h"

#define CL_MAGIC 0x7FFF000A

struct cl_header {
  unsigned int magic;
  unsigned int key;
  unsigned int length;
  unsigned int id;
};
static int nfinesse     = 4;
static int finesse_act  = 0xf;
static int is_bigendian = 1;
static int is_checkid   = 1;

int rev_byteorder(char *buf, int len)
{
  unsigned int i = 0;
  unsigned int *data = NULL;

  data = (unsigned int *)buf;

  for (i = 0 ; i < len/sizeof(unsigned int) ; i++) {
    data[i] = ntohl(data[i]);
  }
	
  return 0;
}

int check_cl_frame(char *buf, int len)
{
  struct cl_header *clheader = NULL;
  char *dp = NULL;
  int retval = 0;;
  unsigned int id = 0;
  int i = 0;

  retval = 0;

  dp = buf;
  for (i = 0 ; i < nfinesse ; i++) {
    clheader = (struct cl_header *)dp;
    if (clheader->magic != CL_MAGIC) {
      fprintf(stderr, "Header broken ! %0x\n", clheader->magic);
      send_fatal_message("cltcp::check_cl_frame Header broken !");
      retval = -1;
    }
    if (is_checkid) {
      if (i == 0) {
	id = clheader->id;
      } else {
	if (id != clheader->id) {
	  fprintf(stderr, "Different ID!! %d(0) %d(%d) \n", id, clheader->id, i);
	  send_error_message("cltcp::check_cl_frame Different ID !");
	  retval = -1;
	}
      }
    }
    //    printf("MAGIC: %0x, ", clheader->magic);
    //    printf("SLOT: %0x, ", (clheader->key) >> 24);
    //    printf("KEY: %0x, ", (clheader->key) & 0xffffff);
    //    printf("LEN: %0x, ", clheader->length);
    //    printf("ID: %0x, ", clheader->id);
    //    printf("Trailer: %0x \n",
    //	   *((unsigned int *)(dp + clheader->length + 16)));
    dp += clheader->length + 20;
  }
	
  return retval;
}


int cltcp_open(char *hostname, int port)
{
  int sock;
  struct sockaddr_in srvaddr;
  struct hostent *hp;
  unsigned long int inaddr;
  int status;

  sock = socket(PF_INET, SOCK_STREAM, 0);

  memset((char*)&srvaddr, 0, sizeof(srvaddr));
  srvaddr.sin_family = AF_INET;
  srvaddr.sin_port = htons((unsigned short int)port);

  inaddr = inet_addr(hostname);
  if (inaddr == INADDR_NONE) {
    hp = gethostbyname(hostname);
    if (hp == NULL) return -1;
    if (hp->h_addrtype != AF_INET) return -1;
    memcpy((char *)&srvaddr.sin_addr,
	   (char*)hp->h_addr, hp->h_length);
  } else {
    memcpy((char *)&srvaddr.sin_addr,
	   (char*)&inaddr, sizeof(inaddr));
  }

  // Time out
  struct timeval tv;
  tv.tv_sec  = 6;
  tv.tv_usec = 0;
  setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&tv, sizeof(tv));

  status = connect(sock, (struct sockaddr *)&srvaddr, sizeof(srvaddr));

  return sock;	
}


int cltcp_close(int sock)
{
  return close(sock);
}

int cltcp_raw_read(int sock, char *buf, int len)
{
  int nleft, nrecv;

  nleft = len;
  while (nleft > 0) {
    nrecv = recv(sock, buf, nleft, 0);
    if (nrecv < 0) {
      return nrecv;
    } else if (nrecv == 0) {
      break;
    }

#if 0
    {
      int i;
      printf("\nlen: %d, nrecv: %d", len, nrecv);
      for (i = 0 ;  i < nrecv ; i++) {
	if ((i % 16) == 0) {
	  printf("\n %04x: ", i);
	}
	printf("%02x ", buf[i] & 0xff);
      }
      printf("\n");
    }
#endif

    nleft -= nrecv;
    buf += nrecv;
  }

  return len - nleft;
}

int cltcp_read(int sock, char *buf, int len)
{
  char *dp = NULL;
  struct cl_header *clheader = NULL;
  int nread=0, nbody=0;
  int status=0;
  int i = 0;

  dp = buf;
  for (i = 0 ; i < nfinesse ; i++) {
    nread = cltcp_raw_read(sock, dp, sizeof(struct cl_header));
    if(nread <= 0){return-1;} // time out ?
    if (is_bigendian) rev_byteorder(dp, sizeof(struct cl_header));
    clheader = (struct cl_header *)dp;
    if (clheader->magic != CL_MAGIC) {
      char message[400];
      fprintf(stderr, "MAGIC Error :");
      sprintf(message, "%04x %04x %04x %04x",
	      clheader->magic, clheader->key,
	      clheader->length, clheader->id);
      fprintf(stderr, "%s\n", message);
      send_fatal_message(message);
      
      std::exit(-1);
    }

    nbody = clheader->length;
    dp += nread;
    nread = cltcp_raw_read(sock, dp, nbody + 4);
    if (is_bigendian) rev_byteorder(dp, nread);
    dp += nread;
  }

  status = check_cl_frame(buf, dp - buf);

  return dp - buf;
}

int xx_cltcp_read(int sock, char *buf, int len)
{
  int nleft=0, nrecv=0;
  int status=0;

  nleft = len;

  nrecv = recv(sock, buf, nleft, 0);

  if (is_bigendian) {
    status = rev_byteorder(buf, nrecv);
  }
  status = check_cl_frame(buf, nrecv);

  return nrecv;
}


int cltcp_write(int sock, char *buf, int len)
{
  int nleft=0, nsent=0;

  nleft = len;
  while (nleft > 0) {
    nsent = send(sock, buf, nleft, 0);
    if (nsent <= 0) {
      return nsent;
    } 
    nleft -= nsent;
    buf += nsent;
  }

  return len - nleft;
}


int cltcp_wait_data(int fd, int timeout)
{
  fd_set rfds;
  struct timeval tv;
  int status;

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  status = select(fd + 1, &rfds, NULL, NULL, &tv);
  if (status == -1) {
    perror("cltcp_wait_data");
  } else if (status <= 0)  {
    //fprintf(stderr, "Timeout !!\n");
  }

  return status;
}

int cltcp_drop_event(int sock)
{
  int nrecv=0;
  int status=0;
  int len = 0;
  static int buf[1500];

  while(1) {

    status = cltcp_wait_data(sock, 1);
    /* printf("wait status : %d\n", status); */
    if (status <= 0) {
      /* printf("Timeout empty FIFO\n"); */
      break;
    }

    nrecv = recv(sock, buf, 1500, 0);
    if (nrecv < 0) {
      perror("cltcp_drop_event");
      break;
    } else if (nrecv == 0) {
      break;
    }

#if 0
    {
      int i;
      printf("\nlen: %d, nrecv: %d", len, nrecv);
      for (i = 0 ;  i < nrecv ; i++) {
	if ((i % 16) == 0) {
	  printf("\n %04x: ", i);
	}
	printf("%02x ", buf[i] & 0xff);
      }
      printf("\n");
    }
#endif
    len += nrecv;

  }

  return len;
}

int cltcp_set_finesse_act(int act)
{
  int i=0;

  finesse_act =  act & 0xf;

  nfinesse = 0;
  for (i = 0 ; i < 4 ; i++) {
    if (((finesse_act >> i) & 0x1) == 0x1) {
      nfinesse++;
    }
  }

  return nfinesse;
}

int cltcp_get_finesse_act()
{
  return finesse_act;
}

int cltcp_get_nfinesse()
{
  return nfinesse;
}



#ifdef TESTMAIN
static int is_drop = 0;

int main(int argc, char* argv[])
{
  int status=0;
  int port = 24;
  char hostname[] = "192.168.0.17";
  static char buf[4*1024*1024];
  int socket=0;
  int i=0;

  for (i = 1 ; i < argc ; i++) {
    int ival;
    if (strcmp(argv[i], "--drop-event") == 0) {
      is_drop = 1;
    }
    if (sscanf(argv[i], "--finesse=%d", &ival) == 1) {
      nfinesse = ival;
    }
  }

  socket = cltcp_open(hostname, port);

  if (is_drop) {
    printf("Drop Event!!\n");
    status = cltcp_drop_event(socket);
    return 0;
  }

  while (1) {
    status = cltcp_read(socket, buf, 1024*1024);
    if (status > 0) {
      printf("len : %d", status);
#if 0
      for (i = 0 ;  i < status ; i++) {
	if ((i % 16) == 0) {
	  printf("\n %04x: ", i);
	}
	printf("%02x ", buf[i] & 0xff);
      }
#endif
    }
    printf("\n");
  }

  status = cltcp_close(socket);
	
  return 0;
}
#endif
