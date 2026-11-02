#pragma once

#define CVAR_VIEW_ASPECT "gSettings.Resolution.Aspect"
#define CVAR_VIEW_HEIGHT "gSettings.Resolution.Height"

struct tViewAspect {
  const char *name;
  int x, y;
};
extern const tViewAspect View_aspects[];
extern const int N_VIEW_ASPECTS;
extern const int View_heights[];
extern const int N_VIEW_HEIGHTS;

void Resolution_ScreenSize(int *width, int *height);
void Resolution_ViewSize(int *width, int *height);
void Resolution_Apply();
