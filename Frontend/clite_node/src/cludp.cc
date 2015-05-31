/*
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/select.h>

#include "rbcp.h"
#include "copperlite.h"
#include "MessageHelper.h"

const char *default_host = "192.168.0.16";
int default_port = 4660;
struct sockaddr_in destsockaddr;
static int sequence = 0;


struct copperlite_regs {
  unsigned int version;
  unsigned int id;
  char scr;
  char rcr;
  unsigned short int rsv1;
  char fcr;
  char keyword[3];
  unsigned int ffth;
  unsigned int rsv2[3];
};


int udpreg_wait_answer(int fd, int timeout)
{
  fd_set rfds;
  struct timeval tv;
  int status=0;

  FD_ZERO(&rfds);
  FD_SET(fd, &rfds);

  tv.tv_sec = timeout;
  tv.tv_usec = 0;

  status = select(fd + 1, &rfds, NULL, NULL, &tv);
  if (status == -1) {
    perror("udpreg_wait_answer");
  } else if (status <= 0)  {
    fprintf(stderr, "Timeout !!\n");
  } 

  return status;
}

int udpreg_open(char *hostname, int port)
{
  struct addrinfo hints, *ai;
  int sock=0;
  int status=0;

  memset((char *)&hints, 0, sizeof(hints));
  hints.ai_family = AF_INET;
  status = getaddrinfo(hostname, NULL, &hints, &ai);
  if (status != 0) {
    fprintf(stderr, "Unknown host %s (%s)\n",
	    hostname, gai_strerror(status));
    perror("udpreg_open");
    return -1;
  }

  memset((char *)&destsockaddr, 0, sizeof(destsockaddr));
  memcpy(&destsockaddr, ai->ai_addr, ai->ai_addrlen);
  //destsockaddr.sin_addr.s_addr = ai.ai_addr.sin_addr.s_addr;
  //destsockaddr.sin_addr.s_addr = inet_addr(dest);
  destsockaddr.sin_port = htons(port);
  destsockaddr.sin_family = AF_INET;
  freeaddrinfo(ai);

  sock = socket(AF_INET, SOCK_DGRAM, 0);
  if (sock < 0) {
    perror("udpreg_open socket");
    return -1;
  }
  status = connect(sock,
		   (struct sockaddr *)&destsockaddr, sizeof(destsockaddr));
  if (status != 0) {
    perror("udpreg_open connect");
    close(sock);
    return -1;
  }

  return sock;
}

int udpreg_send(int sock, char *buf, int len)
{
  int status=0;

  status = sendto(sock, buf, len, 0,
		  (struct sockaddr *)&destsockaddr, sizeof(destsockaddr));
  if (status != len) {
    perror("udpreg_send sendto");
    send_error_message("cludp::udpreg_send error");
    return -1;
  }

  return 0;
}

int udpreg_receive(int sock, char *buf)
{
  int status=0;
  //socklen_t fromlen;

  //status = *len = recvfrom(sock, buf, 1024, 0, NULL, &fromlen);
  status = recvfrom(sock, buf, 2048, 0, NULL, NULL);
	
  if (status < 0) {
    perror("udpreg_receive recvfrom");
    send_error_message("cludp::udpreg_receive error");
    return -1;
  }
  return status;
}

int udpreg_close(int sock)
{
  return close(sock);
}


int udpreg_read(int sock, char *buf, unsigned int addr, int len)
{
  struct bcp_header bcp;
  struct rbcp_header *rbcp;
  char *data=NULL;
  int status=0;
  int rlen = 0;

  bcp.type    = RBCP_VER;
  bcp.command = RBCP_CMD_RD;
  bcp.address = htonl(addr);
  bcp.length  = len;
  bcp.id = sequence++;

  status = udpreg_send(sock, (char *)&bcp, sizeof(bcp));
  /* fprintf(stderr, "#D send\n"); */
  status = udpreg_wait_answer(sock, 5);
  if (status > 0) {
    rlen = udpreg_receive(sock, buf);
    /* fprintf(stderr, "#D receive %d\n", rlen); */

    rbcp = (struct rbcp_header *)buf;
    data = buf + sizeof(struct rbcp_header);	
#if 0
    if (rlen > 0) {
      int i;
      printf("type: 0x%x, command: 0x%x, id: 0x%x, length: %d, address: %d",
	     rbcp->type & 0xff, rbcp->command& 0xff,
	     rbcp->id & 0xff, rbcp->length & 0xff,
	     ntohl(rbcp->address & 0xffffffff));
	
      for (i = 0 ; i < (int)(rlen - sizeof(struct rbcp_header)) ; i++) {
	if ((i % 8) == 0) printf("\n%04x: ", addr + i);
	printf("%02x ", data[i] & 0xff);
      }
      printf("\n");
    }
#endif

  } else {
    printf("udpreg_read someting wrong\n");
    rlen = -1;
  }

  return rlen;
}

