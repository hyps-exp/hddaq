//--------------------------------------------
//  apvdaq.c
//  author:ASANO Hidemitsu
//  Kyoto Univ.
//  ver1.0 2009 6 25
//  ver1.1 2009 7 11
//  ver1.2 2009 9 27
//  ver1.3 2009 10 9
//  last modified 2009 10 29 
//--------------------------------------------


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <math.h>
#include <fstream>
#include <sstream>


#include "gef/gefcmn_vme.h"

#include "data_buffer.hh"
#include "MessageHelper.h"

#include "apvdaq.hh"
#include "apvdaq_function.hh"
#include "apvdaq_param.hh"

#include <sys/time.h>


unsigned short *data;

void ssleep(int time)
{
  usleep(time*1000000);
}

void msleep(int time)
{
  usleep(time*1000);
}

void delay_us()
{
  usleep(1);
}

void gettimeofday_sec(unsigned long *t_sec, unsigned long *t_usec)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  //printf("tv_sec = %d, tv_usec = %d\n",tv.tv_sec, tv.tv_usec);

  *t_sec  = (unsigned long)tv.tv_sec;  
  *t_usec = (unsigned long)tv.tv_usec;
}

void get_clock_gettime(unsigned long *t_sec, unsigned long *t_nsec)
{
  struct timespec tv;
  clock_gettime(CLOCK_REALTIME,&tv);
  
  *t_sec  = (unsigned long)tv.tv_sec;
  *t_nsec = (unsigned long)tv.tv_nsec;
}

void reset_ai2c_state(unsigned long  module) 
{
  long data1,data2;
  data1 = SET_RESET_I2C;
  vwrite32(module,WRITE_AI2C_RESET,&data1);//set RESET I2C
  msleep(1);//sleep 1 millisecond

  data2 = UNSET;//0
  vwrite32(module,WRITE_AI2C_RESET,&data2);//unset RESET I2C
  msleep(1);
  
  return ;
}



unsigned char apv_i2c_write(unsigned char address, unsigned char *i2cdata, unsigned char bytes,unsigned long module)
// Write one or more data bytes to the I2C bus using the specified address 
{
  unsigned long apv_i2c_word;
  
  // reset i2c state machine */
  reset_ai2c_state(module);

  msleep(1);//sleep 1 millisecond

  // write (up to)4 data byte(s) */
  apv_i2c_word = i2cdata[0]|(i2cdata[1]<<8)|(i2cdata[2]<<16)|(i2cdata[3]<<24); 
  //vwrite32(module,WRITE_AI2C_WORD,&apv_i2c_word);
  vwrite32_uint(module,WRITE_AI2C_WORD,&apv_i2c_word);

  // write address and issue I2C write command
  apv_i2c_word = (bytes<<8)|(address<<1)|APV_WRITE; //WRITE 0
  //vwrite32(module,WRITE_AI2C_ADDR_RW, &apv_i2c_word);
  vwrite32_uint(module,WRITE_AI2C_ADDR_RW, &apv_i2c_word);

  msleep(1);

  // read back error word
  //vread32(module,READ_AI2C_ERR,&apv_i2c_word); 
  vread32_uint(module,READ_AI2C_ERR,&apv_i2c_word); 

  // return both VME error and I2C error
  return (apv_i2c_word&0x0f);
  //
}



unsigned char apv_i2c_read(unsigned char address, unsigned long *i2cdata, unsigned char bytes,unsigned long module)
// Reads one or more data bytes to the I2C bus using the specified address 
{
  unsigned long apv_i2c_word;
  unsigned char error;
  

  /* reset i2c state machine */
  reset_ai2c_state(module);

  msleep(1);

  /* write address and issue I2C read command */
  apv_i2c_word = (bytes<<8)|(address<<1)|APV_READ; // READ 1
  //vwrite32(module,WRITE_AI2C_ADDR_RW,&apv_i2c_word);
  vwrite32_uint(module,WRITE_AI2C_ADDR_RW,&apv_i2c_word);

  msleep(1);

  /* read back error word */
  //vread32(module,READ_AI2C_ERR,&apv_i2c_word);
  vread32_uint(module,READ_AI2C_ERR,&apv_i2c_word);
  error = apv_i2c_word & 0x0f;

  /* read back data byte(s) */
  //vread32(module,READ_AI2C_WORD,i2cdata);
  vread32_uint(module,READ_AI2C_WORD,i2cdata);

  // printf( "I2C read A:  Addr=%2d Data=%3d Result=%3d \n",address,data[0],error);
  // return both VME error and I2C error 
 
  return error;
}



