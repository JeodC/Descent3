#pragma once

#include <cstdint>
#include <functional>
#include <unordered_map>
#include <utility>
#include <vector>

#include "GameInteractor_Hooks.h"

typedef uint32_t HOOK_ID;

// Hooks should be idempotent and execution order is not guaranteed. If two operations must happen in a specific order,
// they should be placed in the same hook.
class GameInteractor {
public:
  static GameInteractor *Instance;

  // Game Hooks
  HOOK_ID nextHookId = 1;

  template <typename H> struct RegisteredGameHooks {
    inline static std::unordered_map<HOOK_ID, typename H::fn> functions;
    inline static std::unordered_map<int32_t, std::unordered_map<HOOK_ID, typename H::fn>> functionsForID;
    inline static std::unordered_map<uintptr_t, std::unordered_map<HOOK_ID, typename H::fn>> functionsForPtr;
    inline static std::unordered_map<HOOK_ID, std::pair<typename H::filter, typename H::fn>> functionsForFilter;
  };
  template <typename H> struct HooksToUnregister {
    inline static std::vector<HOOK_ID> hooks;
    inline static std::vector<HOOK_ID> hooksForID;
    inline static std::vector<HOOK_ID> hooksForPtr;
    inline static std::vector<HOOK_ID> hooksForFilter;
  };

  // General Hooks
  template <typename H> HOOK_ID RegisterGameHook(typename H::fn h) {
    if (this->nextHookId == 0 || this->nextHookId >= UINT32_MAX)
      this->nextHookId = 1;
    while (RegisteredGameHooks<H>::functions.find(this->nextHookId) != RegisteredGameHooks<H>::functions.end()) {
      this->nextHookId++;
    }

    RegisteredGameHooks<H>::functions[this->nextHookId] = h;
    return this->nextHookId++;
  }
  template <typename H> void UnregisterGameHook(HOOK_ID hookId) {
    if (hookId == 0)
      return;
    HooksToUnregister<H>::hooks.push_back(hookId);
  }
  template <typename H, typename... Args> void ExecuteHooks(Args &&...args) {
    // Remove pending hooks for this type
    for (auto &hookId : HooksToUnregister<H>::hooks) {
      RegisteredGameHooks<H>::functions.erase(hookId);
    }
    HooksToUnregister<H>::hooks.clear();
    // Execute hooks
    for (auto &hook : RegisteredGameHooks<H>::functions) {
      hook.second(std::forward<Args>(args)...);
    }
  }

  // ID based Hooks
  template <typename H> HOOK_ID RegisterGameHookForID(int32_t id, typename H::fn h) {
    if (this->nextHookId == 0 || this->nextHookId >= UINT32_MAX)
      this->nextHookId = 1;
    while (RegisteredGameHooks<H>::functionsForID[id].find(this->nextHookId) !=
           RegisteredGameHooks<H>::functionsForID[id].end()) {
      this->nextHookId++;
    }

    RegisteredGameHooks<H>::functionsForID[id][this->nextHookId] = h;
    return this->nextHookId++;
  }
  template <typename H> void UnregisterGameHookForID(HOOK_ID hookId) {
    if (hookId == 0)
      return;
    HooksToUnregister<H>::hooksForID.push_back(hookId);
  }
  template <typename H, typename... Args> void ExecuteHooksForID(int32_t id, Args &&...args) {
    // Remove pending hooks for this type
    for (auto hookIdIt = HooksToUnregister<H>::hooksForID.begin();
         hookIdIt != HooksToUnregister<H>::hooksForID.end();) {
      bool remove = false;

      if (RegisteredGameHooks<H>::functionsForID[id].size() == 0) {
        break;
      }

      for (auto it = RegisteredGameHooks<H>::functionsForID[id].begin();
           it != RegisteredGameHooks<H>::functionsForID[id].end();) {
        if (it->first == *hookIdIt) {
          it = RegisteredGameHooks<H>::functionsForID[id].erase(it);
          remove = true;
          break;
        } else {
          ++it;
        }
      }

      if (remove) {
        hookIdIt = HooksToUnregister<H>::hooksForID.erase(hookIdIt);
      } else {
        ++hookIdIt;
      }
    }
    // Execute hooks
    for (auto &hook : RegisteredGameHooks<H>::functionsForID[id]) {
      hook.second(std::forward<Args>(args)...);
    }
  }

  // PTR based Hooks
  template <typename H> HOOK_ID RegisterGameHookForPtr(uintptr_t ptr, typename H::fn h) {
    if (this->nextHookId == 0 || this->nextHookId >= UINT32_MAX)
      this->nextHookId = 1;
    while (RegisteredGameHooks<H>::functionsForPtr[ptr].find(this->nextHookId) !=
           RegisteredGameHooks<H>::functionsForPtr[ptr].end()) {
      this->nextHookId++;
    }

    RegisteredGameHooks<H>::functionsForPtr[ptr][this->nextHookId] = h;
    return this->nextHookId++;
  }
  template <typename H> void UnregisterGameHookForPtr(HOOK_ID hookId) {
    if (hookId == 0)
      return;
    HooksToUnregister<H>::hooksForPtr.push_back(hookId);
  }
  template <typename H, typename... Args> void ExecuteHooksForPtr(uintptr_t ptr, Args &&...args) {
    // Remove pending hooks for this type
    for (auto hookIdIt = HooksToUnregister<H>::hooksForPtr.begin();
         hookIdIt != HooksToUnregister<H>::hooksForPtr.end();) {
      bool remove = false;

      if (RegisteredGameHooks<H>::functionsForPtr[ptr].size() == 0) {
        break;
      }

      for (auto it = RegisteredGameHooks<H>::functionsForPtr[ptr].begin();
           it != RegisteredGameHooks<H>::functionsForPtr[ptr].end();) {
        if (it->first == *hookIdIt) {
          it = RegisteredGameHooks<H>::functionsForPtr[ptr].erase(it);
          remove = true;
          break;
        } else {
          ++it;
        }
      }

      if (remove) {
        hookIdIt = HooksToUnregister<H>::hooksForPtr.erase(hookIdIt);
      } else {
        ++hookIdIt;
      }
    }
    // Execute hooks
    for (auto &hook : RegisteredGameHooks<H>::functionsForPtr[ptr]) {
      hook.second(std::forward<Args>(args)...);
    }
  }

