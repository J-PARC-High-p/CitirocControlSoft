#ifndef CTRL_IMPL_H
#define CTRL_IMPL_H

#include "configLoader.hh"

void resetDirectControl(const std::string& ip);
void resetProbeRegister(const std::string& ip);
void resetReadRegister(const std::string& ip);

void sendDirectControl(const std::string& ip);
void sendProbeRegister(const std::string& ip);
void sendReadRegister(const std::string& ip);
void sendSlowControl(const std::string& ip);
void sendSlowControlSub(const std::string& ip, 
			veasiroc::regRbcpType& reg);

#endif