int apvi2c_writeread(unsigned char address, unsigned char reg, unsigned char i2cdata, char *verbose,unsigned long module)
/* Write 2 bytes to I2C, read back in the APV specific way and compare.
   Possible return codes:
       0...OK
      -1...Error during I2C write operation (e.g. bus busy, no ACK)
      -2...Error during I2C set read register operation (e.g. no ACK)
      -3...Error during I2C read operation (e.g. no ACK)
      -4...Write and readback values do not match (possibly device failure) */
{
  unsigned char st;
  unsigned char write[4];
  unsigned long readback;
  unsigned char i;
  char action[100];


  /* I2C write */
  write[0]=reg;
  write[1]=i2cdata;

  i=0;
  do{
    st=apv_i2c_write(address,write,2,module);//address write 2byte
    sprintf(action,"I2C write: Addr=%2d Data=%3d %3d Result(1)=%3d \n",address,write[0],write[1],st);
    //shut up by joo
    if(st != 0 ){strcat(verbose,action);}
    i++;
  }while ((st) && (i<MAX_I2C_RETRIES));
  if(st != 0) return(-1);      /* write error  */

  
  /* Set read register */
  write[0]|=0x01;
  i=0;
  do{
    st=apv_i2c_write(address,write,1,module);
    sprintf(action,"I2C write: Addr=%2d Data=%3d Result(2)=%3d \n",address,write[0],st);
    //shut up by joo
    if(st != 0){strcat(verbose,action);}
    i++;
  }while ((st) && (i<MAX_I2C_RETRIES));

  if(st != 0) return(-2);      /* set read register error */
  
  
  /* I2C read */
  i=0;
  do{
    st=apv_i2c_read(address,&readback,1,module);
    sprintf(action,"I2C read:  Addr=%2d Data=%4lu Result(3)=%3d \n",address,readback,st);
    //shut up by joo
    if(st != 0){strcat(verbose,action);}
    i++;
  }while ((st) && (i<MAX_I2C_RETRIES));
  
  if(st != 0) return(-3);      /* read error */
  
  
  /* compare */
  if(i2cdata!=readback) st=-4;		   /* read != write */
 
  return(st);
}



int apv_readerr(unsigned char address, unsigned char i2cdata, char *verbose,unsigned long module)
{
  int st;
  unsigned char write[4];
  unsigned long readback;
  unsigned char i;
  char action[100];

  /* Set read register */
  write[0]=i2cdata;
  i=0;
  do{
    st=apv_i2c_write(address,write,1,module);
    sprintf(action,"I2C write: Addr=%2d Data=%3d Result=%3d Module=%lx \n",address,write[0],st,module);
    strcat(verbose,action);
    i++;
  }while ((st) && (i<MAX_I2C_RETRIES));

  if(st != 0) return(-2);      /* set read register error */
  
  i=0;
  do{
     st=apv_i2c_read(address,&readback,1,module);
     sprintf(action,"I2C read:  Addr=%2d Data=%3lu Result=%3d Module=%lx \n",address,readback,st,module);
     strcat(verbose,action);
     i++;
  }while ((st) && (i<MAX_I2C_RETRIES));

  if(st != 0) return(-3);      /* read error */

  return readback;
}



int init_apv(unsigned char address, char *verbose,unsigned long module)
// initializes operation mode of an apv
{
  int st;
  //unsigned char i,j;
  int i,j;

  i=0;
  do{
    st=0;
    for(j=0;j<=15;j++){ 
      st|=apvi2c_writeread(address,apv_i2c_settings[j][0],apv_i2c_settings[j][1],verbose,module);
    }	 
    apv_softreset();
    st|=apv_readerr(address,0,verbose,module);
    i++;
  }while ((st) && (i<MAX_I2C_RETRIES));

  return(st);
}



void  apv_softreset(void)
{
  write_trigger_seq(1); 

  return ;
}



void apv_hardreset(void)
{
  write_conf(0x100);//1000000000 turn on reset apv
  write_conf(0x0ff);//0111111111 turn off reset apv

  printf("APV HARD RESET\n");
  return;
}

void write_conf(long conf_register)
{
  vwrite32(BROADCAST,WRITE_CONF,&conf_register); 
  msleep(10);
  return;
}

int phos_i2c_write(long address,long phosi2cdata,unsigned long module)
{
  long data2;
  long e,error;
  data2 = SET_RESET_I2C;
  vwrite32(module,WRITE_PI2C_RESET,&data2);//set RESET I2C
  msleep(200);

  data2 = UNSET;
  vwrite32(module,WRITE_PI2C_RESET,&data2);//unset RESET I2C
  msleep(200);
  printf("Reset PHOS_I2C state\n");  
  
  vwrite32(module,WRITE_PI2C_WORD,&phosi2cdata); 
  e = (address<<1) + 0x100;//number of data bytes (bytes<<8 | address<<1) 
  vwrite32(module,WRITE_PI2C_ADDR_RW, &e); 
  msleep(10);
  vread32(module,READ_PI2C_ERR,&e); 
  error = e & 0x0f;
  // if(!error) std::cerr<<"phos_i2c error \n 19 September 2015"<<std::endl;
  //OK: error =0
  
  return error; 
}

void reset_phos(void)
{
  long delay;
  //D8 RESET_APV25
  //D7 APV25_POWER on
  //D6 PHOS4 power off
  //D5 Shift register 0 or Sequncer 1
  //D4 ADC_S2=0
  //D3 ADC_S1=0
  //D2 Normal 0 inverted 1 clock for ADC data acceptance
  //D1 enable external frame detection
  //D0 enable internal frame detection

  write_conf(0x0000);//PHOS power off 

  write_conf(0x160);//PHOS power on,turn on reset apv 101100000 
  
  write_conf(0x060);//PHOS power on,turn off reset apv  001100000: 
  

  //write dummy I2C and this is reset signal
  phos_i2c_write(99,1,BROADCAST);

  delay=10;

  phos_i2c_write(0 ,(1<<5)|delay,BROADCAST);//delayline=1:ADC clock (ns) 

  delay=0;

  phos_i2c_write(0 ,(3<<5)|delay,BROADCAST); //delayline=3:Trg In (ns)
 
  printf("Reset PHOS\n");  

  return ;
}

