#ifndef CAMAC_H_
#define CAMAC_H_

void dump_pccreg(struct pccreg *pccreg);
int cam_naf(int n, int a, int f);
int gen_cam_command(int *data1, int *data2, int n, int a, int f, int data, int flag );
int gen_daq_command(int *data1, int *data2, int func, int flag );
int cam_gen_init( int length, int* buf );
int cam_gen_cc( int *buf, int n, int a, int f, int data );
int cam_gen_daq( int* buf, int cmd, int data );
void cam_decode_cc_frame( int data1, int data2, int* n, int* a, int* f, int* data, int* status);
int cam_extract_cc_data( int* rplybuf, int len, int* actuallen, int* data );
int cam_extract_cc_status( int* rplybuf, int len, int* actuallen, int* status );
int cam_extract_cc_qx( int status, int* q, int* x );
int cam_extract_daq_data( int* rplybuf, int len, int* actuallen, int* data );
int cam_open( void );
int cam_close( int fd );
int cam_single_cc( int fd, int n, int a, int f, int *data, int *q, int *x );
int cam_single_daq( int fd, int func, int *data, int *daq_status );
int cam_reset( int fd );
int cam_clear_fifo( int fd );
int cam_dump_pccreg( int fd, struct pccreg *pccreg );
int cam_get( int fd, int* data, int* rply );
int cam_getint( int fd, int* data, int* rply );
int cam_put(int fd, int data, int cmd );
int cam_exec_pio( int fd, int* cmdbuf, int* rplybuf );
int cam_exec_dma( int fd, int* cmdbuf, int* rplybuf );
int cam_exec_dma_seq( int fd, int* cmdbuf, int* rplybuf );
int cam_exec( int fd, int* cmdbuf, int* rplybuf );
int cam_enable_lam( int fd, int enable_pattern );
int cam_wait_lam( int fd, int* lam_pattern, int timeout );
int cam_disable_lam( int fd );
int cam_enable_trig( int fd );
int cam_wait_trig( int fd, int* event_count, int timeout );
int cam_disable_trig( int fd );


#endif
