#include <cstring>
#include <typeinfo>

#include "ddio_common.h"
#include "newui_core.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"
#include "ui.h"

static bool UsesArrowKey(UIGadget *gadget, int key) {
  const bool vertical = key == KEY_UP || key == KEY_DOWN;
  switch (gadget->Class()) {
  case uiEdit:
  case uiSlider:
    return !vertical;
  case uiListBox:
    return vertical;
  }
  if (dynamic_cast<newuiMenuOptionButton *>(gadget)) {
    return true;
  }
  if (dynamic_cast<newuiListBox *>(gadget) || dynamic_cast<newuiComboBox *>(gadget) ||
      dynamic_cast<UIComboBox *>(gadget)) {
    return vertical;
  }
  if (strstr(typeid(*gadget).name(), "newuiSlider")) {
    return !vertical;
  }
  return false;
}

static bool Moved_focus = false;

static void MoveFocus(va_list args) {
  int *key = va_arg(args, int *);
  UIGadget *focus = va_arg(args, UIGadget *);
  const bool in_group = va_arg(args, int);
  if (in_group || !focus || UsesArrowKey(focus, *key)) {
    return;
  }
  if (*key == KEY_DOWN || *key == KEY_RIGHT) {
    *key = KEY_TAB;
    Moved_focus = true;
  } else if (*key == KEY_UP || *key == KEY_LEFT) {
    *key = KEY_SHIFTED + KEY_TAB;
    Moved_focus = true;
  }
}

static void RegisterArrowFocus() {
  COND_VB_SHOULD(VB_UI_WINDOW_KEY_DOWN, true, { MoveFocus(args); });
  COND_VB_SHOULD(VB_UI_GADGET_PROCESS_KEYS, true, {
    if (Moved_focus) {
      Moved_focus = false;
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterArrowFocus);
