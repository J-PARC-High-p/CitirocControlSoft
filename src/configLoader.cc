#include"configLoader.hh"

#include<iostream>
#include<fstream>
#include<sstream>
#include<cstdio>
#include<algorithm>

static const std::string class_name = "configLoader";

using namespace veasiroc;

// -----------------------------------------------------------------------
configLoader::configLoader()
{
  initialize_alias();
  initialize_slowcontrol_register();
  initialize_other_register();
}

// -----------------------------------------------------------------------
regRbcpType
configLoader::copy_probereg()
{
  static const std::string func_name = "["+class_name+"::"+__func__+"()]";

  m_bit_rbcp.clear();
  m_reg_rbcp.clear();
  auto itr_type    = m_otherreg_map.find("Probe");
  auto itr_channel = m_otherreg_map.find("Probe Channel");
  probeType type     =  static_cast<probeType>((itr_type->second).reg[0]);
  uint32_t  position =  (itr_channel->second).reg[0];
  
  switch(type){
  case is_out_pa_hg:
    position = position*2;
    break;
  case is_out_pa_lg:
    position = position*2 +1;
    break;
  case is_out_ssh_hg:
    position = 64 + position*2;
    break;
  case is_out_ssh_lg:
    position = 64 + position*2 +1;
    break;
  case is_out_fs:
    position = 128 + position;
    break;
  default:
    break;
  }

  m_bit_rbcp.resize(160);
  for(auto itr = m_bit_rbcp.begin(); itr != m_bit_rbcp.end(); ++itr){
    *itr = false;
  }
  m_bit_rbcp[position] = true;

  translate_bit2reg();
  reverse(m_reg_rbcp.begin(), m_reg_rbcp.end());
#if DEBUG
  print(m_reg_rbcp, "Probe Slow Control");
#endif
  return m_reg_rbcp;
}

// -----------------------------------------------------------------------
regRbcpType
configLoader::copy_probereg_null()
{
  copy_probereg();
  //  for(auto& val : m_reg_rbcp) val = 0;
  for(uint32_t i = 0; i<m_reg_rbcp.size(); ++i) m_reg_rbcp[i] = 0;

#if DEBUG
  print(m_reg_rbcp, "Probe Slow Control (NULL)");
#endif

  return m_reg_rbcp;
}

// -----------------------------------------------------------------------
regRbcpType
configLoader::copy_readreg()
{
  static const std::string func_name = "["+class_name+"::"+__func__+"()]";

  m_bit_rbcp.clear();
  m_reg_rbcp.clear();
  auto itr = m_otherreg_map.find("High Gain Channel");
  uint32_t channel = (itr->second).reg[0];

  Register cont = itr->second;
  cont.reg[0] = channel;
  fill_bit(cont);

  translate_bit2reg();
#if DEBUG
  print(m_reg_rbcp, "Read Slow Control");
#endif
  return m_reg_rbcp;
}

// -----------------------------------------------------------------------
regRbcpType
configLoader::copy_screg()
{
  static const std::string func_name = "["+class_name+"::"+__func__+"()]";

  m_bit_rbcp.clear();
  m_reg_rbcp.clear();
  auto itr_end = m_screg_map.end();
  //  for( const auto& reg_name : m_screg_order){
  for( uint32_t i = 0; i<m_screg_order.size(); ++i){
    auto& reg_name = m_screg_order[i];
    auto itr = m_screg_map.find(reg_name);
    if(itr != itr_end){
      fill_bit(itr->second);
    }else{
      // Not found
      std::cerr << "#E: "
		<< func_name 
		<< " No such register key [" << reg_name << "]"
		<< std::endl;
    }
  }// for(screg_order)

  translate_bit2reg();
  reverse(m_reg_rbcp.begin(), m_reg_rbcp.end());

#if DEBUG
  std::cout << func_name << std::endl;
  print(m_reg_rbcp, "EASIROC");
#endif
  return m_reg_rbcp;
}

