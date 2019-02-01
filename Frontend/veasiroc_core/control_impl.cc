#include<iostream>
#include<fstream>
#include<sstream>
#include<bitset>
#include<cstdio>
#include<cstdlib>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netinet/tcp.h>
#include<string.h>

#include"control_impl.hh"
#include"rbcp.h"
#include"my_endian.h"
#include"RegisterMap.hh"

namespace{
  // Local index -----------------------------------------
  const int i_easiroc1 = 0;
  const int i_easiroc2 = 1; 

  std::bitset<8> reg_easiroc1;
  std::bitset<8> reg_easiroc2;
  std::bitset<8> reg_module;
};

//_________________________________________________________________________
void
resetDirectControl(const std::string& ip)
{
  // reset direct control registers
  reg_easiroc1.set(   i_rstbRead );
  reg_easiroc1.set(   i_rstbSr   );
  reg_easiroc1.reset( i_loadSc   );
  reg_easiroc1.set(   i_selectSc );
  reg_easiroc1.set(   i_pwrOn    );
  reg_easiroc1.set(   i_resetPA  );
  reg_easiroc1.set(   i_valEvt   );
  reg_easiroc1.reset( i_razChn   );

  reg_easiroc2.set(   i_rstbRead );
  reg_easiroc2.set(   i_rstbSr   );
  reg_easiroc2.reset( i_loadSc   );
  reg_easiroc2.set(   i_selectSc );
  reg_easiroc2.set(   i_pwrOn    );
  reg_easiroc2.set(   i_resetPA  );
  reg_easiroc2.set(   i_valEvt   );
  reg_easiroc2.reset( i_razChn   );

  reg_module.reset( i_selectProbe );
  reg_module.reset( i_selectHg    );
  reg_module.reset( i_ledBusy     );
  reg_module.reset( i_ledReady    );
  reg_module.reset( i_ledUser     );
  reg_module.reset( i_userOutput  );
  reg_module.reset( i_startCycle2 );
  reg_module.reset( i_startCycle1 );

  sendDirectControl(ip);
}

//_________________________________________________________________________
void
resetPedestalSupp(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg = g_conf.copy_pedestal_suppression_null();
  int n_reg = reg.size();

  RBCP rbcp(ip);
  const uint8_t *reg_null = static_cast<const uint8_t*>(&reg[0]);
  rbcp.write(reg_null, addr_pede_supp, n_reg);
}

//_________________________________________________________________________
void
resetProbeRegister(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg_null = g_conf.copy_probereg_null();

  reg_easiroc1.reset( i_selectSc );
  reg_easiroc2.reset( i_selectSc );

  sendSlowControlSub(ip, reg_null, reg_null);
}

//_________________________________________________________________________
void
resetReadRegister(const std::string& ip)
{
  reg_easiroc1.reset( i_rstbRead );
  reg_easiroc2.reset( i_rstbRead );
  sendDirectControl(ip);

  reg_easiroc1.set( i_rstbRead );
  reg_easiroc2.set( i_rstbRead );
  sendDirectControl(ip);
}

//_________________________________________________________________________
void
sendDirectControl(const std::string& ip)
{
  const size_t n_reg = 3;
  uint8_t reg_direct[n_reg] = {
    static_cast<uint8_t>(reg_easiroc1.to_ulong()),
    static_cast<uint8_t>(reg_easiroc2.to_ulong()),
    static_cast<uint8_t>(reg_module.to_ulong())
  };

  RBCP rbcp(ip);
  rbcp.write(reg_direct, addr_direct_ctrl, n_reg);
}

//_________________________________________________________________________
void
sendPedestalSupp(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg = g_conf.copy_pedestal_suppression();
  int n_reg = reg.size();

  RBCP rbcp(ip);
  const uint8_t *reg_pede = static_cast<const uint8_t*>(&reg[0]);
  rbcp.write(reg_pede, addr_pede_supp, n_reg);
}

