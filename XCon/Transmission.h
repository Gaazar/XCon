#pragma once
#include <string>
#include "ControlPack.h"

void InitTransmission(unsigned short port);
void CleanTransmission();
void SendControl(ControlPack cp, std::string raddr, unsigned short port);
void SendFeedback(FeedbackPack cp, std::string raddr, unsigned short port);
void ControlRecv(void(*cb)(int, char*));
