#include "controls.h"
#include "joystick.h"
#include "port/Input/Gamepad.h"
#include "port/PortInit.hpp"
#include "port/Settings/CVar.h"

#define CVAR_GAMEPAD_DEFAULTS "gControls.GamepadDefaults"

struct tPadDefault {
  int id;
  ct_type ctype;
  uint8_t value;
  uint8_t flags;
};

static const tPadDefault Pad_defaults[] = {
    {ctfFORWARD_THRUSTAXIS, ctAxis, CT_Y_AXIS, 0},
    {ctfUP_THRUSTAXIS, ctAxis, 0, 0},
    {ctfUP_BUTTON, ctButton, JOYPAD_DPAD_UP, 0},
    {ctfDOWN_BUTTON, ctButton, JOYPAD_DPAD_DOWN, 0},
    {ctfRIGHT_THRUSTAXIS, ctAxis, CT_X_AXIS, 0},
    {ctfPITCH_DOWNAXIS, ctAxis, CT_R_AXIS, CTFNF_INVERT},
    {ctfBANK_RIGHTAXIS, ctAxis, 0, 0},
    {ctfHEADING_RIGHTAXIS, ctAxis, CT_Z_AXIS, 0},
    {ctfFIREPRIMARY_BUTTON, ctButton, JOYPAD_RT, 0},
    {ctfFIRESECONDARY_BUTTON, ctButton, JOYPAD_LT, 0},
    {ctfFIREFLARE_BUTTON, ctButton, JOYPAD_X, 0},
    {ctfAFTERBURN_BUTTON, ctButton, JOYPAD_L3, 0},
    {ctfHEADLIGHT_BUTTON, ctButton, JOYPAD_R3, 0},
    {ctfPREV_INVBTN, ctButton, JOYPAD_DPAD_LEFT, 0},
    {ctfNEXT_INVBTN, ctButton, JOYPAD_DPAD_RIGHT, 0},
    {ctfINV_USEBTN, ctButton, JOYPAD_B, 0},
    {ctfCNTMS_USEBTN, ctButton, JOYPAD_Y, 0},
    {ctfWPNSEL_PCYCLEBTN, ctButton, JOYPAD_LB, 0},
    {ctfWPNSEL_SCYCLEBTN, ctButton, JOYPAD_RB, 0},
    {ctfREARVIEW_BTN, ctButton, JOYPAD_A, 0},
};

static void UseGamepadDefaults() {
  if (!CVarGetInteger(CVAR_GAMEPAD_DEFAULTS, 1)) {
    return;
  }
  for (const tPadDefault &pad : Pad_defaults) {
    for (ct_function &need : Controller_needs) {
      if (need.id == pad.id) {
        need.ctype[0] = pad.ctype;
        need.value[0] = pad.value;
        need.flags[0] = pad.flags;
        break;
      }
    }
  }
}

static RegisterPortInitFunc initFunc(UseGamepadDefaults);