//_________________________________________________________________________
void
sendProbeRegister(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg_probe = g_conf.copy_probereg();
  veasiroc::regRbcpType reg_null  = g_conf.copy_probereg_null();

  int i_easiroc = g_conf.get_index_probe();

  reg_easiroc1.reset( i_selectSc );
  reg_easiroc2.reset( i_selectSc );

  if(i_easiroc == i_easiroc1){
    reg_module.reset( i_selectProbe );
    sendSlowControlSub(ip, reg_probe, reg_null);
  }else{
    reg_module.set( i_selectProbe );
    sendSlowControlSub(ip, reg_null,  reg_probe);
  }
}

//_________________________________________________________________________
void
sendReadRegister(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  int i_easiroc = g_conf.get_index_readsc();
  veasiroc::regRbcpType reg = g_conf.copy_readreg();
  int n_reg = reg.size();

  if(i_easiroc == i_easiroc1){
    reg_module.reset( i_selectHg );
  }else{
    reg_module.set( i_selectHg );
  }

  resetReadRegister(ip);

  RBCP rbcp(ip);
  const uint8_t *reg_read = static_cast<const uint8_t*>(&reg[0]);    
  if(i_easiroc == i_easiroc1){
    rbcp.write(reg_read, addr_read_reg1, n_reg);
  }else{
    rbcp.write(reg_read, addr_read_reg2, n_reg);
  }
}

//_________________________________________________________________________
void
sendSelectableLogic(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg = g_conf.copy_selectable_logic();
  int n_reg = reg.size();

  RBCP rbcp(ip);
  const uint8_t *reg_sel = static_cast<const uint8_t*>(&reg[0]);
  rbcp.write(reg_sel, addr_selectable, n_reg);
}

//_________________________________________________________________________
void
sendSlowControl(const std::string& ip)
{
  reg_easiroc1.set( i_selectSc );
  reg_easiroc2.set( i_selectSc );
    
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg_easiroc1 = g_conf.copy_screg(i_easiroc1);
  veasiroc::regRbcpType reg_easiroc2 = g_conf.copy_screg(i_easiroc2);
    
  sendSlowControlSub(ip, reg_easiroc1, reg_easiroc2);
}

//_________________________________________________________________________
void
sendSlowControlSub(const std::string& ip,
		   veasiroc::regRbcpType& reg1,
		   veasiroc::regRbcpType& reg2)
{
  reg_easiroc1.reset( i_loadSc );
  reg_easiroc2.reset( i_loadSc );
  reg_easiroc1.set( i_rstbSr );
  reg_easiroc2.set( i_rstbSr );
  reg_module.reset( i_startCycle1 );
  reg_module.reset( i_startCycle2 );
  sendDirectControl(ip);

  RBCP rbcp(ip);

  int n_reg = reg1.size();
  const uint8_t *reg_slow1 = static_cast<const uint8_t*>(&reg1[0]);
  rbcp.write(reg_slow1, addr_slow_ctrl1, n_reg);

  n_reg = reg2.size();
  const uint8_t *reg_slow2 = static_cast<const uint8_t*>(&reg2[0]);
  rbcp.write(reg_slow2, addr_slow_ctrl2, n_reg);    

  reg_module.set( i_startCycle1 );
  reg_module.set( i_startCycle2 );
  sendDirectControl(ip);

  sleep(1);

  reg_easiroc1.set( i_loadSc );
  reg_easiroc2.set( i_loadSc );
  reg_module.reset( i_startCycle1 );
  reg_module.reset( i_startCycle2 );
  sendDirectControl(ip);

  reg_easiroc1.reset( i_loadSc );
  reg_easiroc2.reset( i_loadSc );
  sendDirectControl(ip);
}

//_________________________________________________________________________
void
sendTimeWindow(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg = g_conf.copy_time_window();
  int n_reg = reg.size();

  RBCP rbcp(ip);
  const uint8_t *reg_time = static_cast<const uint8_t*>(&reg[0]);
  rbcp.write(reg_time, addr_time_window, n_reg);
}
