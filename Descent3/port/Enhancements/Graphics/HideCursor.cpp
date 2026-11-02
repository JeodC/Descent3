#include "HideCursor.h"

#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

static void RegisterHideCursor() {
  COND_VB_SHOULD(VB_SHOW_MOUSE_CURSOR, CVarGetInteger(CVAR_HIDE_CURSOR, HIDE_CURSOR_DEFAULT), { *should = false; });
}

static RegisterPortInitFunc initFunc(RegisterHideCursor, {CVAR_HIDE_CURSOR});
