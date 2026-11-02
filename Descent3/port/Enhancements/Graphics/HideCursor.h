#pragma once

#define CVAR_HIDE_CURSOR "gSettings.HideMouseCursor"
#ifdef D3_GLES
#define HIDE_CURSOR_DEFAULT true
#else
#define HIDE_CURSOR_DEFAULT false
#endif
