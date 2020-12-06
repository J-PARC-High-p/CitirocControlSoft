#ifndef FEMCITI_CONF_REG_H_
#define FEMCITI_CONF_REG_H_

#include<unistd.h>
#include<vector>
#include<map>
#include<string>
#include<algorithm>

namespace femcitiroc
{

  typedef std::vector<uint32_t> regType;
  typedef std::vector<uint8_t>  regRbcpType;

  enum BitOrder{
    lsb2msb = 0,
    msb2lsb = 1 
  };
  
  struct Register
  {
    int32_t nbits;
    int32_t bit_order;
    bool    active_low;
    regType reg;
  };

  class configLoader
  {
  public:
    configLoader();

    regRbcpType copy_probereg();
    regRbcpType copy_probereg_null();
    regRbcpType copy_readreg();
    regRbcpType copy_screg();

    int         get_index_probe();
    int         get_index_readsc();
    static configLoader& get_instance();

    void        print( const regRbcpType& cont, const std::string& arg = "");
    int         read_YAML( const std::string& filename);

  private:
    // Registers (YAML base)
#if CXX14
    std::map<std::string, uint32_t, std::less<>>   m_reg_alias;
    std::map<std::string, Register, std::less<>>   m_screg_map;
#else
    std::map<std::string, uint32_t>   m_reg_alias;
    std::map<std::string, Register>   m_screg_map;
#endif
    std::vector<std::string>                       m_screg_order;

#if CXX14
    std::map<std::string, Register, std::less<>>   m_otherreg_map;
#else
    std::map<std::string, Register>   m_otherreg_map;
#endif

    // Registers (RBCP base)
    std::vector<bool>                              m_bit_rbcp;
    regRbcpType                                    m_reg_rbcp;

    // Probe register type
    enum probeType {
      is_input_dac,
      is_out_pa_hg,
      is_out_pa_lg,
      is_peak_sensing_modeb_hg,
      is_out_ssh_hg,
      is_peak_sensing_modeb_lg,
      is_out_ssh_lg,
      is_out_fs,
      is_out_none
    };

    configLoader( const configLoader& );
    configLoader& operator = ( const configLoader& );

    void fill_bit(Register& cont);

    void initialize_alias();
    void initialize_other_register();
    void initialize_slowcontrol_register();

    void translate_bit2reg();
  };

  inline configLoader&
  configLoader::get_instance()
  {
    static configLoader inst;
    return inst;
  }

};



#endif
