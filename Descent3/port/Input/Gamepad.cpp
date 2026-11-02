#include "Gamepad.h"

#include <cstdlib>
#include <cstring>
#include <iterator>

#include <SDL3/SDL.h>

#include "controller.h"
#include "ddio_common.h"
#include "joystick.h"
#include "Keys.h"
#include "log.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"

#define PAD_SDL_BUTTONS (JOYPAD_LT - 1)
#define PAD_NUM_BUTTONS JOYPAD_RT
#define PAD_TRIGGER_PRESS 16384
#define PAD_STICK_PRESS 16384
#define PAD_STICK_RELEASE 8192

static SDL_Gamepad *Pads[MAX_JOYSTICKS];

static const char *const Button_names[PAD_NUM_BUTTONS] = {
    "A",      "B",      "X",       "Y",    "Back",    "Guide",   "Start",   "L3",      "R3",       "LB", "RB", "D-Up",
    "D-Down", "D-Left", "D-Right", "Misc", "Paddle1", "Paddle2", "Paddle3", "Paddle4", "Touchpad", "LT", "RT"};
static const char *const Axis_names[] = {"Left X", "Left Y", "Right X", "Right Y", "LT", "RT"};

const char *Gamepad_ButtonLabel(int button) {
  return (button >= 1 && button <= PAD_NUM_BUTTONS) ? Button_names[button - 1] : nullptr;
}

static const char *ButtonName(int joy, unsigned btn) {
  return (joy >= 0 && joy < MAX_JOYSTICKS && Pads[joy]) ? Gamepad_ButtonLabel(btn) : nullptr;
}

static const char *AxisName(int joy, unsigned axis) {
  if (joy < 0 || joy >= MAX_JOYSTICKS || !Pads[joy] || axis < 1 || axis > std::size(Axis_names)) {
    return nullptr;
  }
  return Axis_names[axis - 1];
}

bool Gamepad_Held() {
  for (SDL_Gamepad *pad : Pads) {
    if (!pad) {
      continue;
    }
    for (int i = 0; i < PAD_SDL_BUTTONS; i++) {
      if (SDL_GetGamepadButton(pad, (SDL_GamepadButton)i)) {
        return true;
      }
    }
    if (SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER) >= PAD_TRIGGER_PRESS ||
        SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER) >= PAD_TRIGGER_PRESS) {
      return true;
    }
  }
  return false;
}

bool Gamepad_ButtonDown(int button) {
  if (button < 1 || button > PAD_NUM_BUTTONS) {
    return false;
  }
  for (SDL_Gamepad *pad : Pads) {
    if (!pad) {
      continue;
    }
    if (button == JOYPAD_LT || button == JOYPAD_RT) {
      const SDL_GamepadAxis axis = button == JOYPAD_LT ? SDL_GAMEPAD_AXIS_LEFT_TRIGGER : SDL_GAMEPAD_AXIS_RIGHT_TRIGGER;
      if (SDL_GetGamepadAxis(pad, axis) >= PAD_TRIGGER_PRESS) {
        return true;
      }
    } else if (SDL_GetGamepadButton(pad, (SDL_GamepadButton)(button - 1))) {
      return true;
    }
  }
  return false;
}

static void InitStick(va_list args) {
  const int joy = va_arg(args, int);
  SDL_Joystick *stick = va_arg(args, SDL_Joystick *);
  tJoyInfo *caps = va_arg(args, tJoyInfo *);

  if (Pads[joy]) {
    SDL_CloseGamepad(Pads[joy]);
    Pads[joy] = nullptr;
  }
  const SDL_JoystickID id = SDL_GetJoystickID(stick);
  if (!SDL_IsGamepad(id)) {
    return;
  }
  if (!SDL_WasInit(SDL_INIT_GAMEPAD) && !SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
    LOG_ERROR << "Could not initialize gamepads: " << SDL_GetError();
    return;
  }
  SDL_Gamepad *pad = SDL_OpenGamepad(id);
  if (!pad) {
    return;
  }

  memset(caps, 0, sizeof(*caps));
  strncpy(caps->name, SDL_GetGamepadName(pad), sizeof(caps->name) - 1);
  caps->num_btns = PAD_NUM_BUTTONS;
  caps->axes_mask = JOYFLAG_XVALID | JOYFLAG_YVALID | JOYFLAG_ZVALID | JOYFLAG_RVALID | JOYFLAG_UVALID | JOYFLAG_VVALID;
  caps->minx = caps->miny = caps->minz = caps->minr = caps->minu = caps->minv = -32767;
  caps->maxx = caps->maxy = caps->maxz = caps->maxr = caps->maxu = caps->maxv = 32768;
  Pads[joy] = pad;
  LOG_DEBUG.printf("JOYSTICK: Initialized gamepad named [%s].", caps->name);
}

static bool ReadStick(va_list args) {
  const int joy = va_arg(args, int);
  tJoyPos *pos = va_arg(args, tJoyPos *);
  SDL_Gamepad *pad = Pads[joy];
  if (!pad) {
    return true;
  }

  pos->x = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTX);
  pos->y = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFTY);
  pos->z = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTX);
  pos->r = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHTY);
  pos->u = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER);
  pos->v = SDL_GetGamepadAxis(pad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER);
  for (int i = 0; i < JOYPOV_NUM; ++i) {
    pos->pov[i] = JOYPOV_CENTER;
  }
  for (int i = 0; i < PAD_SDL_BUTTONS; ++i) {
    if (i != SDL_GAMEPAD_BUTTON_BACK && i != SDL_GAMEPAD_BUTTON_GUIDE && i != SDL_GAMEPAD_BUTTON_START &&
        SDL_GetGamepadButton(pad, (SDL_GamepadButton)i)) {
      pos->buttons |= (1 << i);
    }
  }
  if (pos->u >= PAD_TRIGGER_PRESS) {
    pos->buttons |= (1 << (JOYPAD_LT - 1));
  }
  if (pos->v >= PAD_TRIGGER_PRESS) {
    pos->buttons |= (1 << (JOYPAD_RT - 1));
  }
  return false;
}