// -----------------------------------------------------------------------
void
configLoader::fill_bit(Register& cont)
{
  //  for(const auto& val : cont.reg){
  for(uint32_t i = 0; i<cont.reg.size(); ++i){
    auto& val = cont.reg[i];
    for(int i = 0; i<cont.nbits; ++i){
      if(cont.bit_order == lsb2msb){
	int bit = cont.active_low ^ ((val >> i) & 1 ? true : false);
	m_bit_rbcp.push_back(bit == 0 ? false : true);
      }else{
	int bit = cont.active_low ^ ((val >> (cont.nbits -1 - i)) & 1 ? true : false);
	m_bit_rbcp.push_back(bit == 0 ? false : true);
      }// bit_order
    }// for(i)
  }// for(reg)
}

// -----------------------------------------------------------------------
void
configLoader::initialize_alias()
{
  // Power register
  m_reg_alias["Enable"]   = 1;
  m_reg_alias["Disable"]  = 0;

  // DAC slop
  m_reg_alias["fine"]     = 1;
  m_reg_alias["coarse"]   = 0;

  // RS or discri
  m_reg_alias["RS"]       = 1;
  m_reg_alias["trigger"]  = 0;
  
  // T&D bias, LG PA bias
  m_reg_alias["weakbias"] = 1;
  m_reg_alias["highbias"] = 0;

  // Input DAC reference
  m_reg_alias["external"] = 1;
  m_reg_alias["internal"] = 0;

  // Slow shaper
  m_reg_alias["25ns"] =     1;
  m_reg_alias["50ns"] =     2;
  m_reg_alias["75ns"] =     3;
  m_reg_alias["100ns"] =    4;
  m_reg_alias["125ns"] =    5;
  m_reg_alias["150ns"] =    6;
  m_reg_alias["175ns"] =    7;

  m_reg_alias["NoC"] =      0;
  m_reg_alias["100fF"] =    8;
  m_reg_alias["200fF"] =    4;
  m_reg_alias["300fF"] =    12;
  m_reg_alias["400fF"] =    2;
  m_reg_alias["500fF"] =    10;
  m_reg_alias["600fF"] =    6;
  m_reg_alias["700fF"] =    14;
  m_reg_alias["800fF"] =    1;
  m_reg_alias["900fF"] =    9;
  m_reg_alias["1.0pF"] =    5;
  m_reg_alias["1.1pF"] =    13;
  m_reg_alias["1.2pF"] =    3;
  m_reg_alias["1.3pF"] =    11;
  m_reg_alias["1.4pF"] =    7;
  m_reg_alias["1.5pF"] =    15;

  // Selectable logic
  m_reg_alias["OR32U"] =    64;
  m_reg_alias["OR32D"] =    65;
  m_reg_alias["OR64"]  =    66;

  // Probe type
  m_reg_alias["Out_PA_HG"]  = is_out_pa_hg;
  m_reg_alias["Out_PA_LG"]  = is_out_pa_lg;
  m_reg_alias["Out_ssh_HG"] = is_out_ssh_hg;
  m_reg_alias["Out_ssh_LG"] = is_out_ssh_lg;
  m_reg_alias["Out_fs"]     = is_out_fs;
}

// -----------------------------------------------------------------------
void
configLoader::initialize_other_register()
{
  // Read slow control
  {
    const std::string name = "High Gain Channel";
    Register cont = {8, msb2lsb, false, {0}};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }

  // Probe
  {
    // Internal
    const std::string name = "Probe Channel";
    Register cont = {1, lsb2msb, false, {0}};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }

  {
    // Internal
    const std::string name = "Probe";
    Register cont = {1, lsb2msb, false, {is_out_pa_hg}};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }

  // Module registers
  {
    const std::string name = "SelectableLogic";
    Register cont = {8, msb2lsb, false, {0}};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }

  {
    const std::string name = "HG";
    Register cont = {16, msb2lsb, false, 
		     {
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0
		     }};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }

  {
    const std::string name = "LG";
    Register cont = {16, msb2lsb, false, 
		     {
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0
		     }};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }

  {
    const std::string name = "TimeWindow";
    Register cont = {16, msb2lsb, false, {0}};
    m_otherreg_map.insert(std::make_pair(name, cont));
  }
}

