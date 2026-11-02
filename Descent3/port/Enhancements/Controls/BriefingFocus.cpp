#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "TelCom.h"
#include "TelComEfxStructs.h"

static void RegisterBriefingFocus() {
  COND_VB_SHOULD(VB_BRIEFING_BUTTON_FOCUSABLE, true, {
    TCBUTTONDESC *desc = va_arg(args, TCBUTTONDESC *);
    desc->osflags |= OBF_MOUSEOVERFOCUS;
    *should = true;
  });
  COND_VB_SHOULD(VB_TELCOM_ARROW_MOVES_FOCUS, true, {
    if (Telcom_system.current_status == TS_MISSION) {
      *should = true;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterBriefingFocus);
