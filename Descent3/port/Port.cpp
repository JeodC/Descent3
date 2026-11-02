#include "Port.h"

#include "GameInteractor/GameInteractor.h"
#include "PortInit.hpp"
#include "Settings/Settings.h"

void Port_Startup() {
  Settings::Load();
  GameInteractor::Instance = new GameInteractor();
  PortInit::InitAll();
}
