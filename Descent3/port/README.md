# Port hooks

This fork is upstream Descent 3 plus the features its handheld build needs. The features live in `Descent3/port/`. Upstream files only gain hook calls and a few marked lines, so rebasing onto upstream conflicts only at those spots.

The hook system is the GameInteractor from HarbourMasters' [Ship of Harkinian](https://github.com/HarbourMasters/Shipwright) and [2 Ship 2 Harkinian](https://github.com/HarbourMasters/2ship2harkinian).

## Layout

| Path | Holds |
|--|--|
| `GameInteractor/` | The hook registry, the event hook table and the vanilla behavior enum |
| `Enhancements/<Category>/` | One feature per file |
| `Input/` | The gamepad layer |
| `UI/` | The ImGui menu and its pages |
| `Settings/` | `descent3.cfg.json` |
| `ModuleBridge/` | `GameInteractor_Should` for the netcon and netgame modules |
| `Port.cmake` | Adds every file here to the `Descent3` target, with ImGui and nlohmann/json |

## Startup

`main()` calls `Port_Startup()` before anything else. It loads `descent3.cfg.json`, creates the `GameInteractor`, then runs every function registered with `RegisterPortInitFunc`. Features register their listeners there, so they apply from the first frame on.

## Vanilla behavior hooks

A vanilla behavior (VB) hook wraps a decision the game makes. The original expression becomes the `result` argument, and the original code stays as it was:

```cpp
if (GameInteractor_Should(VB_PLAY_PLAYER_HIT_SOUND, true, playerobj, sound_handle))
  Sound_system.Play3dSound(sound_handle, SND_PRIORITY_HIGHEST, playerobj);
```

With no listener, `GameInteractor_Should` returns `result` and the game behaves as upstream does. A listener can change the answer, and read the extra arguments:

```cpp
COND_VB_SHOULD(VB_PLAY_PLAYER_HIT_SOUND, limit, {
  PlayHitSound(args);
  *should = false;
});
```

Some sites call `GameInteractor_Should` only so listeners can change values passed by pointer. They ignore the return value, and a comment above the call says so.

[`GameInteractor_VanillaBehavior.h`](GameInteractor/GameInteractor_VanillaBehavior.h) lists every VB with its `result` expression and its arguments in order. Varargs are promoted, so bools and small integers arrive as `int32_t` and floats as `double`.

## Event hooks

An event hook tells listeners something happened. Each is a line in [`GameInteractor_HookTable.h`](GameInteractor/GameInteractor_HookTable.h), with an `ExecuteOn` function in `GameInteractor_Hooks.h` and `GameInteractor.cpp`.

| Hook | Called from | When |
|--|--|--|
| `OnControllerSuspend`, `OnControllerResume` | `ddio/sdlcontroller.cpp` | The game stops or starts reading the controls |
| `OnControlsProcessed` | `Descent3/GameLoop.cpp` | The frame's keys and buttons have been processed |
| `OnDatabaseCreated` | `Descent3/sdlmain.cpp` | The game has created its settings database |
| `OnLevelWarpListFilled` | `Descent3/menu.cpp` | New Game's level list is filled |
| `OnMissionListFilled` | `Descent3/menu.cpp` | New Game's mission list is filled |
| `OnMissionStatusUpdate` | `Descent3/pilot.cpp` | The pilot's mission progress is updated |
| `OnMovieFrame` | `Descent3/d3movie.cpp` | A movie frame has been drawn, at the given place and size |
| `OnMovieEnd` | `Descent3/d3movie.cpp` | A movie ends |
| `OnRendContextCreated`, `OnRendContextClosing` | `renderer/HardwareOpenGL.cpp` | The GL context is created or about to close |
| `OnRendFlip` | `renderer/HardwareOpenGL.cpp` | The frame is about to be shown |

Hooks should be idempotent, and the order listeners run in isn't guaranteed.

## Features

