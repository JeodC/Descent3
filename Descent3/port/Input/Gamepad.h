#pragma once

enum {
  JOYPAD_A = 1,
  JOYPAD_B,
  JOYPAD_X,
  JOYPAD_Y,
  JOYPAD_BACK,
  JOYPAD_GUIDE,
  JOYPAD_START,
  JOYPAD_L3,
  JOYPAD_R3,
  JOYPAD_LB,
  JOYPAD_RB,
  JOYPAD_DPAD_UP,
  JOYPAD_DPAD_DOWN,
  JOYPAD_DPAD_LEFT,
  JOYPAD_DPAD_RIGHT,
  JOYPAD_LT = 22,
  JOYPAD_RT
};

const char *Gamepad_ButtonLabel(int button);
bool Gamepad_Held();
bool Gamepad_ButtonDown(int button);
