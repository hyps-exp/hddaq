#include "fir_calibration.hh"

#include "apvdaq_param.hh"
#include "apvdaq.hh"
#include "apvdaq_function.hh"

#include <iostream>
#include <fstream>

//=====================================
FirCalib& FirCalib::get_instance(void)
{
  static FirCalib fir_calib;
  return fir_calib;
}

//=====================================
FirCalib::FirCalib()
{
  memset(fir_adc,0,sizeof(fir_adc));
  memset(firconst,0,sizeof(firconst));
}
//=====================================
void FirCalib::ClearData()
{
  memset(fir_adc,0,sizeof(fir_adc));
  memset(firconst,0,sizeof(firconst));
}
//=====================================
void FirCalib::StartCalibration()
{
  std::cout<<"Start FIR Parameter Calibration"<<std::endl;
  
  std::cout<<"BLOCKING EXTERNAL TRIGGER ... SETTING VETO = 1 ..."<<std::endl;
  
  veto_on(BROADCAST);
  usleep(10);
  std::cout<<"Check Veto Status"<<std::endl;
  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      printf("Module %d:\t",module);
      if(is_veto(MASTER+address)) std::cout<<"VETO_ON"<<std::endl;
      else std::cout<<"VETO_OFF"<<std::endl;
    }

  for(int event = 0; event<MAXFIRCAL; ++event)
    {
      int max_try = 0;
      while(1)
	{
	  if(max_try >= 10) 
	    {
	      std::cerr<<"#E fail to find tick mark exiting"<<std::endl;
	      exit(-1);
	    }

	  clear_fifo();
	  usleep(10);
	  veto_clear();
	  while(1)
	    {
	      write_testtrigger(BROADCAST,1);//Synchronized Trigger Only for Chip Idling
	      usleep(200);
	      int veto_status_count = 0;
	      for (int  module = 0; module < APVDAQNUMB; ++module )
		{
		  unsigned int address = (module<<24);
		  if(is_veto(MASTER+address)) veto_status_count++;
		}

	      if(veto_status_count == APVDAQNUMB) break;
	    }
	  int count = 0;	  
	  for (int  module = 0; module < APVDAQNUMB; ++module )
	    {
	      unsigned int address = (module<<24);
	      if(TickSearch(MASTER+address)==true) count++;//Search For Clock Idling Tick Mark
	      else std::cerr<<"#E MODULE #"<<module+1<<" Tick Mark Search Fail Try to Adjust Phos 4 Delay"<<std::endl;
	    }
	  
	  if( !(count == APVDAQNUMB) ) 
	    {
	      max_try ++;
	      continue;
	    }

	  if(ReadFirData()==false)  	      
	    {
	      max_try ++;
	      continue;
	    }
	  else break;
	}
      
      if(event%1000==999) std::cout<<std::dec<<"FIR Calibration Proccessing Event # = "<<event+1<<std::endl;
    }
}
//=====================================
bool FirCalib::TickSearch(unsigned int module)
{
  int adc[4]={0};
  unsigned long dataword;
  int j;

  //Search Tick Mark
  for( j = 1; j <= 36; j++)
    {
      if(j == 36) 
	{
	  std::cerr<<"#E Tick Search Fail, Try to adjust Phos4 delay for module: "<<std::hex<<module<<std::endl;
	  return false;
	}
      vread32_uint(module,READ_FIFO_TEMP_A,&dataword);
      adc[0] = dataword&0x3ff;
      adc[1] = (dataword>>16)&0x3ff;

      vread32_uint(module,READ_FIFO_TEMP_B,&dataword);
      adc[2] = dataword&0x3ff;
      if(APVCHIPNUMB==4)
	adc[3] = (dataword>>16)&0x3ff;
      else if (APVCHIPNUMB==3)
	adc[3] = adc[2];
      else std::cerr<<"#E APV Chip number setting is wrong"<<std::endl;
      //            printf("0x%x : adc_A[%d] = %d, adc_B[%d] = %d\t",module,j,adc[0],j,adc[1]);
      //            printf("0x%x : adc_C[%d] = %d, adc_D[%d] = %d\n",module,j,adc[2],j,adc[3]);

      if((adc[0]>=MAXADC)&&(adc[1]>=MAXADC)&&(adc[2]>=MAXADC)&&(adc[3]>=MAXADC)) 
	{    
	  //	  printf("0x%x : adc_A[%d] = %d, adc_B[%d] = %d\t",module,j,adc[0],j,adc[1]);
	  //	  printf("0x%x : adc_C[%d] = %d, adc_D[%d] = %d\n",module,j,adc[2],j,adc[3]);
	  break;
	}
    }
  
  //Read out 34 remaining adc values
  for( int k = 0; k < 34; ++k )
    {
      vread32_uint(module,READ_FIFO_TEMP_A,&dataword);
      adc[0] = dataword&0x3ff;
      adc[1] = (dataword>>16)&0x3ff;

      vread32_uint(module,READ_FIFO_TEMP_B,&dataword);
      adc[2] = dataword&0x3ff;
      if(APVCHIPNUMB==4)
	adc[3] = (dataword>>16)&0x3ff;
      else if (APVCHIPNUMB==3)
	adc[3] = adc[2];
      else std::cerr<<"#E APV Chip number setting is wrong"<<std::endl;
      //      printf("0x%x : adc_A[%d] = %d, adc_B[%d] = %d\t",module,j,adc[0],j,adc[1]);
      //      printf("0x%x : adc_C[%d] = %d, adc_D[%d] = %d\n",module,j,adc[2],j,adc[3]);

      if(   !((adc[0]<=MINADC)&&(adc[1]<=MINADC)&&(adc[2]<=MINADC)&&(adc[3]<=MINADC))   )
	{
	  std::cerr<<"#E Invalid Value in the 34 remaining adc values followed by a tick mark"<<std::endl;
	  printf("0x%x : adc_A[%d] = %d, adc_B[%d] = %d\t",module,k,adc[0],k,adc[1]);
	  printf("0x%x : adc_C[%d] = %d, adc_D[%d] = %d\n",module,k,adc[2],k,adc[3]);
	  return false;
	}

    } 
  return true;
}
//=====================================
bool FirCalib::ReadFirData()
{
  int adc[APVDAQNUMB][4][35]; unsigned long dataword;
  memset(adc,0,sizeof(adc));
  for (int  module = 0; module < APVDAQNUMB; ++module )
    {
      unsigned int address = (module<<24);
      
      for(int i = 0; i < 35; i++)
	{
	  vread32_uint(MASTER+address,READ_FIFO_TEMP_A,&dataword); 
	  adc[module][0][i] = dataword&0x3ff;  
	  adc[module][1][i] = (dataword>>16)&0x3ff;
	  //      printf("0x%x : adc_A[%d] = %d, adc_B[%d] = %d\n",module,k,adc[0],k,adc[1]);	  
	  vread32_uint(MASTER+address,READ_FIFO_TEMP_B,&dataword);
	  adc[module][2][i] = dataword&0x3ff;                                                                 
	  if(APVCHIPNUMB==4)                                                      
	    adc[module][3][i] = (dataword>>16)&0x3ff;                                                         
	  else if (APVCHIPNUMB==3)                                                                 
	    adc[module][3][i] = adc[module][2][i];                                                                       
	  else std::cerr<<"#E APV Chip number setting is wrong"<<std::endl;                         
	  
	  if(i == 0)
	    {
	      int count = 0;
	      for (int chip = 0; chip < APVCHIPNUMB; ++chip )
		if(adc[module][chip][i]>MAXADC) count++;
	      if(count!=APVCHIPNUMB) return false;
	    }
	  else
	    {
	      int count = 0;
	      for (int chip = 0; chip < APVCHIPNUMB; ++chip )                                                                  
		if(adc[module][chip][i]<MINADC) count++;
	      if(count!=APVCHIPNUMB) return false;
	    }                                                                                                    
	}
      for(int i = 0; i < 35; i++)
	for(int chip = 0; chip < APVCHIPNUMB; chip ++)
	  {
	    fir_adc[module][chip][i] += adc[module][chip][i];
	  }
    }
  return true;
}
//=====================================
void FirCalib::AvgFirData()
{
  for(int i=0;i<APVDAQNUMB;++i)
    for(int j=0;j<APVCHIPNUMB;++j)
      for(int k=0;k<35;++k)
	{
	  fir_adc[i][j][k] = fir_adc[i][j][k]/(double)MAXFIRCAL;
#if 1
	  std::cout<<"fir_adc["<<i<<"]["<<j<<"]["<<k<<"] = "<<fir_adc[i][j][k]<<std::endl;
#endif
	}
  
}
//=====================================
void FirCalib::CalFirParam()
{
  // calculate FIR coefficients and program them
  // input: double fir_adc[MAX_APV][35] (not normalized,
  // averaged ADC values, [34]=0, [33]=baseline)
  // output: double firconst[MAX_APV][FirNumOfParam] (signed fractional)
  unsigned char i, imax;
  int j,k,l;
  double max_fir_adc=0., sumcoeff=0., sum=0.;
  double vdelta[FirNumOfParam], hF[FirNumOfParam], h[34];
  double H[FirNumOfParam*FirNumOfParam];

  for (i=0; i<APVDAQNUMB; i++) // loop over module 
    {
      for(j=0; j<APVCHIPNUMB; j++) // apv chips
	{
	  for(k=0; k<33; k++) // subtract baseline and find peak
	    {
	      h[k]=fir_adc[i][j][k]-fir_adc[i][j][33]; // subtract baseline
	      //search max of fir_adc[MAX_APV][35]:
	      if (k==0)
		{
		  max_fir_adc=h[0];
		  imax=0;
		}
	      if (h[k]>max_fir_adc)
		{
		  max_fir_adc=h[k];
		  imax=k;
		}
	    }
	    for (k=0; k<FirNumOfParam; k++) // normalize
	    {
	      h[k]=h[k+imax]/max_fir_adc;
	    }
	  // initialize matrix H:
	    for (k=0; k<FirNumOfParam; k++)
	    {
	      for (l=0; l<=k; l++)
		{
		  H[k*FirNumOfParam+l]= h[k-l];
		}
	      for (l=k+1; l<FirNumOfParam; l++)
		{
		  H[k*FirNumOfParam+l]=0.;
		}
	    }
	  // initialize vector delta_i, hF:
	  memset(hF,0,sizeof(hF));
	  memset(vdelta,0,sizeof(vdelta));
	  vdelta[0]=1.;
	  // calculate filter coefficients (recursive inversion of
	  // triangular matrix):
	  sumcoeff=0.;
	  for (k=0; k<FirNumOfParam; k++)
	    {
	      sum=0;
	      for (l=0; l<k; l++)
		{
		  sum = sum+H[k*FirNumOfParam+l]*hF[l];
		}
	      hF[k] = 1./H[k*FirNumOfParam+k]*(vdelta[k]-sum);
	      sumcoeff += hF[k];
	    }
	  // normalize filter coefficients (sum of all coefficients := 1)
	  for(k=0; k<FirNumOfParam; k++)
	    {
	      firconst[i][j][k]=hF[k]/sumcoeff;
	    }
	}
    }
  
  //fir cost check
  for(i=0; i<APVDAQNUMB; i++){
    for(j=0; j<APVCHIPNUMB; j++){
      for(k=0; k<FirNumOfParam; k++){
	printf("mod %d chip %d channel %d const : %lf\n",i,j,k,firconst[i][j][k]) ;
      }
    }
  }

  return ;
}
//=====================================
void FirCalib::RecordFirParam()
{
  std::string filename = "FirParameter.txt";
  std::string filename_to_read = "FirParameterToRead.txt";
  std::cout<<"#D RecordMeanRMS()"<<std::endl;

  std::ofstream ofs,ofs_to_read;
  ofs.open(filename.c_str());
  ofs_to_read.open(filename_to_read.c_str());
  for(int module=0;module<APVDAQNUMB;++module)
    {
      ofs_to_read<<"Module: "<<module<<"\n";
      for(int chip=0;chip<APVCHIPNUMB;++chip)
	{
	  ofs_to_read<<"Chip: "<<chip<<"\n";
	  for(int k=0;k<FirNumOfParam;++k)
	    {
	      ofs<<firconst[module][chip][k]<<"\t";
	      ofs_to_read<<fir_adc[module][chip][k]<<"\t";
	    }
	  ofs_to_read<<"\n";
	  for(int k=0;k<FirNumOfParam;++k)
	    {
	      ofs_to_read<<firconst[module][chip][k]<<"\t";
	    }
	  ofs_to_read<<"\n";
	}
    }
  ofs.close();
  ofs_to_read.close();
}
//=====================================
void FirCalib::ReadFirParam()
{
  std::string filename = "/home/sks/jaeyong_work/parameter/FirParameter.txt";
  std::ifstream ifs;
  ifs.open(filename.c_str());
  if(ifs.fail()){ std::cerr<<"#E file open fail: "<<filename<<std::endl; exit(-1);}
  for(int module=0;module<APVDAQNUMB;++module)
    for(int chip=0;chip<APVCHIPNUMB;++chip)
      for(int k=0;k<FirNumOfParam;++k)
	ifs>>firconst[module][chip][k];
  ifs.close();
}
//=====================================
void FirCalib::WriteFirParamToFpga()
{
  //  std::cout << "Writing FIR Parameters on FPGA..." << std::endl ;
  unsigned long offset, data;

  for(unsigned long module=0 ; module<APVDAQNUMB ; module++)
    for(unsigned long chip=0 ; chip <APVCHIPNUMB ; chip++)
      for(unsigned long k = 0 ; k < FirNumOfParam ; k++)
	{
	  //module address setup
	  unsigned int address = (module<<24);
	  unsigned long offset = ( WRITE_FIR_PARA | ( (chip&0x3) << 6 ) | ( (k&0xf) << 2 ) );
	  unsigned long data = (int)(8192.0*firconst[module][chip][k]);

	  vwrite32_uint(MASTER+address,offset,&data); 
	}

}
