#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/UI/Gui.h"

// menu.cpp
#define IDV_OPTIONS 12
#define IDV_PILOT 13

static void RegisterMainMenu() {
  COND_VB_SHOULD(VB_OPEN_MAIN_MENU_ITEM, true, {
    const int item = va_arg(args, int);
    if (item == IDV_OPTIONS || item == IDV_PILOT) {
      Gui_OpenMenu(item == IDV_OPTIONS ? "Settings" : "Pilot");
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterMainMenu);
