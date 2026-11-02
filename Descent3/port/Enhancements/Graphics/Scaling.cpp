#include "Scaling.h"

#include <algorithm>

#include "args.h"
#include "game.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

static int Movie_x, Movie_y, Movie_w = 0, Movie_h;

static void TrackMovie(int x, int y, int width, int height) {
  Movie_x = x;
  Movie_y = y;
  Movie_w = width;
  Movie_h = height;
}

static void ScaleFrame(va_list args) {
  int *width = va_arg(args, int *);
  int *height = va_arg(args, int *);
  const int window_w = va_arg(args, int);
  const int window_h = va_arg(args, int);
  int *source_x = va_arg(args, int *);
  int *source_y = va_arg(args, int *);
  int *source_w = va_arg(args, int *);
  int *source_h = va_arg(args, int *);
  bool fill;
  if (Movie_w) {
    if (!FindArg("-subtitles")) {
      *source_x = Movie_x;
      *source_y = *source_h - Movie_y - Movie_h;
      *source_w = Movie_w;
      *source_h = Movie_h;
      const float scale = std::min((float)window_w / Movie_w, (float)window_h / Movie_h);
      *width = (int)(Movie_w * scale);
      *height = (int)(Movie_h * scale);
    }
    fill = CVarGetInteger(CVAR_MOVIE_FILL, FILL_DEFAULT);
  } else if (GetScreenMode() != SM_GAME) {
    fill = CVarGetInteger(CVAR_MENU_FILL, FILL_DEFAULT);
  } else if (CVarGetInteger(CVAR_PIXEL_PERFECT, 0)) {
    const int scale = std::max(1, std::min(window_w / *source_w, window_h / *source_h));
    *width = *source_w * scale;
    *height = *source_h * scale;
    return;
  } else {
    fill = CVarGetInteger(CVAR_STRETCH, 0);
  }
  if (fill) {
    *width = window_w;
    *height = window_h;
  }
}

static void RegisterScaling() {
  const bool pixel_perfect = CVarGetInteger(CVAR_PIXEL_PERFECT, 0);
  COND_HOOK(OnMovieFrame, true, TrackMovie);
  COND_HOOK(OnMovieEnd, true, [] { Movie_w = 0; });
  COND_VB_SHOULD(VB_REND_FRAME_SIZE, true, { ScaleFrame(args); });
  COND_VB_SHOULD(VB_REND_LINEAR_FILTER, pixel_perfect, {
    if (GetScreenMode() == SM_GAME) {
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterScaling,
                                     {CVAR_STRETCH, CVAR_PIXEL_PERFECT, CVAR_MENU_FILL, CVAR_MOVIE_FILL});