int udpreg_write(int sock, char *data, unsigned int addr, int len)
{
  struct bcp_header *bcp;
  struct rbcp_header *rbcp;
  char *bcp_body;
  static char sbuf[2048];
  static char rbuf[2048];
  int status= 0;
  int rlen  = 0;

  bcp = (struct bcp_header *)sbuf;
  bcp_body = sbuf + sizeof(struct bcp_header);

  bcp->type = RBCP_VER;
  bcp->command = RBCP_CMD_WR;
  bcp->address = htonl(addr);
  //bcp->length = len + sizeof(struct bcp_header);
  bcp->length = len;
  bcp->id = sequence++;

  if (len < (int)(2048 - sizeof(struct bcp_header))) {
    memcpy(bcp_body, data, len);
  } else {
    fprintf(stderr, "udpreg_write: Buffer overflow %d\n", len);
    return -1;
  }

  status = udpreg_send(sock, (char *)&sbuf, sizeof(struct bcp_header) + len);
  /* fprintf(stderr, "#D send\n"); */
  status = udpreg_wait_answer(sock, 5);
  if (status > 0) {
    rlen = udpreg_receive(sock, rbuf);
    /* fprintf(stderr, "#D receive %d\n", rlen); */
    rbcp = (struct rbcp_header *)rbuf;
    data = rbuf + sizeof(struct rbcp_header);	
#if 1
    if (rlen > 0) {
      int i;
      printf("type: 0x%x, command: 0x%x, id: 0x%x, length: 0x%x, address: 0x%x",
	     rbcp->type & 0xff, rbcp->command& 0xff,
	     rbcp->id & 0xff, rbcp->length & 0xff,
	     ntohl(rbcp->address & 0xffffffff));

      for (i = 0 ; i < (int)(rlen - sizeof(struct rbcp_header)) ; i++) {
	if ((i % 8) == 0) printf("\n%04x: ", addr + i);
	printf("%02x ", data[i] & 0xff);
      }
      printf("\n");
    }
#endif
  } else {
    printf("udpreg_read someting wrong\n");
    rlen = -1;
  }

  return rlen;
}


#ifdef TESTMAIN 
#define BUFSIZE 1024