void set_phosdelay(bool fifo_clk_pol,int adc_clk_delay,int trg_in_delay,unsigned long module)
{
  // fifo_clk_pol 0:normal 1:inverted

  //D8 RESET_APV25
  //D7 APV25_POWER on
  //D6 PHOS4 power off
  //D5 Shift register 0 or Sequncer 1
  //D4 ADC_S2=0
  //D3 ADC_S1=0
  //D2 Normal 0 inverted 1 clock for ADC data acceptance
  //D1 enable external frame detection
  //D0 enable internal frame detection
  write_conf(0x0000);//PHOS power off 

  write_conf(0x160);//PHOS power on,turn on reset apv 101100000 
  
  write_conf(0x060|(fifo_clk_pol<<2));//PHOS power on,turn off reset apv  001100000: 

  //write dummy I2C and this is reset signal
  phos_i2c_write(99,1,module);
  
  //delayline=1:ADC clock (ns)
  phos_i2c_write(0 ,(1<<5)|adc_clk_delay,module);

  //delayline=3:Trg In (ns)
  //only master module can adjust trg delay
  phos_i2c_write(0 ,(3<<5)|trg_in_delay,module);
  
  printf("set PHOS Delay\n");  

  return ;
}



void init_altera(unsigned long module)
{
  long data1,data2;
  data1 = SET;//1
  vwrite32(module,WRITE_NCONFIG,&data1);  //set START_LD_ALT1
  msleep(1);//sleep 1 second

  data2 = UNSET;//0
  vwrite32(module,WRITE_NCONFIG,&data2);  //unset START_LD_ALT1
  printf("Intialize ALTERA\n");  
  ssleep(1);


  return ;
}



