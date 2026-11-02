#pragma once

#define CVAR_STRETCH "gSettings.Resolution.Stretch"
#define CVAR_PIXEL_PERFECT "gSettings.Resolution.PixelPerfect"
#define CVAR_MENU_FILL "gSettings.Menus.Fill"
#define CVAR_MOVIE_FILL "gSettings.Movies.Fill"
#ifdef D3_GLES
#define FILL_DEFAULT 1
#else
#define FILL_DEFAULT 0
#endif
