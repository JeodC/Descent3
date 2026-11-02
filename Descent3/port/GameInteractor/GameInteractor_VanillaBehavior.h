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
  // true
  // ```
  // Whether ProgramVersion saves the version in its copy of the registry.
  // #### `args`
  // - None
  VB_SAVE_PROGRAM_VERSION,
} GIVanillaBehavior;
