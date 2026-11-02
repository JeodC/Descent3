#include "controls.h"
#include "game.h"
#include "marker.h"
#include "object.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/Input/Gamepad.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

#define CVAR_DROP_MARKER_BUTTON "gControls.DropMarkerButton"

static bool Was_down = false;

static void ReadDropMarker() {
  const bool down = Gamepad_ButtonDown(CVarGetInteger(CVAR_DROP_MARKER_BUTTON, 0));
  if (down && !Was_down && Control_poll_flag && Player_object && Player_object->type == OBJ_PLAYER) {
    char message[] = "";
    DropMarker(message);
  }
  Was_down = down;
}

static void RegisterDropMarker() {
  COND_HOOK(OnControlsProcessed, CVarGetInteger(CVAR_DROP_MARKER_BUTTON, 0), ReadDropMarker);
}

static RegisterPortInitFunc initFunc(RegisterDropMarker, {CVAR_DROP_MARKER_BUTTON});