int main(int argc, char* argv[])
{
  int sock;
  //struct bcp_header bcp;
  static char buf[BUFSIZE];
  char *header;
  char *body;
  //int rlen;
  char hostname[256];
  int port;
  unsigned int raddress, rlen;
  unsigned int waddress;
  int wdata;

  struct copperlite_regs clregs;
  int is_copper_read = 1;
  int is_finesse_read = 1;

  int status;
  int i;

  header = buf;
  body = buf + sizeof(struct rbcp_header);

  strcpy(hostname, default_host);
  port = default_port;

  wdata = -1;
  rlen = 0;
  for (i = 1 ; i < argc ; i++) {
    if (sscanf(argv[i], "--host=%s", hostname) == 1) ;
    if (sscanf(argv[i], "--port=%d", &port) == 1) ;
    if (sscanf(argv[i], "--read=%x:%x", &raddress, &rlen) == 1);
    if (sscanf(argv[i], "--write=%x:%x", &waddress, &wdata) == 1);
    if (strncmp(argv[i], "--seq1", 6) == 0) ;
  }

  printf("host: %s\n", hostname);
  sock = udpreg_open(hostname, port);


  if (rlen > 0) {
    printf("read: 0x%x : %d\n", raddress, rlen);
    status = udpreg_read(sock, buf, raddress, rlen);
  }
  if (wdata >= 0) {
    printf("write: 0x%x : %x\n", waddress, wdata);
    buf[0] = wdata & 0xff;
    buf[1] = 0;
    status = udpreg_write(sock, buf, waddress, 1);
  }


  for (i = 1 ; i < argc ; i++) {
    if (sscanf(argv[i], "--scr=%x", &wdata) == 1) {
      buf[0] = wdata & 0xff;
      buf[1] = 0;
      status = udpreg_write(sock, buf, CL_SCR, 1);
    }
    if (sscanf(argv[i], "--rcr=%x", &wdata) == 1) {
      buf[0] = wdata & 0xff;
      buf[1] = 0;
      status = udpreg_write(sock, buf, CL_RCR, 1);
    }
    if (sscanf(argv[i], "--fcr=%x", &wdata) == 1) {
      buf[0] = wdata & 0xff;
      buf[1] = 0;
      status = udpreg_write(sock, buf, CL_FCR, 1);
    }
    if (sscanf(argv[i], "--keyword=%x", &wdata) == 1) {
      buf[2] = wdata & 0xff;
      buf[1] = (wdata >> 8) & 0xff;
      buf[0] = (wdata >> 16) & 0xff;
      status = udpreg_write(sock, buf, CL_KEYWORD, 3);
    }
    if (sscanf(argv[i], "--ffth=%x", &wdata) == 1) {
      buf[3] = wdata & 0xff;
      buf[2] = (wdata >> 8) & 0xff;
      buf[1] = (wdata >> 16) & 0xff;
      buf[0] = (wdata >> 24) & 0xff;
      status = udpreg_write(sock, buf, CL_FFTH, 4);
    }
  }


  if (is_copper_read) {
    memset(buf, 0, BUFSIZE);
    status = udpreg_read(sock, buf, 0, sizeof(clregs));
    /*
      for (i = 0 ; i < 16 ; i++) {
      printf("%02x ", buf[i] & 0xff);
      }
      printf("\n");
    */
    clregs.version = ntohl(*((unsigned int *)(body + CL_VERSION)));
    clregs.id = ntohl(*((unsigned int *)(body + CL_ID)));
    clregs.scr = body[CL_SCR];
    clregs.rcr = body[CL_RCR];
    clregs.fcr = body[CL_FCR];
    clregs.keyword[0] = body[CL_KEYWORD];
    clregs.keyword[1] = body[CL_KEYWORD+1];
    clregs.keyword[2] = body[CL_KEYWORD+2];
    clregs.ffth = ntohl(*((unsigned int *)(body + CL_FFTH)));

    printf("FPGA Ver.: %08x, ", clregs.version);
    printf("ID       : %08x\n", clregs.id);
    printf("Control  : %02x,       ", clregs.scr & 0xff);
    printf("Reset    : %02x\n", clregs.rcr & 0xff);
    printf("FCR      : %02x,       ", clregs.fcr & 0xff);
    printf("Keyword  : %02x %02x %02x\n",
	   clregs.keyword[0] & 0xff, clregs.keyword[1] & 0xff,
	   clregs.keyword[2] & 0xff);
    printf("FF Th.   : %08x", clregs.ffth);
    printf("\n");
  }

  if (is_finesse_read) {
    printf("- FINESSE space -");
    memset(buf, 0, BUFSIZE);
    status = udpreg_read(sock, buf, CL_FINESSE, 0x40);
    for (i = 0 ; i < 0x40 ; i++) {
      if ((i%4) == 0) printf("\n%04x: ", CL_FINESSE + i);
      printf("%02x ", body[i] & 0xff);
    }
    printf("\n    :    ----");
    memset(buf, 0, BUFSIZE);
    status = udpreg_read(sock, buf, CL_FINESSE + 0x7A*4, 0x20);
    for (i = 0 ; i < 0x20 ; i++) {
      if ((i%4) == 0) printf("\n%04x: ", CL_FINESSE + 0x7A*4 + i );
      printf("%02x ", body[i] & 0xff);
    }
    printf("\n");
		
  }


  udpreg_close(sock);

  return 0;
}
#endif