  // Filter based Hooks
  template <typename H> HOOK_ID RegisterGameHookForFilter(typename H::filter f, typename H::fn h) {
    if (this->nextHookId == 0 || this->nextHookId >= UINT32_MAX)
      this->nextHookId = 1;
    while (RegisteredGameHooks<H>::functionsForFilter.find(this->nextHookId) !=
           RegisteredGameHooks<H>::functionsForFilter.end()) {
      this->nextHookId++;
    }

    RegisteredGameHooks<H>::functionsForFilter[this->nextHookId] = std::make_pair(f, h);
    return this->nextHookId++;
  }
  template <typename H> void UnregisterGameHookForFilter(HOOK_ID hookId) {
    if (hookId == 0)
      return;
    HooksToUnregister<H>::hooksForFilter.push_back(hookId);
  }
  template <typename H, typename... Args> void ExecuteHooksForFilter(Args &&...args) {
    // Remove pending hooks for this type
    for (auto &hookId : HooksToUnregister<H>::hooksForFilter) {
      RegisteredGameHooks<H>::functionsForFilter.erase(hookId);
    }
    HooksToUnregister<H>::hooksForFilter.clear();
    // Execute hooks
    for (auto &hook : RegisteredGameHooks<H>::functionsForFilter) {
      if (hook.second.first(std::forward<Args>(args)...)) {
        hook.second.second(std::forward<Args>(args)...);
      }
    }
  }

  template <typename H> void ProcessUnregisteredHooks() {
    // Normal
    for (auto &hookId : HooksToUnregister<H>::hooks) {
      RegisteredGameHooks<H>::functions.erase(hookId);
    }
    HooksToUnregister<H>::hooks.clear();

    // ID
    for (auto &hookId : HooksToUnregister<H>::hooksForID) {
      for (auto &idGroup : RegisteredGameHooks<H>::functionsForID) {
        for (auto it = idGroup.second.begin(); it != idGroup.second.end();) {
          if (it->first == hookId) {
            it = idGroup.second.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
    HooksToUnregister<H>::hooksForID.clear();

    // Ptr
    for (auto &hookId : HooksToUnregister<H>::hooksForPtr) {
      for (auto &ptrGroup : RegisteredGameHooks<H>::functionsForPtr) {
        for (auto it = ptrGroup.second.begin(); it != ptrGroup.second.end();) {
          if (it->first == hookId) {
            it = ptrGroup.second.erase(it);
          } else {
            ++it;
          }
        }
      }
    }
    HooksToUnregister<H>::hooksForPtr.clear();

    // Filter
    for (auto &hookId : HooksToUnregister<H>::hooksForFilter) {
      RegisteredGameHooks<H>::functionsForFilter.erase(hookId);
    }
    HooksToUnregister<H>::hooksForFilter.clear();
  }

  void RemoveAllQueuedHooks() {
#define DEFINE_HOOK(name, _) ProcessUnregisteredHooks<name>();
#include "GameInteractor_HookTable.h"
#undef DEFINE_HOOK
  }

#define DEFINE_HOOK(name, args)                                                                                        \
  struct name {                                                                                                        \
    typedef std::function<void args> fn;                                                                               \
    typedef std::function<bool args> filter;                                                                           \
  };
#include "GameInteractor_HookTable.h"
#undef DEFINE_HOOK
};

#define REGISTER_VB_SHOULD(flag, body)                                                                                 \
  GameInteractor::Instance->RegisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(                              \
      flag, [](GIVanillaBehavior _, bool *should, va_list originalArgs) {                                              \
        va_list args;                                                                                                  \
        va_copy(args, originalArgs);                                                                                   \
        body;                                                                                                          \
        va_end(args);                                                                                                  \
      })
#define COND_HOOK(hookType, condition, body)                                                                           \
  {                                                                                                                    \
    static HOOK_ID hookId = 0;                                                                                         \
    GameInteractor::Instance->UnregisterGameHook<GameInteractor::hookType>(hookId);                                    \
    hookId = 0;                                                                                                        \
    if (condition) {                                                                                                   \
      hookId = GameInteractor::Instance->RegisterGameHook<GameInteractor::hookType>(body);                             \
    }                                                                                                                  \
  }
#define COND_ID_HOOK(hookType, id, condition, body)                                                                    \
  {                                                                                                                    \
    static HOOK_ID hookId = 0;                                                                                         \
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::hookType>(hookId);                               \
    hookId = 0;                                                                                                        \
    if (condition) {                                                                                                   \
      hookId = GameInteractor::Instance->RegisterGameHookForID<GameInteractor::hookType>(id, body);                    \
    }                                                                                                                  \
  }
#define COND_VB_SHOULD(id, condition, body)                                                                            \
  {                                                                                                                    \
    static HOOK_ID hookId = 0;                                                                                         \
    GameInteractor::Instance->UnregisterGameHookForID<GameInteractor::ShouldVanillaBehavior>(hookId);                  \
    hookId = 0;                                                                                                        \
    if (condition) {                                                                                                   \
      hookId = REGISTER_VB_SHOULD(id, body);                                                                           \
    }                                                                                                                  \
  }
