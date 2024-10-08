#include"FPGAModule.hh"
#include"UDPRBCP.hh"
#include<iostream>

FPGAModule::FPGAModule(char* ipAddr, unsigned int port, rbcp_header* sendHeader,
		       int disp_mode)
  :
  ipAddr_(ipAddr),
  port_(port),
  sendHeader_(sendHeader),
  disp_mode_(disp_mode)
{

}

FPGAModule::~FPGAModule()
{

}

int FPGAModule::WriteModule(unsigned int module_id,
			    unsigned int local_address,
			    unsigned int write_data
			    )
{
  unsigned int udp_addr 
    = ((module_id & module_id_mask) << module_id_shift)
    + ((local_address & address_mask) << address_shift)
    + ((write_data & exdata_mask) >> exdata_shift);

  char udp_wd = static_cast<char>(write_data & data_mask);
  
  UDPRBCP udpMan(ipAddr_, port_, sendHeader_,
		 static_cast<UDPRBCP::rbcp_debug_mode>(disp_mode_));
  udpMan.SetWD(udp_addr, 1, &udp_wd);
  return udpMan.DoRBCP();
}

int FPGAModule::ReadModule(unsigned int module_id,
			   unsigned int local_address,
			   unsigned int length
			   )
{
  rd_data_.clear();
  unsigned int udp_addr 
    = ((module_id & module_id_mask) << module_id_shift)
    + ((local_address & address_mask) << address_shift);

  UDPRBCP udpMan(ipAddr_, port_, sendHeader_,
		 static_cast<UDPRBCP::rbcp_debug_mode>(disp_mode_));
  udpMan.SetRD(udp_addr, length);
  int ret;
  if((ret = udpMan.DoRBCP()) > -1){ udpMan.CopyRD(rd_data_); }

  return ret;
}

unsigned int FPGAModule::ReadModule_nCycle(unsigned int module_id,
					   unsigned int local_address,
					   int ncycle
					   )
{
  if(ncycle > 4){
    std::cerr << "#E :FPGAModule::ReadModule_nCycle, too many cycle " 
	      << ncycle << std::endl;
    return 0xeeeeeeee;
  }

  unsigned int data = 0;
  for(int i = 0; i<ncycle; ++i){
    if( this->ReadModule(module_id, local_address+i, 1) > -1){
      unsigned int tmp = (unsigned int)rd_data_[0];
      data += (tmp & 0xff) << 8*i;
    }else{
      return 0xeeeeeeee;
    }
  }

  rd_word_ = data;
  return rd_word_;
}