| File | What it does | Hooks |
|--|--|--|
| `Enhancements/Audio/HitSoundLimit.cpp` | Plays one hit sound at a time per ship | `VB_PLAY_PLAYER_HIT_SOUND`, `VB_PLAY_CONTACT_SOUND`, `VB_WALL_DAMAGE_PLAYS_SOUND` |
| `Enhancements/Controls/ArrowFocus.cpp` | Moves focus with the arrow keys in the old dialogs | `VB_UI_GADGET_PROCESS_KEYS`, `VB_UI_WINDOW_KEY_DOWN` |
| `Enhancements/Controls/BriefingFocus.cpp` | Lets the briefing's buttons take focus | `VB_BRIEFING_BUTTON_FOCUSABLE`, `VB_TELCOM_ARROW_MOVES_FOCUS` |
| `Enhancements/Controls/DropMarker.cpp` | Drops a marker with a gamepad button | `OnControlsProcessed` |
| `Enhancements/Controls/GameKeys.cpp` | Opens the menu's pages from the keys for the old screens, and lets the gamepad skip cinematics | `VB_PROCESS_GAME_KEY`, `VB_CINEMATIC_KEY_PRESSED` |
| `Enhancements/Controls/GamepadDefaults.cpp` | Sets the handheld default bindings in `Controller_needs` | None |
| `Enhancements/Graphics/HideCursor.cpp` | Hides the mouse cursor | `VB_SHOW_MOUSE_CURSOR` |
| `Enhancements/Graphics/Resolution.cpp` | Sets the game view's aspect ratio and resolution | `VB_GAME_RESOLUTION`, `VB_FULL_GAME_WINDOW`, `VB_SHOW_OPTIONS_MENU` |
| `Enhancements/Graphics/Scaling.cpp` | Scales the frame to the window, fills it with menus, and scales movies without their black bands | `VB_REND_FRAME_SIZE`, `VB_REND_LINEAR_FILTER`, `OnMovieFrame`, `OnMovieEnd` |
| `Enhancements/Graphics/UniformHud.cpp` | Scales the HUD uniformly and centers it | `VB_HUD_SCALE`, `VB_HUD_POSITION` |
| `Enhancements/Menus/MainMenu.cpp` | Opens the menu from the main menu's Options and Pilot | `VB_OPEN_MAIN_MENU_ITEM` |
| `Enhancements/Menus/NewGame.cpp` | Puts Retribution and Mercenary first, and offers every level of a finished mission | `VB_SORT_MISSION_LIST`, `VB_NEW_GAME_HIGHEST_LEVEL`, `OnMissionListFilled` |
| `Enhancements/Menus/SaveSlots.cpp` | Names saves after their level, and saves to the slot picked in the menu | `VB_SAVE_DESCRIPTION`, `VB_SHOW_SAVE_DIALOG` |
| `Enhancements/Multiplayer/NetgameMenu.cpp` | Stops the controls while the netgame's on-screen menu is up | `VB_NETGAME_MENU_UP` |
| `Enhancements/Multiplayer/ObserverCycling.cpp` | Cycles observed players with the weapon cycling buttons | `VB_CYCLE_WEAPONS`, `OnControlsProcessed` |
| `Enhancements/Progress/FlaggedLevels.cpp` | Shows the levels reached after cheating in New Game, grayed out | `OnMissionStatusUpdate`, `OnLevelWarpListFilled` |
| `Input/Gamepad.cpp` | Reads pads through SDL's gamepad layout, navigates menus, names bindings and opens the menu on SELECT | `VB_JOY_INIT_STICK`, `VB_JOY_READ_STICK`, `VB_CONTROLLER_BINDING_TEXT`, `VB_GAME_HANDLES_SDL_EVENT`, `OnControllerSuspend`, `OnControllerResume` |
| `Settings/JsonDatabase.cpp` | Keeps the game's settings in `descent3.cfg.json` | `OnDatabaseCreated`, `VB_EXPORT_REGISTRY`, `VB_SAVE_PROGRAM_VERSION` |
| `UI/Gui.cpp` | Draws the menu each frame, and restarts or quits a level without the game's dialogs | `OnRendContextCreated`, `OnRendContextClosing`, `OnRendFlip`, `VB_CONFIRM_EXIT_GAME`, `VB_SHOW_LEVEL_WARP_DIALOG` |
| `UI/GuiMission.cpp` | Opens the TelCom on the briefing | `VB_TELCOM_OPENS_MAIN_MENU` |

## Settings

CVars are kept in `descent3.cfg.json` under `CVars`, nested at each dot of the name, so `gEnhancements.Audio.HitSoundLimit` is `CVars.gEnhancements.Audio.HitSoundLimit`. `CVarGetInteger(name, default)` reads one. The game's own settings, which upstream keeps in `.Descent3Registry`, go in the same file. It's saved when the menu closes and when the game exits.

A feature lists the CVars it reads in its `RegisterPortInitFunc`, and the menu reruns its function when one of them changes:

```cpp
static RegisterPortInitFunc initFunc(RegisterHitSoundLimit, {CVAR_HIT_SOUND_LIMIT});
```

## The module bridge

The netcon (`.d3c`) and netgame (`.d3m`) modules link `ddio`, `ui`, `physics` and `dmfc` statically, so hook calls in those libraries end up inside the modules, away from the executable's `GameInteractor`. Those libraries link `port_module_bridge`, whose `GameInteractor_Should` looks up `GameInteractor_ShouldV` exported by the executable and forwards to it. Without it, the call returns `result`.

Only VB hooks cross the bridge, so code a module links can't call an event hook. `sdlcontroller.cpp` can, because no module uses it.

## Adding a feature

1. Add a VB to `GameInteractor_VanillaBehavior.h`, in alphabetical order with its `result` and `args` documented. For an event hook, add its line to the hook table and its `ExecuteOn` function to `GameInteractor_Hooks.h` and `GameInteractor.cpp`.
2. Call it at the site. Upstream files include `port/GameInteractor/GameInteractor_Hooks.h`.
3. Write the feature in `Enhancements/<Category>/<Feature>.cpp`, ending with its `RegisterPortInitFunc`. `Port.cmake` picks up new files.

## Outside `port/`

Every other change to upstream files carries a mark:

- `// RHH [Bugfix] reason`: a fix for a bug in upstream's code
- `// RHH [Port] reason`: port code that isn't a hook call, such as an accessor the menu reads file-static state through
- `// #region RHH [Bugfix|Port] reason` ... `// #endregion`: a block of either

Hook call sites carry no mark. `git grep "RHH \["` lists the marked lines.

The `D3_GLES` CMake option, off by default, renders through OpenGL ES 3.0. The handheld build turns it on.
