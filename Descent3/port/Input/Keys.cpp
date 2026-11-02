#include "Keys.h"

#include "ddio.h"
#include "ddio_common.h"

void Input_KeyHold(int key, bool isdown) {
  int modifier = (key & KEY_SHIFTED) ? KEY_LSHIFT : (key & KEY_ALTED) ? KEY_LALT : (key & KEY_CTRLED) ? KEY_LCTRL : 0;

  if (isdown) {
    if (modifier) {
      ddio_UpdateKeyState(modifier, true);
    }
    ddio_UpdateKeyState(key & 0xff, true);
  } else {
    ddio_UpdateKeyState(key & 0xff, false);
    if (modifier) {
      ddio_UpdateKeyState(modifier, false);
    }
  }
}

void Input_KeyPress(int key) {
  Input_KeyHold(key, true);
  Input_KeyHold(key, false);
}
