#pragma once

// The pieces of vanilla behavior port code can change or skip through GameInteractor_Should. Each entry shows the
// expression its call site passes as `result`, then the extra arguments in order. Varargs are promoted, so bools and
// small integers arrive as int32_t and floats as double.
typedef enum {
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
  // Whether ProgramVersion saves the version in its copy of the registry.
  // #### `args`
  // - None
  VB_SAVE_PROGRAM_VERSION,

  // #### `result`
  // ```c
  // true
  // ```
  // Whether bumping a wall or a robot hard enough to hurt plays the damage sound, besides the bump sound.
  // #### `args`
  // - None
  VB_WALL_DAMAGE_PLAYS_SOUND,
} GIVanillaBehavior;
