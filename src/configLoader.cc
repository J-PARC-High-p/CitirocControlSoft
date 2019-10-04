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

  // RS or discri
  m_reg_alias["RS"]       = 1;
  m_reg_alias["trigger"]  = 0;
  
  // T&D bias, LG PA bias
  m_reg_alias["weakbias"] = 1;
  m_reg_alias["highbias"] = 0;

  // SCA or PeakD
  m_reg_alias["SCA"]   = 1;
  m_reg_alias["PeakD"]  = 0;

  // Input DAC reference
  m_reg_alias["4.5V"] = 1;
  m_reg_alias["2.5V"] = 0;

  // Slow shaper
  m_reg_alias["12.5ns"] =   1;
  m_reg_alias["25.0ns"] =   2;
  m_reg_alias["37.5ns"] =   3;
  m_reg_alias["50.0ns"] =   4;
  m_reg_alias["62.5ns"] =   5;
  m_reg_alias["75.0ns"] =   6;
  m_reg_alias["87.5ns"] =   7;

  // Fast shaper
  m_reg_alias["LG"] = 1;
  m_reg_alias["HG"] = 0;

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
    const std::string name = "Channel 0 to 31 4-bit_t";
    Register cont = {4, lsb2msb, false, 
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
    const std::string name = "Channel 0 to 31 4-bit";
    Register cont = {4, lsb2msb, false, 
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
    const std::string name = "EN_discri";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Discriminator";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "EN_discri_t";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Discriminator_t";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "4b_dac";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: 4b_dac";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "4b_dac_t";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: 4b_dac_t";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Discriminator Mask";
    Register cont = {1, lsb2msb, true, 
		     {
		       1, 1, 1, 1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1, 1, 1, 1,
		       1, 1, 1, 1, 1, 1, 1, 1
		     }};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: HG T&H(Widlar SCA)";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_HG_T&H(Widlar SCA)";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: LG T&H(Widlar SCA)";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_LG_T&H(Widlar SCA)";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "SCA bias";
    Register cont = {1, lsb2msb, false, {m_reg_alias["weakbias"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: HG Pdet";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_HG_Pdet";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: LG Pdet";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_LG_Pdet";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Sel SCA or PeakD HG";
    Register cont = {1, lsb2msb, false, {m_reg_alias["SCA"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Sel SCA or PeakD LG";
    Register cont = {1, lsb2msb, false, {m_reg_alias["SCA"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "bypass PSC";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Sel Trig Ext PSC";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Fast Shapers Follower";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "PP: Fast Shaper";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Low Gain Slow Shaper";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    Register cont = {3, lsb2msb, true,  {m_reg_alias["87.5ns"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: High Gain Slow Shaper";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    Register cont = {3, lsb2msb, true,  {m_reg_alias["87.5ns"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "LG PA bias";
    Register cont = {1, lsb2msb, false, {m_reg_alias["weakbias"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: High Gain PreAmplifier";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "PP: Low Gain PreAmplifier";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "Fast Shaper on LG";
    Register cont = {1, lsb2msb, false, {m_reg_alias["HG"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_input_dac";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "8-bit DAC reference";
    Register cont = {1, lsb2msb, false, {m_reg_alias["4.5V"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Input 8-bit DAC";
    Register cont = {9, lsb2msb, false, 
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
    const std::string name = "Channel 0 to 31 PA";
    Register cont = {15, msb2lsb, false, 
		     {
		       30680, 30680, 30680, 30680, 30680, 30680, 30680, 30680,
		       30680, 30680, 30680, 30680, 30680, 30680, 30680, 30680,
		       30680, 30680, 30680, 30680, 30680, 30680, 30680, 30680,
		       30680, 30680, 30680, 30680, 30680, 30680, 30680, 30680
		     }};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Temp";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Temp";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: BandGap";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "EN_DAC1";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: DAC1";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_DAC2";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: DAC2";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "DAC1 code";
    Register cont = {10, msb2lsb, false, {256}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "DAC2 code";
    Register cont = {10, msb2lsb, false, {256}};
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
    const std::string name = "PP: High Gain OTAq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "PP: Low Gain OTAq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
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
    const std::string name = "PP: Probe OTAq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Testb_Otaq";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Val_Evt receiver";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Val_Evt receiver";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_Raz_Chn receiver";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "PP: Raz Chn receiver";
    Register cont = {1, lsb2msb, false, {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_out_deg";
    Register cont = {1, lsb2msb, false,  {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_OR32";
    Register cont = {1, lsb2msb, false,  {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_NOR32_oc";
    Register cont = {1, lsb2msb, false,  {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "Trigger Polarity";
    Register cont = {1, lsb2msb, false,  {m_reg_alias["Disable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_NOR32T_oc";
    Register cont = {1, lsb2msb, false,  {m_reg_alias["Enable"]}};
    m_screg_map.insert(std::make_pair(name, cont));
    m_screg_order.push_back(name);
  }

  {
    const std::string name = "EN_32 triggers";
    Register cont = {1, lsb2msb, false,  {m_reg_alias["Enable"]}};
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
