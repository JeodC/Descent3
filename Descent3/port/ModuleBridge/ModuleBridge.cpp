#include <cstdarg>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#endif

#include "port/GameInteractor/GameInteractor_Hooks.h"

using ShouldVFn = bool (*)(GIVanillaBehavior, uint32_t, va_list);

static ShouldVFn GameShouldV() {
  static const ShouldVFn fn = [] {
#if defined(_WIN32)
    return reinterpret_cast<ShouldVFn>(GetProcAddress(GetModuleHandle(nullptr), "GameInteractor_ShouldV"));
#else
    return reinterpret_cast<ShouldVFn>(dlsym(RTLD_DEFAULT, "GameInteractor_ShouldV"));
#endif
  }();
  return fn;
}

bool GameInteractor_Should(GIVanillaBehavior flag, uint32_t result, ...) {
  ShouldVFn shouldV = GameShouldV();
  if (!shouldV) {
    return result;
  }
  va_list args;
  va_start(args, result);
  bool should = shouldV(flag, result, args);
  va_end(args);
  return should;
}
