#pragma once

#include <cstdarg>
#include <cstdint>
#include <filesystem>
#include <vector>

#include "GameInteractor_VanillaBehavior.h"

class newuiListBox;
struct SDL_Window;

#if defined(_WIN32)
#define GI_EXPORT __declspec(dllexport)
#else
#define GI_EXPORT __attribute__((visibility("default")))
#endif

extern "C" {
void GameInteractor_ExecuteOnControlsProcessed();
void GameInteractor_ExecuteOnDatabaseCreated();
void GameInteractor_ExecuteOnLevelWarpListFilled(newuiListBox *list, int highestAllowed, int maxLevel);
void GameInteractor_ExecuteOnMissionListFilled(newuiListBox *list, std::vector<std::filesystem::path> *files);
void GameInteractor_ExecuteOnMissionStatusUpdate();
void GameInteractor_ExecuteOnMovieEnd();
void GameInteractor_ExecuteOnMovieFrame(int x, int y, int width, int height);

bool GameInteractor_Should(GIVanillaBehavior flag, uint32_t result, ...);
GI_EXPORT bool GameInteractor_ShouldV(GIVanillaBehavior flag, uint32_t result, va_list args);
}
