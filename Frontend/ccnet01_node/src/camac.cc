/*
 * camac.c : CAMAC library for PCI / pipeline CAMAC controller
 * Copyright 2003 (C) 2003 Yoshiji Yasu <Yoshiji.YASU@kek.jp>.
 *
 * 
 * version: 0.1  04-DEC-2002, born
 *          0.3  01-AUG-2003, release for beta version
 *                              LAM & TRIG hanlding are not available.
 *          0.4  25-SEP-2003, bug fix
 *          0.5  03-OCT-2003  modification of LAM & TRIG handler
*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "pcc.h"
#include "camac.h"

void dump_pccreg(struct pccreg *pccreg) {

  printf("Tx Control      = %x\n", pccreg->TxControl);
  printf("Tx Status       = %x\n", pccreg->TxStatus);
  printf("Tx Address      = %x\n", pccreg->TxAddress);
  printf("Tx Preset Count = %x\n", pccreg->TxPresetCount);
  printf("Tx Actual Count = %x\n", pccreg->TxActualCount);
  printf("Tx Fifo Count   = %x\n", pccreg->TxFifoCount);
  printf("Rx Control      = %x\n", pccreg->RxControl);
  printf("Rx Status       = %x\n", pccreg->RxStatus);
  printf("Rx Address      = %x\n", pccreg->RxAddress);
  printf("Rx Preset Count = %x\n", pccreg->RxPresetCount);
  printf("Rx Actual Count = %x\n", pccreg->RxActualCount);
  printf("Rx Fifo Count   = %x\n", pccreg->RxFifoCount);
  printf("System          = %x\n", pccreg->System);
  printf("Int Control     = %x\n", pccreg->IntControl);
  printf("Int Status      = %x\n", pccreg->IntStatus);
  printf("Int Fifo Count  = %x\n", pccreg->IntFifoCount);
}

//int cam_naf( n, a, f )
//     int n, a, f; {
int cam_naf( int n, int a, int f ){
  return  ((((n<<8) + a) << 8) + f);
}

int gen_cam_command(int *data1, int *data2, int n, int a, int f, int data, int flag ) {
  int cmd = 0;

  if( n >= 0 && (n <=23 | n==25) && a >= 0 && a <=15 && f >=0 && f <=31 ) {
    cmd = cam_naf(n, a, f);
    switch (flag) {
    case 0: // normal
      cmd |= 0x80000000;
      break;
    case 1: // start bit
      cmd |= 0xC0000000;
      break;
    case 2: // end bit
      cmd |= 0xA0000000;
      break;
    default:
      cmd |= 0xE0000000;
    }
    *data1 = data & 0xFFFFFF;
    *data2 = cmd;
  } else 
    return -1;
  return 0;
}

int gen_daq_command(int *data1, int *data2, int func, int flag ) {
  int cmd;

  cmd = func;
  switch (flag) {
  case 0: // normal
    cmd |= 0x90000000;
    break;
  case 1: // start bit
    cmd |= 0xD0000000;
    break;
  case 2: // end bit
    cmd |= 0xB0000000;
    break;
  default:
    cmd |= 0xF0000000;
  }
  *data1 = 0;
  *data2 = cmd;
  return 0;
}  

int cam_gen_init( int length, int* buf ) {
  int i;
  if( length <= 0 | length > MAX_LENGTH )
    return -1;
  buf[0] = length;
  buf[1] = 0;
  for(i=2;i<length*2+2;i++)
    buf[i] = 0;
  return 0;
}

int cam_gen_cc( int *buf, int n, int a, int f, int data ) {
  int status;
  int header;

  if( buf[0] == buf[1] )
    return -2;
  if( buf[1] < 0 )
    return -3;
  switch (buf[1]) {
  case 0:
    status = gen_cam_command(&buf[buf[1]*2+2],&buf[buf[1]*2+3],n,a,f,data,3);
    break;
  case 1:
    header = (buf[buf[1]*2+1]>>24)&0xFF;
    buf[buf[1]*2+1] &= ~0xF0000000;
    if(header == 0xE0)
      buf[buf[1]*2+1] |= 0xC0000000;
    else
      buf[buf[1]*2+1] |= 0xD0000000;
    status = gen_cam_command(&buf[buf[1]*2+2],&buf[buf[1]*2+3],n,a,f,data,2);
    if( status == -1 )
      return status;
    break;
  default:
    header = (buf[buf[1]*2+1]>>24)&0xFF;
    buf[buf[1]*2+1] &= ~0xF0000000;
    if(header == 0xA0)
      buf[buf[1]*2+1] |= 0x80000000;
    else
      buf[buf[1]*2+1] |= 0x90000000;
    status = gen_cam_command(&buf[buf[1]*2+2],&buf[buf[1]*2+3],n,a,f,data,2);
    if( status == -1 )
      return status;
  }
  buf[1]++;
  return status;
}

int cam_gen_daq( int* buf, int cmd, int data ) {
  int status;
  int header;

  if( buf[0] == buf[1] )
    return -2;
  if( buf[1] < 0 )
    return -3;
  switch (buf[1]) {
  case 0:
    status = gen_daq_command(&buf[buf[1]*2+2],&buf[buf[1]*2+3],cmd,3);
    break;
  case 1:
    header = (buf[buf[1]*2+1]>>24)&0xFF;
    buf[buf[1]*2+1] &= ~0xF0000000;
    if(header == 0xE0)
      buf[buf[1]*2+1] |= 0xC0000000;
    else
      buf[buf[1]*2+1] |= 0xD0000000;
    status = gen_daq_command(&buf[buf[1]*2+2],&buf[buf[1]*2+3],cmd,2);
    if( status == -1 )
      return status;
    break;
  default:
    header = (buf[buf[1]*2+1]>>24)&0xFF;
    buf[buf[1]*2+1] &= ~0xF0000000;
    if(header == 0xA0)
      buf[buf[1]*2+1] |= 0x80000000;
    else
      buf[buf[1]*2+1] |= 0x90000000;
    status = gen_daq_command(&buf[buf[1]*2+2],&buf[buf[1]*2+3],cmd,2);
    if( status == -1 )
      return status;
  }
  buf[1]++;
  return status;
}

void cam_decode_cc_frame( int data1, int data2, int* n, int* a, int* f, int* data, int* status) {

  *data = data1 & 0xFFFFFF;
  *status = (data1>>24) & 0xFF;
  *f    = data2 & 0x1F;
  *a    = (data2>>8) & 0xF;
  *n    = (data2>>16) & 0x1F;
}

int cam_extract_cc_data( int* rplybuf, int len, int* actuallen, int* data ) {
  int i, num;

  num = rplybuf[1]; // get number of reply frames
  if( num > len )
    return -2; // lots of data are included in rplybuf
  for( i = 0; i < num; i++ )
    data[i] = rplybuf[2*i+2] & 0xFFFFFF;
  *actuallen = num;
  return 0;
}

int cam_extract_cc_status( int* rplybuf, int len, int* actuallen, int* status ) {
  int i, num;

  num = rplybuf[1]; // get number of reply frames
  if( num > len )
    return -2; // lots of data are included in rplybuf
  for( i = 0; i < num; i++ )
    status[i] = (rplybuf[2*i+2]>>24) & 0xFF;
  *actuallen = num;
  return 0;
}

int cam_extract_cc_qx( int status, int* q, int* x ) {
  *q = status & 1;
  *x = (status>>1) & 1;
  return 0;
}

int cam_extract_daq_data( int* rplybuf, int len, int* actuallen, int* data ) {
  int i, num;

  num = rplybuf[1]; // get number of reply frames
  if( num > len )
    return -2; // lots of data are included in rplybuf
  for( i = 0; i < num; i++ )
    data[i] = rplybuf[2*i+2];
  *actuallen = num;
  return 0;
}

int cam_open( void ) {
  int fd;

  fd = open("/dev/pcc0", O_RDWR);
  if( fd < 0 )
    return -1;
  else
    return fd;
}

int cam_close( int fd ) {
  return close(fd);
}

int cam_single_cc( int fd, int n, int a, int f, int *data, int *q, int *x ) {
  int qx;
  int data1, data2;
  int cmd, rply;
  int status;


  status = gen_cam_command( &data1, &cmd, n, a, f, *data, 3);
  if( status )
    return -1;
  status = cam_put( fd, data1, cmd );
  if( status )
    return -1;
  status = cam_get( fd, &data2, &rply );
  if( status )
    return -1;
  if( (f>=0) && (f<=7) )
    *data = 0xFFFFFF&data2;
  qx = 0x3&(data2>>24);
  if( qx & 1 )
    *q = 1;
  else
    *q = 0;
  if( qx & 2 )
    *x = 1;
  else
    *x = 0;
  return 0;
}

int cam_single_daq( int fd, int func, int *data, int *daq_status ) {
  int data1, data2;
  int cmd, rply;
  int status;

  status = gen_daq_command( &data1, &cmd, func, 3 );
  if( status )
    return -1;
  status = cam_put( fd, data1, cmd );
  if( status )
    return -1;
  status = cam_get( fd, &data2, &rply );
  if( status )
    return -1;
  *data = data2;
  *daq_status = 0xFF & rply;
  return 0;
}

int cam_reset( int fd ) {
  int status;
  if( (status = ioctl(fd, PCCIOC_RESET)) < 0 ) {
    return errno;
  }
  return 0;
}

int cam_clear_fifo( int fd ) {
  int status;
  if( (status = ioctl(fd, PCCIOC_CLEAR_FIFO)) < 0 ) {
    return errno;
  }
  return 0;
}

int cam_dump_pccreg( int fd, struct pccreg *pccreg ) {
  int status;
  if( (status = ioctl(fd, PCCIOC_DUMP_REGISTERS, pccreg)) == -1 ) {
    return errno;
  }
  return 0;
}

int cam_get( int fd, int* data, int* rply ) {
  int buf[2], status;

  if( (status = ioctl(fd, PCCIOC_GET_DATA, buf)) < 0 ) {
    return errno;
  }
  *data = buf[0];
  *rply = buf[1];
  return 0;
}

int cam_getint( int fd, int* data, int* rply ) {
  int buf[2], status;

  if( (status = ioctl(fd, PCCIOC_GET_INTDATA, buf)) < 0 ) {
    return errno;
  }
  *data = buf[0];
  *rply = buf[1];
  return 0;
}

int cam_put(int fd, int data, int cmd ) {
  int buf[2], status;

  buf[0] = data;
  buf[1] = cmd;
  if( (status = ioctl(fd, PCCIOC_PUT_DATA, buf)) < 0 ) {
    return errno;
  }
}

int cam_exec_pio( int fd, int* cmdbuf, int* rplybuf ) {
  int status;
  int* pbuf[2];

  pbuf[0] = cmdbuf;
  pbuf[1] = rplybuf;
  if( (status = ioctl(fd, PCCIOC_EXEC_PIO, pbuf)) < 0 ) {
    printf("ioctl-- exec PIO error fd = %d status = %d\n", fd, status);
    return errno;
  }
  return 0;
}

int cam_exec_dma( int fd, int* cmdbuf, int* rplybuf ) {
  int *cbuf = cmdbuf+2;
  int *rbuf = rplybuf+2;
  int length = cmdbuf[1];
  int retlen = 0;
  int i, j, status;
  int loop, remain, amount;

  retlen = length*8;
  loop = length / MAX_FRAME_LENGTH;
  remain = length % MAX_FRAME_LENGTH;
  amount = 0;
  if (loop) {
    for(i=0; i < loop; i++) {
      status = ioctl(fd, PCCIOC_KICK_READ, &retlen);
      if (status < 0)
	return errno;
      retlen = write(fd, cbuf, MAX_BUFFER_SIZE);
      if ( retlen != MAX_BUFFER_SIZE ) {
	if( retlen < 0 )
	  return errno;
	else
	  return -1;
      }
      retlen = read(fd, rbuf, MAX_BUFFER_SIZE);
      if ( retlen != MAX_BUFFER_SIZE ) {
	if ( retlen < 0 )
	  return errno;
	else
	  return -1;
      }
      amount += retlen/8;
      cbuf += MAX_LENGTH;
      rbuf += MAX_LENGTH;
    }
  }
  if (remain) {
    status = ioctl(fd, PCCIOC_KICK_READ, &retlen);
    if (status < 0)
      return errno;
    retlen = write(fd, cbuf, remain*8);
    if ( retlen != remain*8 ) {
      if( retlen < 0 )
	return errno;
      else
	return -1;
    }
    retlen = read(fd, rbuf, remain*8);
    if ( retlen != remain*8 ) {
      if( retlen < 0 )
	return errno;
      else
	return -1;
    }
    amount += retlen/8;
    cbuf += remain*2;
    rbuf += remain*2;
  }
  rplybuf[1] = amount;
  return 0;
}

int cam_exec_dma_seq( int fd, int* cmdbuf, int* rplybuf ) {
  int *cbuf = cmdbuf+2;
  int *rbuf = rplybuf+2;
  int length = cmdbuf[1];
  int retlen = 0;
  int i, j, status;
  int cmdlen;
  int amount, loop, remain;

  retlen = length*8; // in bytes
  cmdlen = length;   // in frame size (8 bytes)
  loop = cmdlen/MAX_FIFO_FRAME;
  remain = cmdlen%MAX_FIFO_FRAME;
  amount = 0;
  if( loop ) {
    for( i = 0; i < loop; i++ ) {
      retlen = write(fd, cbuf, MAX_FIFO_BYTE);
      if( retlen != MAX_FIFO_BYTE ) {
	if( retlen < 0 )
	  return errno;
	else
	  return -1;
      }
      retlen = read(fd, rbuf, MAX_FIFO_BYTE);
      if( retlen != MAX_FIFO_BYTE ) {
	if( retlen < 0 )
	  return errno;
	else
	  return -1;
      }
      amount += retlen/8;
      cbuf += MAX_FIFO_COUNT;
      rbuf += MAX_FIFO_COUNT;
    }
  }
  if( remain ) {
    retlen = write(fd, cbuf, remain*8);
    if( retlen != remain*8 ) {
      if( retlen < 0 )
	return errno;
      else
	return -1;
    }
   retlen = read(fd, rbuf, remain*8); 
   if( retlen != remain*8 ) {
     if( retlen < 0 )
       return errno;
     else
       return -1;
   }
   amount += retlen/8;
  }
  rplybuf[1] = amount;
  return 0;
}

int cam_exec( int fd, int* cmdbuf, int* rplybuf ) {
  int status;

  if( cmdbuf[1] > NUM_FRAME_SWITCH ) 
    status = cam_exec_dma( fd, cmdbuf, rplybuf );
  else
    status = cam_exec_pio( fd, cmdbuf, rplybuf );
  return status;
}

int cam_enable_lam( int fd, int enable_pattern ) {
  int status, data, q, x;

  // write interrupt enable bits
  data = enable_pattern;
  if( (status = cam_single_cc(fd, 25, 1, 16, &data, &q, &x)) < 0 )
    return -1;

  // enable interrupt of CAMAC
  if( (status = cam_single_cc(fd, 25, 1, 26, &data, &q, &x)) < 0 )
    return -1;

  // enable interrupt at PCI
  if( (status = ioctl(fd, PCCIOC_ENABLE_INTERRUPT)) < 0 )
    return -1;

  return 0;
}

int cam_wait_lam( int fd, int* lam_pattern, int timeout ) {
  int status, data[2], q, x;

  data[0]= timeout;
  if( (status = ioctl(fd, PCCIOC_WAIT_INTERRUPT, data)) < 0 )
    return errno;
  data[1] &= 0xF8000000;
  if ( data[1] == 0xE8000000 )
    *lam_pattern = data[0] & 0xFFFFFF;
  else
    return -1;
  return 0;
}

int cam_disable_lam( int fd ) {
  int status, data, q, x;

  // disable interrupt of CAMAC
  if( (status = cam_single_cc(fd, 25, 1, 24, &data, &q, &x)) < 0 )
    return -1;

  // write interrupt enable bits (all disabled)
  data = 0;
  if( (status = cam_single_cc(fd, 25, 1, 16, &data, &q, &x)) < 0 )
    return -1;

  // disable interrupt of PCI
  if( (status = ioctl(fd, PCCIOC_DISABLE_INTERRUPT)) < 0 )
    return -1;

  return 0;
}

int cam_enable_trig( int fd ) {
  int status, data, daq_status;

  // enable trigger at PCI
  if( (status = ioctl(fd, PCCIOC_ENABLE_INTERRUPT)) < 0 )
    return -1;

  // enable trigger input at CAMAC
  status = cam_single_daq( fd, DAQEXE_CTRL_ENABLE, &data, &daq_status);
  if( status < 0 )
    return -1;
  return 0;
}

int cam_wait_trig( int fd, int* event_count, int timeout ) {
  int status, data[2];

  data[0] = timeout;
  if( (status = ioctl(fd, PCCIOC_WAIT_INTERRUPT, data)) < 0 )
    return errno;
  data[1] &= 0xF8000000;
  if ( data[1] == 0xF8000000 )
    *event_count = data[0];
  else
    return -1;
  return 0;
}

int cam_disable_trig( int fd ) {
  int status, data, daq_status;

  // disable trigger input at CAMAC
  status = cam_single_daq( fd, DAQEXE_CTRL_DISABLE, &data, &daq_status);
  if( status < 0 )
    return -1;

  // disable interrupt at PCI
  if( (status = ioctl(fd, PCCIOC_DISABLE_INTERRUPT)) < 0 )
    return -1;

  return 0;
}
