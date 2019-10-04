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
     kRstbRead, kRstbSr, kLoadSc, kSelectSc, kPwrOn, kResetPA, kValEvt, kRazChn
    };

  enum CycleControl
    {
     kSelectProbe, kSelectRead, kLedBusy, kLedReady, kLedUser, kUserOutput, kStartCycle1, kStartCycle2
    };
  
  std::bitset<8> reg_easiroc_pin;
  std::bitset<8> reg_module;
};

//_________________________________________________________________________
void
resetDirectControl(const std::string& ip)
{
  // reset direct control registers
  reg_easiroc_pin.set(   kRstbRead );
  reg_easiroc_pin.set(   kRstbSr   );
  reg_easiroc_pin.reset( kLoadSc   );
  reg_easiroc_pin.set(   kSelectSc );
  reg_easiroc_pin.set(   kPwrOn    );
  reg_easiroc_pin.set(   kResetPA  );
  reg_easiroc_pin.set(   kValEvt   );
  reg_easiroc_pin.reset( kRazChn   );

  reg_module.reset( kSelectProbe );
  reg_module.reset( kSelectRead  );
  reg_module.reset( kLedBusy     );
  reg_module.reset( kLedReady    );
  reg_module.reset( kLedUser     );
  reg_module.reset( kUserOutput  );
  reg_module.reset( kStartCycle2 );
  reg_module.reset( kStartCycle1 );

  sendDirectControl(ip);
}

//_________________________________________________________________________
void
resetProbeRegister(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg_null = g_conf.copy_probereg_null();

  reg_easiroc_pin.reset( kSelectSc );
  sendSlowControlSub(ip, reg_null);
}

//_________________________________________________________________________
void
resetReadRegister(const std::string& ip)
{
  reg_easiroc_pin.reset( kRstbRead );
  sendDirectControl(ip);

  reg_easiroc_pin.set( kRstbRead );
  sendDirectControl(ip);
}

//_________________________________________________________________________
void
sendDirectControl(const std::string& ip)
{
  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  fpga_module.WriteModule(HUL::CITIROC::ASIC::mid,
			  HUL::CITIROC::ASIC::kAddrPinDirectControl,
			  reg_easiroc_pin.to_ulong());

  fpga_module.WriteModule(HUL::CITIROC::ASIC::mid,
			  HUL::CITIROC::ASIC::kAddrCycleControl,
			  reg_module.to_ulong());
}

//_________________________________________________________________________
void
sendProbeRegister(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg_probe = g_conf.copy_probereg();

  reg_easiroc_pin.reset( kSelectSc );
  sendSlowControlSub(ip, reg_probe);
}

//_________________________________________________________________________
void
sendReadRegister(const std::string& ip)
{
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg = g_conf.copy_readreg();

  reg_module.set( kSelectRead );
  resetReadRegister(ip);

  sendReadRegisterSub(ip, reg);

  reg_module.reset( kSelectRead );
}

//_________________________________________________________________________
void
sendReadRegisterSub(const std::string& ip,
		   veasiroc::regRbcpType& reg)
{
  reg_easiroc_pin.reset( kLoadSc );
  reg_easiroc_pin.set( kRstbSr );
  reg_module.reset( kStartCycle1 );
  reg_module.reset( kStartCycle2 );
  sendDirectControl(ip);

  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  int n_reg = reg.size();
  const uint8_t *reg_read = static_cast<const uint8_t*>(&reg[0]);
  fpga_module.WriteModule_nByte(HUL::CITIROC::ASIC::mid,
				HUL::CITIROC::ASIC::kAddrSlowControlFIFO,
				reg_read, n_reg);

  reg_module.set( kStartCycle1 );
  reg_module.set( kStartCycle2 );
  sendDirectControl(ip);

  sleep(1);

  reg_easiroc_pin.set( kLoadSc );
  reg_module.reset( kStartCycle1 );
  reg_module.reset( kStartCycle2 );
  sendDirectControl(ip);

  reg_easiroc_pin.reset( kLoadSc );
  sendDirectControl(ip);
}

//_________________________________________________________________________
void
sendSlowControl(const std::string& ip)
{
  reg_easiroc_pin.set( kSelectSc );
    
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  veasiroc::regRbcpType reg_easiroc = g_conf.copy_screg();

  sendSlowControlSub(ip, reg_easiroc);
}

//_________________________________________________________________________
void
sendSlowControlSub(const std::string& ip,
		   veasiroc::regRbcpType& reg)
{
  reg_easiroc_pin.reset( kLoadSc );
  reg_easiroc_pin.set( kRstbSr );
  reg_module.reset( kStartCycle1 );
  reg_module.reset( kStartCycle2 );
  sendDirectControl(ip);

  RBCP::UDPRBCP rbcp(ip, RBCP::gUdpPort, RBCP::UDPRBCP::kNoDisp);
  HUL::FPGAModule fpga_module(rbcp);

  int n_reg = reg.size();
  const uint8_t *reg_slow = static_cast<const uint8_t*>(&reg[0]);
  fpga_module.WriteModule_nByte(HUL::CITIROC::ASIC::mid,
				HUL::CITIROC::ASIC::kAddrSlowControlFIFO,
				reg_slow, n_reg);

  std::cout << "bit number : " << n_reg*8 << std::endl;
  std::cout << "Slow control signal : " << std::endl;

  std::ios::fmtflags flagsSaved = std::cout.flags();    
  std::cout.setf(std::ios::hex, std::ios::basefield);
  std::cout.setf(std::ios::right, std::ios::adjustfield);
  char fillSaved = std::cout.fill('0');

  for(int i = 0; i < (n_reg/4); i++){
    std::cout << std::setw(2) << static_cast<int>(reg[4*i + 3]);
    std::cout << std::setw(2) << static_cast<int>(reg[4*i + 2]);
    std::cout << std::setw(2) << static_cast<int>(reg[4*i + 1]);
    std::cout << std::setw(2) << static_cast<int>(reg[4*i]);
    std::cout << "\n";
  }

  std::cout << std::setw(2) << 0;
  std::cout << std::setw(2) << static_cast<int>(reg[n_reg]);
  std::cout << std::setw(2) << static_cast<int>(reg[n_reg - 1]);
  std::cout << std::setw(2) << static_cast<int>(reg[n_reg - 2]);
    
  std::cout << std::endl;

  std::cout.flags(flagsSaved);
  std::cout.fill(fillSaved);

  reg_module.set( kStartCycle1 );
  reg_module.set( kStartCycle2 );
  sendDirectControl(ip);

  sleep(1);

  reg_easiroc_pin.set( kLoadSc );
  reg_module.reset( kStartCycle1 );
  reg_module.reset( kStartCycle2 );
  sendDirectControl(ip);

  reg_easiroc_pin.reset( kLoadSc );
  sendDirectControl(ip);
}

