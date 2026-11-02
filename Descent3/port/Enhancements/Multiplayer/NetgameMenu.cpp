#include "controls.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"

static bool Menu_up = false;

static void FollowNetgameMenu(bool up) {
  if (up == Menu_up) {
    return;
  }
  Menu_up = up;
  if (up) {
    SuspendControls();
  } else {
    ResumeControls();
  }
}

static void RegisterNetgameMenu() {
  COND_VB_SHOULD(VB_NETGAME_MENU_UP, true, { FollowNetgameMenu(*should); });
}

static RegisterPortInitFunc initFunc(RegisterNetgameMenu);
