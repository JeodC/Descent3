#pragma once

// The pieces of vanilla behavior port code can change or skip through GameInteractor_Should. Each entry shows the
// expression its call site passes as `result`, then the extra arguments in order. Varargs are promoted, so bools and
// small integers arrive as int32_t and floats as double.
typedef enum {
  // #### `result`
  // ```c
  // false
  // ```
  // Whether the briefing's button is a tab stop.
  // #### `args`
  // - `*TCBUTTONDESC` (desc)
  VB_BRIEFING_BUTTON_FOCUSABLE,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the name a joystick's axis or button shows in bindings.
  // #### `args`
  // - `int32_t` (type, ctAxis or ctButton)
  // - `int32_t` (m_ControlList[ctrl].id, the joystick)
  // - `int32_t` (bind, promoted from `uint8_t`)
  // - `**const char` (str)
  VB_CONTROLLER_BINDING_TEXT,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the weapon cycling buttons switch weapons this frame.
  // #### `args`
  // - `*ct_packet` (cycle_prim)
  // - `*ct_packet` (cycle_sec)
  VB_CYCLE_WEAPONS,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the registry writes .Descent3Registry.
  // #### `args`
  // - `*CRegistry` (this)
  VB_EXPORT_REGISTRY,

  // #### `result`
  // ```c
  // force_res_change
  // ```
  // Whether the game window takes the whole screen, or else the pilot's saved size.
  // #### `args`
  // - None
  VB_FULL_GAME_WINDOW,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the game's event filter handles the event, rather than dropping it.
  // #### `args`
  // - `*SDL_Event` (event)
  VB_GAME_HANDLES_SDL_EVENT,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the size the game view renders at, which starts as the screen's.
  // #### `args`
  // - `*int` (scr_width)
  // - `*int` (scr_height)
  VB_GAME_RESOLUTION,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may move where the HUD draws an item, in screen pixels.
  // #### `args`
  // - `*int` (x)
  // - `*int` (y)
  VB_HUD_POSITION,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the HUD's scale from its 640x480 layout.
  // #### `args`
  // - `*float` (Hud_aspect_x)
  // - `*float` (Hud_aspect_y)
  VB_HUD_SCALE,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the stick's description.
  // #### `args`
  // - `int32_t` (joy)
  // - `*SDL_Joystick` (stick)
  // - `*tJoyInfo` (caps)
  VB_JOY_INIT_STICK,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the stick's axes and buttons are read the vanilla way, into a cleared pos.
  // #### `args`
  // - `int32_t` (joy)
  // - `*tJoyPos` (pos)
  VB_JOY_READ_STICK,

  // #### `result`
  // ```c
  // m_iProtectedFlags & DMFC_PRF_INOPTIONS
  // ```
  // A listener may follow whether the netgame's on-screen menu (F6) is up, asked once a frame.
  // #### `args`
  // - None
  VB_NETGAME_MENU_UP,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the highest level New Game offers.
  // #### `args`
  // - `*int` (highest)
  VB_NEW_GAME_HIGHEST_LEVEL,

  // #### `result`
  // ```c
  // scalar > .01, or with || (robotobj->mtype.phys_info.flags & PF_LOCK_MASK) for a robot
  // ```
  // Whether a ship bumping a ship or a robot makes a sound, and lets the robots hear it.
  // #### `args`
  // - `int32_t` (the player's id)
  VB_PLAY_CONTACT_SOUND,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether a hit on the player's ship plays its sound.
  // #### `args`
  // - `*object` (playerobj)
  // - `int32_t` (the sound)
  VB_PLAY_PLAYER_HIT_SOUND,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the size the frame is drawn at in the window, which it's centered in, and the part of the
  // frame drawn, in pixels from its bottom left.
  // #### `args`
  // - `*int` (scaledWidth)
  // - `*int` (scaledHeight)
  // - `int32_t` (w, the window's width in pixels)
  // - `int32_t` (h, the window's height in pixels)
  // - `*int` (sourceX)
  // - `*int` (sourceY)
  // - `*int` (sourceWidth, GOpenGLFBOWidth to start with)
  // - `*int` (sourceHeight, GOpenGLFBOHeight to start with)
  VB_REND_FRAME_SIZE,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the frame is scaled to the window with linear filtering, or else nearest.
  // #### `args`
  // - None
  VB_REND_LINEAR_FILTER,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the description the save dialog starts with.
  // #### `args`
  // - `*char` (desc)
  // - `int32_t` (sizeof(desc))
  VB_SAVE_DESCRIPTION,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether ProgramVersion saves the version in its copy of the registry.
  // #### `args`
  // - None
  VB_SAVE_PROGRAM_VERSION,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether menus and the TelCom show the mouse cursor.
  // #### `args`
  // - None
  VB_SHOW_MOUSE_CURSOR,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the game opens the options menu, between frames.
  // #### `args`
  // - None
  VB_SHOW_OPTIONS_MENU,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether New Game's mission list sorts its missions by name.
  // #### `args`
  // - None
  VB_SORT_MISSION_LIST,

  // #### `result`
  // ```c
  // false
  // ```
  // Whether Up or Down moves focus, as Shift+Tab or Tab does, when there's no text to scroll.
  // #### `args`
  // - None
  VB_TELCOM_ARROW_MOVES_FOCUS,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether the gadget in focus processes this frame's key.
  // #### `args`
  // - `*UIGadget` (m_GadgetCur)
  VB_UI_GADGET_PROCESS_KEYS,

  // #### `result`
  // ```c
  // true
  // ```
  // A listener may change the key the window handles.
  // #### `args`
  // - `*int` (key)
  // - `*UIGadget` (m_GadgetCur, the gadget in focus, may be null)
  // - `int32_t` (m_CurGadgetInGroup, promoted from `bool`)
  VB_UI_WINDOW_KEY_DOWN,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether bumping a wall or a robot hard enough to hurt plays the damage sound, besides the bump sound.
  // #### `args`
  // - None
  VB_WALL_DAMAGE_PLAYS_SOUND,
} GIVanillaBehavior;
