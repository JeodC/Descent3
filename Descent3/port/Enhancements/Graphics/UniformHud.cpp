#include <algorithm>

#include "game.h"
#include "hud.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

#define CVAR_UNIFORM_HUD "gEnhancements.Graphics.UniformHud"

static void RegisterUniformHud() {
  const bool uniform = CVarGetInteger(CVAR_UNIFORM_HUD, 1);
  COND_VB_SHOULD(VB_HUD_SCALE, uniform, {
    float *aspect_x = va_arg(args, float *);
    float *aspect_y = va_arg(args, float *);
    *aspect_x = *aspect_y = std::min(*aspect_x, *aspect_y);
  });
  COND_VB_SHOULD(VB_HUD_POSITION, uniform, {
    int *x = va_arg(args, int *);
    int *y = va_arg(args, int *);
    *x += (int)((Game_window_w - DEFAULT_HUD_WIDTH * Hud_aspect_x) / 2);
    *y += (int)((Game_window_h - DEFAULT_HUD_HEIGHT * Hud_aspect_y) / 2);
  });
}

static RegisterPortInitFunc initFunc(RegisterUniformHud, {CVAR_UNIFORM_HUD});
