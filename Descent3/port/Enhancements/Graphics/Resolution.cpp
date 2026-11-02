#include "Resolution.h"

#include "config.h"
#include "game.h"
#include "gamesequence.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

const tViewAspect View_aspects[] = {
    {"Screen", 0, 0},
    {"Original (4:3)", 4, 3},
    {"16:10", 16, 10},
    {"Widescreen (16:9)", 16, 9},
    {"Ultrawide (21:9)", 21, 9},
};
const int N_VIEW_ASPECTS = sizeof(View_aspects) / sizeof(View_aspects[0]);
const int View_heights[] = {240, 360, 480, 540, 720, 1080, 1440};
const int N_VIEW_HEIGHTS = sizeof(View_heights) / sizeof(View_heights[0]);

static bool Resize_pending = false;

void Resolution_ScreenSize(int *width, int *height) {
  const tVideoResolution &screen = Video_res_list[Current_video_resolution_id];
  *width = screen.width;
  *height = screen.height;
}

void Resolution_ViewSize(int *width, int *height) {
  int screen_w, screen_h;
  Resolution_ScreenSize(&screen_w, &screen_h);
  const int view_h = CVarGetInteger(CVAR_VIEW_HEIGHT, 0);
  const int aspect = CVarGetInteger(CVAR_VIEW_ASPECT, 0);
  *height = (view_h > 0 && view_h < screen_h) ? view_h : screen_h;
  *width = (aspect > 0 && aspect < N_VIEW_ASPECTS) ? *height * View_aspects[aspect].x / View_aspects[aspect].y
                                                   : *height * screen_w / screen_h;
  *width &= ~1;
}

void Resolution_Apply() {
  if (GetScreenMode() == SM_GAME) {
    Resize_pending = true;
    Game_interface_mode = GAME_OPTIONS_INTERFACE;
  }
}

static void RegisterResolution() {
  COND_VB_SHOULD(VB_GAME_RESOLUTION, CVarGetInteger(CVAR_VIEW_ASPECT, 0) || CVarGetInteger(CVAR_VIEW_HEIGHT, 0), {
    int *width = va_arg(args, int *);
    int *height = va_arg(args, int *);
    Resolution_ViewSize(width, height);
  });
  COND_VB_SHOULD(VB_FULL_GAME_WINDOW, true, { *should = true; });
  COND_VB_SHOULD(VB_SHOW_OPTIONS_MENU, true, {
    if (Resize_pending) {
      Resize_pending = false;
      SetScreenMode(SM_GAME, true);
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterResolution, {CVAR_VIEW_ASPECT, CVAR_VIEW_HEIGHT});
