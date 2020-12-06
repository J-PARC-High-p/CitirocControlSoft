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

#include <iomanip>

#include"control_impl.hh"
#include"rbcp.hh"
#include"FPGAModule.hh"
#include"UDPRBCP.hh"
#include"RegisterMap.hh"

namespace{
  // Local index -----------------------------------------
  enum PinDirectControl
    {
      kRstbRead, kRstbSr, kLoadSc, kSelectSc, kPwrOn, kResetbPA, kValEvt, kRazChn, kRstbPSC, kPSModeb
    };

  enum CycleControl
    {
      kStartCycle, kSelectRead
    };
  
  std::bitset<16> reg_citiroc_pin;
  std::bitset<8> reg_module;
};

//_________________________________________________________________________
void
resetDirectControl(const std::string& ip)
{
  // reset direct control registers
  reg_citiroc_pin.set(   kRstbRead );
  reg_citiroc_pin.set(   kRstbSr   );
  reg_citiroc_pin.reset( kLoadSc   );
  reg_citiroc_pin.set(   kSelectSc );
  reg_citiroc_pin.set(   kPwrOn    );
  reg_citiroc_pin.set(   kResetbPA  );
  reg_citiroc_pin.set(   kValEvt   );
  reg_citiroc_pin.reset( kRazChn   );
  reg_citiroc_pin.set(   kRstbPSC );
  reg_citiroc_pin.set(   kPSModeb );

  reg_module.reset( kStartCycle );
  reg_module.reset( kSelectRead  );

  sendDirectControl(ip);
}

//_________________________________________________________________________
void
resetProbeRegister(const std::string& ip)
{
  femcitiroc::configLoader& g_conf = femcitiroc::configLoader::get_instance();
  femcitiroc::regRbcpType reg_null = g_conf.copy_probereg_null();

  reg_citiroc_pin.reset( kSelectSc );
  sendProbeRegisterSub(ip, reg_null);
}

//_________________________________________________________________________
void
resetReadRegister(const std::string& ip)
{
  reg_citiroc_pin.reset( kRstbRead );
  sendDirectControl(ip);

  reg_citiroc_pin.set( kRstbRead );
  sendDirectControl(ip);
}

//_________________________________________________________________________
void
resetSlowControl(const std::string& ip)
{
  reg_citiroc_pin.set( kSelectSc );
  sendDirectControl(ip);

  // reg_citiroc_pin.reset( kRstbSr );
  // reg_citiroc_pin.reset( kResetbPA );
  // reg_citiroc_pin.reset( kRstbPSC );
  // sendDirectControl(ip);
 
  // reg_citiroc_pin.set( kRstbSr );
  // reg_citiroc_pin.set( kResetbPA );
  // reg_citiroc_pin.set( kRstbPSC );
  // sendDirectControl(ip);
}

//_________________________________________________________________________
void
sendDirectControl(const std::string& ip)
{
  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  int n_reg_pin = (reg_citiroc_pin.size() + 7)/8;

  fpga_module.WriteModule(HUL::CITIROC::ASIC::mid,
  			  HUL::CITIROC::ASIC::kAddrPinDirectControl,
  			  reg_citiroc_pin.to_ulong(), n_reg_pin);

  fpga_module.WriteModule(HUL::CITIROC::ASIC::mid,
  			  HUL::CITIROC::ASIC::kAddrCycleControl,
  			  reg_module.to_ulong());
}

//_________________________________________________________________________
void
sendProbeRegister(const std::string& ip)
{
  femcitiroc::configLoader& g_conf = femcitiroc::configLoader::get_instance();
  femcitiroc::regRbcpType reg_probe = g_conf.copy_probereg();

  reg_citiroc_pin.reset( kSelectSc );
  //  resetProbeRegister(ip);

  sendProbeRegisterSub(ip, reg_probe);

  reg_citiroc_pin.set( kSelectSc );
}

//_________________________________________________________________________
void
sendProbeRegisterSub(const std::string& ip,
		   femcitiroc::regRbcpType& reg)
{
  reg_citiroc_pin.reset( kSelectSc );
  reg_citiroc_pin.set( kRstbSr );
  reg_module.reset( kStartCycle );
  sendDirectControl(ip);

  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  int n_reg = reg.size();
  const uint8_t *reg_read = static_cast<const uint8_t*>(&reg[0]);
  fpga_module.WriteModule_nByte(HUL::CITIROC::ASIC::mid,
				HUL::CITIROC::ASIC::kAddrSlowControlFIFO,
				reg_read, n_reg);

  reg_module.set( kStartCycle );
  sendDirectControl(ip);

  sleep(1);

  reg_module.reset( kStartCycle );
  sendDirectControl(ip);

}

//_________________________________________________________________________
void
sendReadRegister(const std::string& ip)
{
  femcitiroc::configLoader& g_conf = femcitiroc::configLoader::get_instance();
  femcitiroc::regRbcpType reg = g_conf.copy_readreg();

  reg_module.set( kSelectRead );
  // resetReadRegister(ip);

  sendReadRegisterSub(ip, reg);

  reg_module.reset( kSelectRead );
}

//_________________________________________________________________________
void
sendReadRegisterSub(const std::string& ip,
		   femcitiroc::regRbcpType& reg)
{
  reg_citiroc_pin.set( kRstbSr );
  reg_module.reset( kStartCycle );
  sendDirectControl(ip);

  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  int n_reg = reg.size();
  const uint8_t *reg_read = static_cast<const uint8_t*>(&reg[0]);
  fpga_module.WriteModule_nByte(HUL::CITIROC::ASIC::mid,
				HUL::CITIROC::ASIC::kAddrSlowControlFIFO,
				reg_read, n_reg);

  reg_module.set( kStartCycle );
  sendDirectControl(ip);

  sleep(1);

  reg_module.reset( kStartCycle );
  sendDirectControl(ip);
}

//_________________________________________________________________________
void
sendSlowControl(const std::string& ip)
{    
  femcitiroc::configLoader& g_conf = femcitiroc::configLoader::get_instance();
  femcitiroc::regRbcpType reg_citiroc = g_conf.copy_screg();

  reg_citiroc_pin.set( kSelectSc );
  // resetSlowControl(ip);

  sendSlowControlSub(ip, reg_citiroc);
}

//_________________________________________________________________________
void
sendSlowControlSub(const std::string& ip,
		   femcitiroc::regRbcpType& reg)
{
  reg_citiroc_pin.reset( kLoadSc );
  reg_citiroc_pin.set( kRstbSr );
  reg_module.reset( kStartCycle );
  sendDirectControl(ip);

  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  int n_reg = reg.size();
  const uint8_t *reg_slow = static_cast<const uint8_t*>(&reg[0]);
  fpga_module.WriteModule_nByte(HUL::CITIROC::ASIC::mid,
				HUL::CITIROC::ASIC::kAddrSlowControlFIFO,
				reg_slow, n_reg);

  reg_module.set( kStartCycle );
  sendDirectControl(ip);

  sleep(1);

  reg_citiroc_pin.set( kLoadSc );
  reg_module.reset( kStartCycle );
  sendDirectControl(ip);

  reg_citiroc_pin.reset( kLoadSc );
  sendDirectControl(ip);
}

