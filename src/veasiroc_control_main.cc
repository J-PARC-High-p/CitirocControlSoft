#include<iostream>
#include<cstdlib>
#include<vector>
#include<bitset>

#include"configLoader.hh"
#include"RegisterMap.hh"
#include"control_impl.hh"

int parse_option(std::vector<std::string>& arg);

// __________________________________________________________________________
namespace{
  const std::string opt_ref[] =
    {
      "-ip=", "-yaml=",
      "-sc", "-read", "-probe", "-all",
      "-probe-off",
      "-q",
      "bad-option"
    };

  std::string ip;
  std::vector<std::string> yaml_list;

  enum Command
    {
      i_sc, i_read, i_probe, i_probe_off,
      sizeCommand
    };

  std::bitset<sizeCommand> exec_list;

  bool quiet = false;
}

// __________________________________________________________________________
int main(int argc, char* argv[])
{
  if(argc == 1){
    std::cout << "Usage\n";
    std::cout << "  veasiroc_control [options...]\n\n";
    std::cout << "Options:\n";
    std::cout << " -ip=xxx.yyy.zzz.aaa (Must)\n";
    std::cout << " -yaml=file-path\n";
    std::cout << "   If -yaml=auto is designated, this program refers\n";
    std::cout << "   the register files in ${HOME}/vme-easiroc-registers,\n";
    std::cout << "   which are labeled by the ip address.\n";
    std::cout << "   i.e., RegisterValue_aaa.yml, InputDAC_aaa.yml, PedeSup_aaa.yml\n";
    std::cout << " -sc\n";
    std::cout << "   Send slow control registers\n";
    std::cout << " -read\n";
    std::cout << "   Send read slow control registers\n";
    std::cout << " -probe\n";
    std::cout << "   Send probe registers\n";
    std::cout << " -all\n";
    std::cout << "   Equal to -sc -read -probe\n";
    std::cout << " -probe-off\n";
    std::cout << "   Reset probe registers\n";
    std::cout << " -q\n";
    std::cout << "   Quiet. Hide all the message.\n";
    std::cout << std::endl;
    return -1;
  }
  
  std::vector<std::string> arg_list;
  for(int i = 1; i<argc; ++i) arg_list.push_back(std::string(argv[i]));
  if(0 != parse_option(arg_list)) return -1;

  if(0 == yaml_list.size()){
    // use default registers
    yaml_list.push_back("default_register/RegisterValue.yml");
    yaml_list.push_back("default_register/InputDAC.yml");
  }else if(yaml_list[0] == "auto"){
    std::string home_dir   = getenv("HOME");
    std::string module_num = ip.substr(ip.find_last_of(".")+1,
				       ip.size()-1);
    
    yaml_list.clear();
    yaml_list.push_back(home_dir + "/vme-easiroc-registers/RegisterValue/RegisterValue_" + module_num + ".yml");
    yaml_list.push_back(home_dir + "/vme-easiroc-registers/InputDAC/InputDAC_" + module_num + ".yml");
  }
  
  // parse result
  if(!quiet){
    std::cout << "IP:\t " << ip << std::endl;
    for(unsigned int i = 0; i<yaml_list.size(); ++i){
      std::cout << "YAML:\t " << yaml_list[i] << std::endl;
    }
    std::cout << "Execute functions: " << std::endl;
    if(exec_list[i_sc])    std::cout << " - Slow Control" << std::endl;
    if(exec_list[i_read])  std::cout << " - Read register" << std::endl;
    if(exec_list[i_probe]) std::cout << " - Probe register" << std::endl;
    if(exec_list[i_probe_off]) std::cout << " - Reset probe register" << std::endl;
  }
  
  veasiroc::configLoader& g_conf = veasiroc::configLoader::get_instance();
  for(unsigned int i = 0; i<yaml_list.size(); ++i) g_conf.read_YAML(yaml_list[i]);

  resetDirectControl(ip);
  if(exec_list[i_sc])    sendSlowControl(ip);
  if(exec_list[i_probe]) sendProbeRegister(ip);
  if(exec_list[i_read])  sendReadRegister(ip);
  if(exec_list[i_probe_off]) resetProbeRegister(ip);
  
  return 0;
}

// __________________________________________________________________________
int parse_option(std::vector<std::string>& arg)
{
  exec_list.reset();
  size_t n_opt = sizeof(opt_ref)/sizeof(std::string);

  for(unsigned int i = 0; i<arg.size(); ++i){
    for(unsigned int j = 0; j<n_opt; ++j){
      if(true 
	 && std::string::npos != arg[i].find('=')
	 && std::string::npos != arg[i].find(opt_ref[j])
	 ){
	std::string ref    = arg[i].substr(0, opt_ref[j].size());
	std::string option = arg[i].substr(ref.size(), arg[i].size()-1);

	if( ref == "-ip=")   ip = option;
	if( ref == "-yaml=") yaml_list.push_back(option);

	break;
      }else if(arg[i] == opt_ref[j]){
	if( arg[i] == "-sc")    exec_list.set( i_sc    );
	if( arg[i] == "-read")  exec_list.set( i_read  );
	if( arg[i] == "-probe") exec_list.set( i_probe );
	if( arg[i] == "-all")   exec_list.set();
	if( arg[i] == "-probe-off") exec_list.set( i_probe_off );

	if( arg[i] == "-q")     quiet = true;

	break;
      }// find
      
      if( opt_ref[j] == "bad-option"){
	std::cerr << "#E: No such option (" << arg[i] << ")" << std::endl;
	return -2;
      }
    }// for(j)
  }// for(i)

  if(ip.empty()){
    std::cerr << "#E: IP address must be designated" << std::endl;
    return -1;
  } 

  return 0;
}