static bool Menu_navigation = true;
static bool Back_held = false;
static bool Trigger_down[2] = {};
static int Stick_dir[2] = {};
static int Held_key[PAD_NUM_BUTTONS] = {};

static int MenuKey(int button) {
  switch (button) {
  case SDL_GAMEPAD_BUTTON_DPAD_UP:
    return KEY_UP;
  case SDL_GAMEPAD_BUTTON_DPAD_DOWN:
    return KEY_DOWN;
  case SDL_GAMEPAD_BUTTON_DPAD_LEFT:
    return KEY_LEFT;
  case SDL_GAMEPAD_BUTTON_DPAD_RIGHT:
    return KEY_RIGHT;
  case SDL_GAMEPAD_BUTTON_SOUTH:
    return KEY_ENTER;
  case SDL_GAMEPAD_BUTTON_EAST:
    return KEY_ESC;
  case SDL_GAMEPAD_BUTTON_LEFT_SHOULDER:
    return KEY_SHIFTED + KEY_TAB;
  case SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER:
    return KEY_TAB;
  case JOYPAD_LT - 1:
    return KEY_PAGEUP;
  case JOYPAD_RT - 1:
    return KEY_PAGEDOWN;
  }
  return 0;
}

static void Button(int button, bool down) {
  if (button >= PAD_NUM_BUTTONS) {
    return;
  }
  if (button == SDL_GAMEPAD_BUTTON_BACK) {
    if (down) {
      Back_held = true;
    } else if (Back_held) {
      Back_held = false;
      Input_KeyPress(KEY_ESC);
    }
    return;
  }
  if (!down) {
    if (Held_key[button]) {
      Input_KeyHold(Held_key[button], false);
      Held_key[button] = 0;
    }
    return;
  }

  if (button == SDL_GAMEPAD_BUTTON_START) {
    Input_KeyPress(Menu_navigation ? KEY_ENTER : KEY_PAUSE);
    return;
  }
  if (Menu_navigation) {
    if (int key = MenuKey(button)) {
      Input_KeyHold(key, true);
      Held_key[button] = key;
    }
  }
}

static void Axis(int axis, int value) {
  switch (axis) {
  case SDL_GAMEPAD_AXIS_LEFT_TRIGGER:
  case SDL_GAMEPAD_AXIS_RIGHT_TRIGGER: {
    bool &down = Trigger_down[axis - SDL_GAMEPAD_AXIS_LEFT_TRIGGER];
    bool now = value >= PAD_TRIGGER_PRESS;
    if (now != down) {
      down = now;
      Button(axis == SDL_GAMEPAD_AXIS_LEFT_TRIGGER ? JOYPAD_LT - 1 : JOYPAD_RT - 1, now);
    }
    break;
  }
  case SDL_GAMEPAD_AXIS_LEFTX:
  case SDL_GAMEPAD_AXIS_LEFTY: {
    int &dir = Stick_dir[axis];
    int now = dir;
    if (value >= PAD_STICK_PRESS) {
      now = 1;
    } else if (value <= -PAD_STICK_PRESS) {
      now = -1;
    } else if (std::abs(value) < PAD_STICK_RELEASE) {
      now = 0;
    }
    if (now != dir) {
      dir = now;
      if (now && Menu_navigation) {
        if (axis == SDL_GAMEPAD_AXIS_LEFTX) {
          Input_KeyPress(now < 0 ? KEY_LEFT : KEY_RIGHT);
        } else {
          Input_KeyPress(now < 0 ? KEY_UP : KEY_DOWN);
        }
      }
    }
    break;
  }
  }
}

static bool FilterEvent(const SDL_Event *event) {
  switch (event->type) {
  case SDL_EVENT_GAMEPAD_BUTTON_DOWN:
  case SDL_EVENT_GAMEPAD_BUTTON_UP:
    Button(event->gbutton.button, event->type == SDL_EVENT_GAMEPAD_BUTTON_DOWN);
    return true;
  case SDL_EVENT_GAMEPAD_AXIS_MOTION:
    Axis(event->gaxis.axis, event->gaxis.value);
    return true;
  }
  return false;
}

static void RegisterGamepad() {
  COND_VB_SHOULD(VB_JOY_INIT_STICK, true, { InitStick(args); });
  COND_VB_SHOULD(VB_JOY_READ_STICK, true, { *should = ReadStick(args); });
  COND_VB_SHOULD(VB_CONTROLLER_BINDING_TEXT, true, {
    const int type = va_arg(args, int);
    const int joy = va_arg(args, int);
    const unsigned bind = va_arg(args, int);
    const char **text = va_arg(args, const char **);
    if (const char *name = type == ctAxis ? AxisName(joy, bind) : ButtonName(joy, bind)) {
      *text = name;
    }
  });
  COND_VB_SHOULD(VB_GAME_HANDLES_SDL_EVENT, true, {
    const SDL_Event *event = va_arg(args, SDL_Event *);
    if (FilterEvent(event)) {
      *should = false;
    }
  });
  COND_HOOK(OnControllerSuspend, true, [] { Menu_navigation = true; });
  COND_HOOK(OnControllerResume, true, [] { Menu_navigation = false; });
}

static RegisterPortInitFunc initFunc(RegisterGamepad);
