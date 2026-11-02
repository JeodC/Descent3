#include "ddio.h"
#include "ddio_common.h"
#include "gamecinematics.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/UI/Gui.h"

static bool OpenPage(int key) {
  switch (key) {
  case KEY_F2:
    Gui_OpenMenu("Settings");
    return true;
  case KEY_ALTED + KEY_F2:
  case KEY_ALTED + KEY_F3:
    Gui_OpenMenu("Mission", "Saved Games");
    return true;
  case KEY_F4:
    Gui_OpenMenu("Mission", "Guide-Bot");
    return true;
  case KEY_SHIFTED + KEY_TAB:
    Gui_OpenMenu("Mission", "Objectives");
    return true;
  case KEY_SHIFTED + KEY_F8:
    Gui_OpenMenu("Mission", "Ship's Log");
    return true;
  case KEY_SHIFTED + KEY_F9:
    Gui_OpenMenu("Mission", "HUD Messages");
    return true;
  }
  return false;
}

static void RegisterGameKeys() {
  COND_VB_SHOULD(VB_PROCESS_GAME_KEY, true, {
    const int key = va_arg(args, int);
    if ((key == KEY_ESC && Cinematic_inuse) || OpenPage(key)) {
      *should = false;
    }
  });
  COND_VB_SHOULD(VB_CINEMATIC_KEY_PRESSED, true, {
    for (int i = 0; i < DDIO_MAX_KEYS && !*should; i++) {
      *should = KEY_STATE(i);
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterGameKeys);