void clear_fifo(void)
{
  long maskdata;
  maskdata = 0x80000000;
  vwrite32(BROADCAST,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(MASTER,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE1,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE2,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE3,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE4,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE5,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE6,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE7,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE8,WRITE_LRES,&maskdata);// clear FIFO
  //vwrite32(SLAVE9,WRITE_LRES,&maskdata);// clear FIFO
 // printf("CLEAR FIFO\n");

  return ;
}
void write_cnt(long cnt_word,unsigned long module)
{
  vwrite32(module,WRITE_CNT,&cnt_word);
  printf("SET CNT Register\n");  
  
  return ;
}

void veto_on(unsigned int module)
{
  long vetostatus;
  vetostatus = SET; // WRITE_VETO set
  
  vwrite32(module,WRITE_VETO,&vetostatus);
  printf("VETO ON\n");  
  return ;
}
 


void veto_clear(void)
{
  long vetostatus;
  vetostatus = UNSET;

  vwrite32(BROADCAST,WRITE_VETO,&vetostatus);

  printf("VETO CLEAR\n");  
  return ;
}



bool is_veto(unsigned int module)
{
  long vetostatus;
  vread32(module,READ_VETO,&vetostatus); 

  if(vetostatus == 1)printf( "READ_VETO: ON\n");
  if(vetostatus == 0)printf( "READ_VETO: OFF\n");

  return vetostatus;
}




void write_seq_ram(void)
{
  long seqramdata[SEQUENCER_LENGTH];//256
  int i;
  for(i=0;i<SEQUENCER_LENGTH;i++) seqramdata[i]= 0;

#ifdef SINGLE_PEAK_MODE  
  //SW_TRG and HW_TRG at cell 83
  seqramdata[SINGLE_SEQ_CELL] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
#endif

#ifdef MULTI_PEAK_MODE
  //SW_TRG and HW_TRG at cell 78,81,84,87
  //seqramdata[MULTI_SEQ_CELL1] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
  seqramdata[MULTI_SEQ_CELL2] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
  seqramdata[MULTI_SEQ_CELL3] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
  seqramdata[MULTI_SEQ_CELL4] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
#endif

#ifdef CALIBRATION_MODE
  //SW_TRG and HW_TRG at cell 78,81,84,87
 // seqramdata[CALIBRATION_SEQ_CELL1] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
  seqramdata[CALIBRATION_SEQ_CELL2] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
  seqramdata[CALIBRATION_SEQ_CELL3] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
  seqramdata[CALIBRATION_SEQ_CELL4] |= (HW_TRG_SEQUENCE | SW_TRG_SEQUENCE);
#endif
 
  //SW_CAL this is common setting. do not change.
  seqramdata[2] |= SW_CAL_SEQUENCE;
  seqramdata[3] |= SW_CAL_SEQUENCE;
  seqramdata[250] |= SW_CAL_SEQUENCE;
  seqramdata[251] |= SW_CAL_SEQUENCE;

  //SW_RES common setting. do not change.
  seqramdata[2] |= SW_RES_SEQUENCE;
  seqramdata[4] |= SW_RES_SEQUENCE;

  for(i=0;i<SEQUENCER_LENGTH;i++){
    vwrite32(BROADCAST,(WRITE_SEQ_RAM + (i<<2)),&seqramdata[i]);
  }
  return ;
}



void write_dac_ram(long seq)
{
  int i;
  for(i=0;i<SEQUENCER_LENGTH ;i++)//256
    {
      vwrite32(BROADCAST,(WRITE_DAC_RAM +(i<<2)),&seq);
    }

  return;
}



void write_trigger_seq(long softbits)
{
  //broadcast mode
  //launch sequences synchronously
  vwrite32(BROADCAST,WRITE_TRG_SEQ,&softbits);
  if(softbits == 8)printf( "initiate DACRAM sequence\n");
  return ;
}



void init_dac_ram(void)
{
  write_trigger_seq(8);
  return;
}

void write_soft_trigger(void)
{
  write_trigger_seq(4);
  
  return;
}

void write_apv_caltrig(void)
//write internal calibration request and subsequent trigger
{
  write_trigger_seq(6);
  
  return;
}

void write_testtrigger(unsigned long module,long testrig)
{
  vwrite32(module,WRITE_TRG,&testrig);
  //  vwrite32(SLAVE4,WRITE_TRG,&testrig);
  printf( "TEST TRIGGER\n");
  return ;
}

void read_2fifo_pio(unsigned long module)
{
  unsigned long dataword;
  //unsigned long adc_A,adc_B,adc_C,adc_D;
  unsigned long adc_A[1024],adc_B[1024],adc_C[1024],adc_D[1024];
  int i;
  int data ;

  data_buffer &g_dbuf = data_buffer::get_instance();

  if(module==MASTER){ g_dbuf.add_data(0x0c000000); }
  if(module==SLAVE1){ g_dbuf.add_data(0x1c000000); }
  if(module==SLAVE2){ g_dbuf.add_data(0x2c000000); }
  if(module==SLAVE3){ g_dbuf.add_data(0x3c000000); }

  if(module==SLAVE4){ g_dbuf.add_data(0x4c000000); }
  if(module==SLAVE5){ g_dbuf.add_data(0x5c000000); }
  if(module==SLAVE6){ g_dbuf.add_data(0x6c000000); }
  if(module==SLAVE7){ g_dbuf.add_data(0x7c000000); }

  static unsigned int read_count = 0;
  static unsigned int cnt =0;
  for(i=0;i<200;i++){
    vread32_uint(module,READ_FIFO_TEMP_A,&dataword);

    g_dbuf.add_data(dataword);

    adc_A[i] = dataword&0x3ff;
    adc_B[i] = (dataword>>16)&0x3ff;    

    //    printf("0x%x : adc_A[%d] = %d, adc_B[%d] = %d\n",module,i,adc_A[i],i,adc_B[i]);

    //adc_A[i] = dataword&0x3ff;
    //adc_B[i] = (dataword>>16)&0x3ff;    
    //data = 0 ;
    //data |= adc_A ;
    //data |= (adc_B<<16) ;
    //g_dbuf.add_data(data) ;
  }

  
  for(i=0;i<60;i++)
    {
      vread32_uint(module,READ_FIFO_TEMP_B,&dataword);
      g_dbuf.add_data(dataword);
      //adc_C = dataword&0x3ff;
      //adc_D = (dataword>>16)&0x3ff;
      //printf("0x%x : adc_C[%d] = %d, adc_D[%d] = %d\n",module,i,adc_C,i,adc_D);

      adc_C[i] = dataword&0x3ff;
      adc_D[i] = (dataword>>16)&0x3ff;

      //      printf("0x%x : adc_C[%d] = %d, adc_D[%d] = %d\n",module,i,adc_C[i],i,adc_D[i]);

      //data = 0;
      //data |= adc_C;
      //data |= (adc_D<<16);
      //data |= (adc_B<<16) ;
      //g_dbuf.add_data(data) ;
    }


    {
      std::ofstream ofs;
      std::stringstream ss;
      cnt++;
      ss<<"read"<<cnt<<".txt";
      ofs.open(ss.str().c_str());
      for(i=0;i<300;i++)
	{
	  ofs<<"adcA_["<<i<<"]= "<<adc_A[i]<<", adc_B["<<i<<"] = "<<adc_B[i]<<"\n";
	  ofs<<"adc_C["<<i<<"]= "<<adc_C[i]<<", adc_D["<<i<<"] = "<<adc_D[i]<<"\n";

	}
      ofs.close();
      
    }
  read_count++;

  //clear_fifo();
  return;
}
void switch_buffer(){
  static unsigned int buffer_address = 0;
  long dataword = (0x2)|(0x1&buffer_address);
  vwrite32(BROADCAST,BUFFERSWITCH,&dataword);
  buffer_address++;
  std::cout<<"switching buffer"<<std::endl;
  return ;
}
void switch_buffer(unsigned long module, unsigned long buffer_address){
  long dataword = (0x2)|(0x1&buffer_address);
  vwrite32(module,BUFFERSWITCH,&dataword);
  std::cout<<"switching buffer"<<std::endl;
  return ;
}

unsigned long read_buffer_address(unsigned long module)
{
  unsigned long buffer_address;
  vread32_uint(module,BUFFERSWITCH,&buffer_address);
  return (buffer_address & 0x1);
}

void read_fifo_ZS(unsigned long module)
{
  unsigned long dataword;
  unsigned long adc_A,adc_B,adc_C,adc_D;
  unsigned long SP1,SP2,SP3,SP4,SP5,SP6,SP7,SP8,CHANNEL,DEV,FLAG,CHIP;
  unsigned long datasize1,datasize2,datasize3,datasize4;
  unsigned long flag;
  int i;
  int data ;
  int avg,devsum;

  data_buffer &g_dbuf = data_buffer::get_instance();


  if(module==MASTER){ g_dbuf.add_data(0x0c000000); }
  if(module==SLAVE1){ g_dbuf.add_data(0x1c000000); }
  if(module==SLAVE2){ g_dbuf.add_data(0x2c000000); }
  if(module==SLAVE3){ g_dbuf.add_data(0x3c000000); }

  if(module==SLAVE4){ g_dbuf.add_data(0x4c000000); }
  if(module==SLAVE5){ g_dbuf.add_data(0x5c000000); }
  if(module==SLAVE6){ g_dbuf.add_data(0x6c000000); }
  if(module==SLAVE7){ g_dbuf.add_data(0x7c000000); }

  static unsigned int cnt = 0;
  const int itr = 1;
  std::string message = "SSD FIFO Reading";
  for(int i=0;i<itr;i++)
    {
      cnt++;
      if(!cnt) send_status_message(message);
    }



  vread32_uint(module,READ_FIFO_SIZE,&dataword);
  g_dbuf.add_data(dataword);

   datasize1=dataword&0xff;
   datasize2=(dataword>>8)&0xff;
   datasize3=(dataword>>16)&0xff;
   datasize4=(dataword>>24)&0xff;
  if(datasize1==0&&datasize2==0&&datasize3==0&&datasize4==0)
  //if(datasize1!=128||datasize2!=128||datasize3!=128||datasize4!=128)
    {
      //      read_2fifo_pio(module);
      std::cout<<"module="<< std::hex <<module<<std::endl;
      std::cout<<"datasize1="<< std::dec <<datasize1<<std::endl;
      std::cout<<"datasize2="<< std::dec <<datasize2<<std::endl;
      std::cout<<"datasize3="<< std::dec <<datasize3<<std::endl;
      std::cout<<"datasize4="<< std::dec <<datasize4<<std::endl;
    }
  
//   std::cout<<"CHIP 1 START\n"<<std::endl;
  
  for(i=0;i<datasize1;i++)
    {
        for(int i=0;i<itr;i++)
    {
      cnt++;
      if(!cnt) send_status_message(message);
    }

      vread32_uint(module,READ_FIFO_AA,&dataword);
      g_dbuf.add_data(dataword);
      CHIP = (dataword>>30)&0x3;
      SP1 = dataword&0x3ff;
      SP2 = (dataword>>10)&0x3ff;
      CHANNEL = (dataword>>20)&0x3ff;

      vread32_uint(module,READ_FIFO_AB,&dataword);
      g_dbuf.add_data(dataword);
      SP3 = dataword&0x3ff;
      SP4 = (dataword>>10)&0x3ff;
      SP5 = (dataword>>20)&0x3ff;


      vread32_uint(module,READ_FIFO_AC,&dataword);
      g_dbuf.add_data(dataword);
      SP6 = dataword&0x3ff;
      SP7 = (dataword>>10)&0x3ff;
      SP8 = (dataword>>20)&0x3ff;
      FLAG = (dataword>>30)&0x3;
      //avg = (SP1+SP2+SP3+SP4+SP5+SP6+SP7+SP8)/8;
      //devsum = abs(SP1-avg)+abs(SP2-avg)+abs(SP3-avg)+abs(SP4-avg)+abs(SP5-avg)+abs(SP6-avg)+abs(SP7-avg)+abs(SP8-avg);
      
      //if((devsum>124)&&SP4>SP1&&SP4>SP2&&SP4>SP7&&SP4>SP8&&SP3>SP1&&SP3>SP8&&SP5>SP8&&FLAG==0)	  
      //    if(FLAG==1)printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
      //      if(i%32==0)
      //printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);

    }
  
  
//   std::cout<<"CHIP 2 START\n"<<std::endl;
  for(i=0;i<datasize2;i++)
    {
  for(int i=0;i<itr;i++)
    {
      cnt++;
      if(!cnt) send_status_message(message);
    }

      vread32_uint(module,READ_FIFO_BA,&dataword);
      g_dbuf.add_data(dataword);
      CHIP = (dataword>>30)&0x3;
      SP1 = dataword&0x3ff;
      SP2 = (dataword>>10)&0x3ff;
      CHANNEL = (dataword>>20)&0x3ff;

      vread32_uint(module,READ_FIFO_BB,&dataword);
      g_dbuf.add_data(dataword);
      SP3 = dataword&0x3ff;
      SP4 = (dataword>>10)&0x3ff;
      SP5 = (dataword>>20)&0x3ff;

      vread32_uint(module,READ_FIFO_BC,&dataword);
      g_dbuf.add_data(dataword);
      SP6 = dataword&0x3ff;
      SP7 = (dataword>>10)&0x3ff;
      SP8 = (dataword>>20)&0x3ff;
      FLAG = (dataword>>30)&0x3;
     
      //avg = (SP1+SP2+SP3+SP4+SP5+SP6+SP7+SP8)/8;
      //devsum = abs(SP1-avg)+abs(SP2-avg)+abs(SP3-avg)+abs(SP4-avg)+abs(SP5-avg)+abs(SP6-avg)+abs(SP7-avg)+abs(SP8-avg);
      
      //if((devsum>124)&&SP4>SP1&&SP4>SP2&&SP4>SP7&&SP4>SP8&&SP3>SP1&&SP3>SP8&&SP5>SP8&&FLAG==0)	  
      //   if(FLAG==1)printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
      //      if(i%32==0)
      //	printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
    }
  
//   std::cout<<"CHIP 3 START\n"<<std::endl;
  for(i=0;i<datasize3;i++)
    {
  for(int i=0;i<itr;i++)
    {
      cnt++;
      if(!cnt) send_status_message(message);
    }

      vread32_uint(module,READ_FIFO_CA,&dataword);
      g_dbuf.add_data(dataword);
      CHIP = (dataword>>30)&0x3;
      SP1 = dataword&0x3ff;
      SP2 = (dataword>>10)&0x3ff;
      CHANNEL = (dataword>>20)&0x3ff;

      vread32_uint(module,READ_FIFO_CB,&dataword);
      g_dbuf.add_data(dataword);
      SP3 = dataword&0x3ff;
      SP4 = (dataword>>10)&0x3ff;
      SP5 = (dataword>>20)&0x3ff;

      vread32_uint(module,READ_FIFO_CC,&dataword);
      g_dbuf.add_data(dataword);
      SP6 = dataword&0x3ff;
      SP7 = (dataword>>10)&0x3ff;
      SP8 = (dataword>>20)&0x3ff;
      FLAG = (dataword>>30)&0x3;
      
      //avg = (SP1+SP2+SP3+SP4+SP5+SP6+SP7+SP8)/8;
      //devsum = abs(SP1-avg)+abs(SP2-avg)+abs(SP3-avg)+abs(SP4-avg)+abs(SP5-avg)+abs(SP6-avg)+abs(SP7-avg)+abs(SP8-avg);
    
      //if((devsum>124)&&SP4>SP1&&SP4>SP2&&SP4>SP7&&SP4>SP8&&SP3>SP1&&SP3>SP8&&SP5>SP8&&FLAG==0)	  
      //if(FLAG==1)printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
      //      if(i%32==0)
      //	printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
     
    }
  
  
//   std::cout<<"CHIP 4 START\n"<<std::endl;
  for(i=0;i<datasize4;i++)
    {
  for(int i=0;i<itr;i++)
    {
      cnt++;
      if(!cnt) send_status_message(message);
    }

      vread32_uint(module,READ_FIFO_DA,&dataword);
      g_dbuf.add_data(dataword);
      CHIP = (dataword>>30)&0x3;
      SP1 = dataword&0x3ff;
      SP2 = (dataword>>10)&0x3ff;
      CHANNEL = (dataword>>20)&0x3ff;


      vread32_uint(module,READ_FIFO_DB,&dataword);
      g_dbuf.add_data(dataword);
      SP3 = dataword&0x3ff;
      SP4 = (dataword>>10)&0x3ff;
      SP5 = (dataword>>20)&0x3ff;

      vread32_uint(module,READ_FIFO_DC,&dataword);
      g_dbuf.add_data(dataword);
      SP6 = dataword&0x3ff;
      SP7 = (dataword>>10)&0x3ff;
      SP8 = (dataword>>20)&0x3ff;
      FLAG = (dataword>>30)&0x3;
      //avg = (SP1+SP2+SP3+SP4+SP5+SP6+SP7+SP8)/8;
      //devsum = abs(SP1-avg)+abs(SP2-avg)+abs(SP3-avg)+abs(SP4-avg)+abs(SP5-avg)+abs(SP6-avg)+abs(SP7-avg)+abs(SP8-avg);
      
      //if((devsum>124)&&SP4>SP1&&SP4>SP2&&SP4>SP7&&SP4>SP8&&SP3>SP1&&SP3>SP8&&SP5>SP8&&FLAG==0)	  
      //if(FLAG==1)printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
      //	  if(i%32==0)
      //	    printf("0x%x : CHIP = %d,SP1[%d] = %d,SP2[%d] = %d,SP3[%d] = %d,SP4[%d] = %d, SP5[%d] = %d, SP6[%d] = %d, SP7[%d] = %d, SP8[%d] = %d,FLAG = %d\n",module,CHIP,CHANNEL,SP1,CHANNEL,SP2,CHANNEL,SP3,CHANNEL,SP4,CHANNEL,SP5,CHANNEL,SP6,CHANNEL,SP7,CHANNEL,SP8,FLAG);
    }
  return;
}


// 18 Aug 2015 at Tokai

void FIR_ON()
{
  long config = 0x0;
  config = config | 0x1;
  config = config | RAWZSSEL<<1;
  config = config | ((L2TRG&0x1)<<2);

  std::cout<<"FIR ON data: "<<std::hex<<config<<std::endl;
  vwrite32(BROADCAST,WRITE_USER_CONF,&config);
  return;

}
void FIR_OFF()
{
  long config = 0x0;
  config = config | 0x0;
  config = config | RAWZSSEL<<1;
  config = config | ((L2TRG&0x1)<<2);

  std::cout<<"FIR OFF data: "<<std::hex<<config<<std::endl;
  vwrite32(BROADCAST,WRITE_USER_CONF,&config);
  return;

}


void WaitDReadySsd(unsigned long module)
{
  unsigned long dready = 0;
  //unsigned long dready = 1;
  unsigned int count = 0;
  static unsigned int cnt = 0;
  std::string message = "Polling APVDAQ";
  static unsigned int fail_count = 0;
  while(!dready)
    {
      for(int i=0;i<10000;i++)
	{
	  cnt++;
	  if(!cnt) send_status_message(message);
	}
      if(dready==0)
	{
	  vread32_uint(module,READ_DREADY,&dready);
	}
      if(dready==1)   
	{
	  std::cout<<"dready bit:"<<dready<<std::endl;
	  fail_count = 0;
	  break;
	}
      if(count == 30)
	{
	  std::cerr<<"TIMEOUT!!!!!!!!!!!!!!!!!!!!!!!!!!!"<<std::endl;
	  std::stringstream ss;
	  ss<<"Wait SSD Time Out!! Module"<<std::hex<<module;
	  
	  send_warning_message(ss.str());
	  if(fail_count >= 10) 
	    {
	      std::string fatal_message = "SSD DReady Fail Exit Frontend!";
	      send_fatal_message(fatal_message);
	      send_fatal_message(ss.str());
	      exit(-1);
	    }
	  fail_count++;
	  break;
	}

      ++count;
    }
}
void write_fir_trigger()
{
  long dummy = 0;
  vwrite32(BROADCAST,FIR_CALIB_TRIGGER,&dummy);
  printf( "FIR Calibration Trigger\n");
  return ;
}
void FIRZS_CONFIGURATION() // CONFIGURATION of FIR and ZS test on/off                                    
{
  long config = 0x0;
  config = config | (FIRONOFF&0x1);
  config = config | ((RAWZSSEL&0x1)<<1);
  config = config | ((L2TRG&0x1)<<2);

  vwrite32(BROADCAST,WRITE_USER_CONF,&config);
  return;
}


//=====================================                                                                  
SsdParam& SsdParam::get_instance(void)
{
  static SsdParam ssd_param;
  return ssd_param;
}

//=====================================                                                                  
SsdParam::SsdParam()
{
  memset(mean,0,sizeof(mean));
  memset(rms,0,sizeof(rms));
  memset(DevMean,0,sizeof(DevMean));
  memset(DevRms,0,sizeof(DevRms));
}
//=====================================                                                                  
void SsdParam::ClearData()
{
  memset(mean,0,sizeof(mean));
  memset(rms,0,sizeof(rms));
  memset(DevMean,0,sizeof(DevMean));
  memset(DevRms,0,sizeof(DevRms));
}
//=====================================                                                                  
void SsdParam::ReadPedParam()
{
  std::ifstream ifs1;
  //std::ifstream ifs1,ifs2;
  std::string filename1 = "/home/sks/jaeyong_work/parameter/MeanRms.txt";
  //std::string filename2 = "/home/sks/jaeyong_work/parameter/DevMeanRms.txt";

  ifs1.open(filename1.c_str());
  //ifs2.open(filename2.c_str());
  if(ifs1.fail()) 
  {
    std::cerr<<"#E file open fail Read Mean RMS"<<std::endl;
    std::string message = "File open fail! Check Parameter File, Location: ";
    message+=filename1;
    send_fatal_message(message);
  }
  //if(ifs2.fail()) std::cerr<<"#E file open fail Read DevMeanRMS"<<std::endl;

  for( int i = 0; i <APVDAQNUMB; i++ )//module                                                           
    for( int j = 0; j <APVCHIPNUMB; j++ )//chip                                                          
      for( int k = 0; k < 128; k++)//channel                                                             
        for( int l = 0; l < NumberOfSamples; l++)//sample                                                
          {
            ifs1>>mean[i][j][k][l]>>rms[i][j][k][l];
#if 0
	    std::cout<<"mean["<<i<<"]["<<j<<"]["<<k<<"]["<<l<<"] = "
                     <<mean[i][j][k][l]<<std::endl;
	    std::cout<<"rms["<<i<<"]["<<j<<"]["<<k<<"]["<<l<<"] = "
                     <<rms[i][j][k][l]<<std::endl;
            //      getchar();                                                                           
#endif
          }
/*
  for( int i = 0; i <APVDAQNUMB; i++ )//module                                                           
    for( int j = 0; j <APVCHIPNUMB; j++ )//chip                                                          
      for( int k = 0; k < 128; k++)//channel                                                             
        {
          ifs2>>DevMean[i][j][k]>>DevRms[i][j][k];
#if 0
	  std::cout<<"DevMean["<<i<<"]["<<j<<"]["<<k<<"] = "
                   <<DevMean[i][j][k]<<std::endl;
	  std::cout<<"DevRms["<<i<<"]["<<j<<"]["<<k<<"] = "
                   <<DevRms[i][j][k]<<std::endl;
          //     getchar();                                                                              
#endif
        }
*/
  ifs1.close();
  //ifs2.close();
}
//=====================================                                                                  
void SsdParam::ReadThresParam()
{
  std::ifstream ifs1;
  std::string filename1 = "/home/sks/jaeyong_work/parameter/Threshold.txt";

  ifs1.open(filename1.c_str());
  if(ifs1.fail())
    {
      std::cerr<<"#E file open fail Read Mean RMS"<<std::endl;
      std::string message = "File open fail! Check Parameter File, Location: ";
      message+=filename1;
      send_fatal_message(message);
      exit(-1);
    }

  for( int i = 0; i <APVDAQNUMB; i++ )//module
    ifs1>>ThresParam[i];
  ifs1>>MaxPedThreshold;

  for( int i = 0; i <APVDAQNUMB; i++ )//module
    ifs1>>ThresParam2[i];

  std::stringstream ss;
  ss<<"Threshold: ";
  for( int i = 0; i <APVDAQNUMB; i++ )
    ss<<"\t"<<ThresParam[i];
  ss<<"\tMaxThreshold: "<<MaxPedThreshold;

  ss<<"\nThreshold2: ";
  for( int i = 0; i <APVDAQNUMB; i++ )
    ss<<"\t"<<ThresParam2[i];


  send_normal_message(ss.str());

  ifs1.close();
}
//=====================================                                                                  
void SsdParam::WritePedParamToFpga()
{
  unsigned long channel,chip;
  unsigned long OffAddCh, OffAddChip;
  unsigned long OffsetAddress;
  unsigned long Sadc[9] = {0};
  double threshold[6],threshold2[6];;
  for( int module = 0; module < APVDAQNUMB; ++module )
    {    threshold[module]=ThresParam[module];    threshold2[module]=ThresParam2[module]; }
  for( int module = 0; module < APVDAQNUMB; ++module )
    for( chip = 0;chip < APVCHIPNUMB; chip++ )
      for( channel = 0; channel < 128; channel++ )
        {
          for(int sample = 0; sample < NumberOfSamples; ++sample)
            {
              double rms_threshold;
              if((channel>=32&&channel<48)||(channel>=96&&channel<112))
                rms_threshold = threshold2[module]*rms[module][chip][channel][sample];
              else
                rms_threshold = threshold[module]*rms[module][chip][channel][sample];
              if(rms_threshold >=MaxPedThreshold) rms_threshold = MaxPedThreshold;
              Sadc[sample] = (unsigned long)(mean[module][chip][channel][sample]+rms_threshold);
            }
          for( int i = 0; i < 3 ;i++ )//RAM Selector                                                        \

            {
              unsigned int address = (module<<24);
              unsigned long ch7bit = 0;
              if(channel>=64) ch7bit = 1;
              unsigned long data;
              OffAddCh = (channel&0x3F)<<2;
              OffAddChip = (chip&0x3)<<8;
              OffsetAddress = WRITE_PEDESTAL_ADC | OffAddChip | OffAddCh ;

              data = (((i+1)&0x7)<<29) | ((ch7bit&0x1)<<27) | (Sadc[3*i]&0x1ff) | ((Sadc[3*i+1]&0x1ff)<<9) |\
		((Sadc[3*i+2]&0x1ff)<<18);

              vwrite32_uint(MASTER+address,OffsetAddress,&data);
            }
        }
}

void SsdParam::WriteDevParamToFpga()
{
  unsigned long channel,chip;
  unsigned long OffAddCh;
  unsigned long OffsetAddress;
  double threshold = DevThreshold;
  unsigned int DevData[4] = {0};

  for( int module = 0; module < APVDAQNUMB; ++module )
    for( channel = 0; channel < 128; channel++ )
      {
	for( chip = 0;chip < APVCHIPNUMB; chip++ )
	  {
	    unsigned int dev_adc_threshold = (unsigned int)(DevMean[module][chip][channel]+threshold*DevRms[module][chip][channel]); 
	    if(dev_adc_threshold > 250) DevData[chip] = 250;
	    else DevData[chip] = dev_adc_threshold;                                                                         
	  }
	unsigned int address = (module<<24);
	unsigned long data;
	OffAddCh = (channel&0x7F)<<2;
	OffsetAddress = WRITE_DEV_THRESHOLD | OffAddCh ;

	data = (DevData[0]&0xff) | ((DevData[1]&0xff)<<8) | ((DevData[2]&0xff)<<16)| ((DevData[3]&0xff)<<24);

	vwrite32_uint(MASTER+address,OffsetAddress,&data);
      }
}
void SsdParam::WritePulseShapeParamToFpga()
{
  unsigned long channel,chip;
  unsigned long OffsetAddress;

  for( int module = 0; module < APVDAQNUMB; ++module )
    for( chip = 0;chip < APVCHIPNUMB; chip++ )
      for( int i = 0; i < 2 ; ++ i)
        {
	  unsigned long pulse_param = PulseShapeParameter[chip][i];
          unsigned int address = (module<<24);
          OffsetAddress =  WRITE_PULSE_SHAPE_PARAMETER | ( ((chip*2 + i)&0x7) << 2 );
          vwrite32_uint(MASTER+address,OffsetAddress,&pulse_param);
        }
}