// -----------------------------------------------------------------------
void
configLoader::initialize_slowcontrol_register()
{
  // Register cont = {nbits, bit_order, active_low, {reg(vector)}};

  {
    const std::string name = "EN_input_dac";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "8-bit DAC reference";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Input 8-bit DAC";
    Register cont = {9, lsb2msb, false, 
		     {
		       256, 256, 256, 256, 256, 256, 256, 256,
		       256, 256, 256, 256, 256, 256, 256, 256,
		       256, 256, 256, 256, 256, 256, 256, 256,
		       256, 256, 256, 256, 256, 256, 256, 256
		     }};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Low Gain PA bias";
    Register cont = {1, lsb2msb, false, {m_reg_alias["highbias"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "High Gain PreAmplifier PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_High_Gain_PA";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Low Gain PreAmplifier PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Low_Gain_PA";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Capacitor HG PA Comp";
    Register cont = {4, lsb2msb, false, {14}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Capacitor HG PA Fdbck";
    Register cont = {4, lsb2msb, false, {m_reg_alias["200fF"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Capacitor LG PA Fdbck";
    Register cont = {4, msb2lsb, false, {m_reg_alias["200fF"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Capacitor LG PA Comp";
    Register cont = {4, msb2lsb, false, {9}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "DisablePA & In_calib_EN";
    Register cont = {2, msb2lsb, false, 
		     {
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0
		     }};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Low Gain Slow Shaper PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "En_Low_Gain_Slow Shaper";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Time Constant LG Shaper";
    Register cont = {3, lsb2msb, true,  {m_reg_alias["50ns"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "High Gain Slow Shaper PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "En_High_Gain_Slow Shaper";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Time Constant HG Shaper";
    Register cont = {3, lsb2msb, true,  {m_reg_alias["50ns"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Fast Shapers Follower PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Fast Shaper";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Fast Shaper PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "T&H(Widlar SCA) PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_T&H(Widlar SCA)";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "T&H bias(Widlar)";
    Register cont = {1, lsb2msb, false, {m_reg_alias["highbias"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_discri";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Discriminator PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "RS_or_discri";
    Register cont = {1, lsb2msb, false, {m_reg_alias["trigger"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Discriminator Mask";
    Register cont = {1, lsb2msb, true, 
		     {
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0,
		       0, 0, 0, 0, 0, 0, 0, 0
		     }};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "DAC code";
    Register cont = {10, lsb2msb, false, {840}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "DAC slope";
    Register cont = {1, lsb2msb, false, {m_reg_alias["fine"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "DAC PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_DAC";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "BandGap PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_BandGap";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "High Gain OTAq PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Low Gain OTAq PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Probe OTAq PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "LVDS receivers PP";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_High Gain OTAq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Low Gain OTAq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Probe OTAq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_LVDS receivers";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_out_dig";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_OR32";
    Register cont = {1, lsb2msb, true,  {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_32_triggers";
    Register cont = {1, lsb2msb, true,  {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "NC";
    Register cont = {4, lsb2msb, false,  {0}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

}

// -----------------------------------------------------------------------
void
configLoader::print(const regRbcpType& cont, const std::string& arg)
{
  static const std::string func_name = "["+class_name+"::"+__func__+"()]";

  std::cout << "#D: "
	    << func_name << " "
	    << arg
	    << std::endl;
  
  //  for(const auto& val : cont){
  //    printf(" - %x\n", val);
  //  }// for(reg_rbcp)
  for(uint32_t i = 0; i<cont.size(); ++i){
    printf(" - %x\n", cont[i]);
  }// for(reg_rbcp)
}

// -----------------------------------------------------------------------
void
configLoader::translate_bit2reg()
{
  const size_t nbit_byte = 8;
  const size_t nloop     = m_bit_rbcp.size()/nbit_byte;
  
  for(uint32_t i = 0; i<nloop; ++i){
    uint8_t val = 0;
    for(uint32_t j = 0; j<nbit_byte; ++j){
      val += m_bit_rbcp[nbit_byte*i + j] ? 1 << (nbit_byte-1-j) : 0;
    }// for(j)
    m_reg_rbcp.push_back(val);
  }// for(i)
}

// -----------------------------------------------------------------------
int
configLoader::read_YAML( const std::string& filename)
{
  static const std::string func_name = "["+class_name+"::"+__func__+"()]";

  std::ifstream ifs(filename.c_str());
  if(!ifs.is_open()){
    std::cerr << "#E: " << func_name
	      << " No such YAML file (" << filename << ")"
	      << std::endl;
    return -1;
  }

  enum modeYAML{
    is_easiroc1, is_module
  } present_mode = is_module;

  enum typeYAML{
    is_associative, is_array
  } present_type = is_associative;

  std::string line;
  std::string present_key;
  std::string present_reg;
  int present_index = 0;
  while(getline(ifs, line)){
    // Comment or something
    if(line.empty() || line[0] == '#') continue;

    // YAML document separator
    if(line.find("---") != std::string::npos) continue;

    // YAML document force end
    if(line.find("...") != std::string::npos) break;

    // YAML body
    std::stringstream line_to_word(line);
    if(line[0] != ' ' && line[0] != '-'){
      getline(line_to_word, present_key, ':');
      if(present_key == "EASIROC"){
	present_mode = is_easiroc1;
      }else{
	present_mode = is_module;
      }

      if(present_mode != is_module) continue;
    }// mode change

    line_to_word.str("");
    line_to_word.clear(std::stringstream::goodbit);
    line_to_word << line;

    if(line.find("- ") == std::string::npos){
      present_type  = is_associative;
      present_index = 0;
      
      std::string              buf;
      std::vector<std::string> words;
      while(getline(line_to_word, buf, ':')) words.push_back(buf);
      //      for(auto& i : words) while(i[0] == ' ') i.erase(0, 1);
      for(uint32_t i = 0; i<words.size(); ++i) while(words[i][0] == ' ') words[i].erase(0, 1);
      
      present_key = words[0];
      if(words.size() == 1 || words[1][0] == '#') continue;
      
      std::stringstream word_to_reg(words[1]);
      word_to_reg >> present_reg;
    }else{
      present_type  = is_array;

      std::string buf;
      std::stringstream line_to_reg(line);
      line_to_reg >> buf;
      line_to_reg >> present_reg;
    }

    auto itr = present_mode == is_module ? 
      m_otherreg_map.find(present_key) : m_screg_map.find(present_key);
    
    if(present_mode == is_easiroc1){
#if DEBUG
      std::cout << "EASIROC1::" 
		<< present_key << "::" 
		<< present_reg << " (" << present_index << ")"
		<< std::endl;
#endif

      if(itr == m_screg_map.end()){
	std::cerr << "#E: " << func_name
		  << " No such key (" << present_key << ")"
		  << std::endl;	
	
	return -1;
      }
    }else if(present_mode == is_module){
#if DEBUG
      std::cout << present_key << "::" 
		<< present_reg << " (" << present_index << ")"
		<< std::endl;
#endif

      if(itr == m_otherreg_map.end()){
	std::cerr << "#E: " << func_name
		  << " No such key (" << present_key << ")"
		  << std::endl;	
	
	return -1;
      }
    }

    Register& cont = itr->second;
    if(m_reg_alias.find(present_reg) != m_reg_alias.end()){
      // Alias tranlation
      uint32_t val = m_reg_alias[present_reg];
      cont.reg[present_index] = val;
    }else{
      uint32_t val = 0;
      std::stringstream reg_to_val(present_reg);
      reg_to_val >> val;
      cont.reg[present_index] = val;
    }
      
    if(present_type == is_array) ++present_index;
      
  }// while(getline)

  return 0;
}
