#include "GameInteractor.h"

GameInteractor *GameInteractor::Instance;

void GameInteractor_ExecuteOnControllerResume() {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnControllerResume>();
}

void GameInteractor_ExecuteOnControllerSuspend() {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnControllerSuspend>();
}

void GameInteractor_ExecuteOnControlsProcessed() {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnControlsProcessed>();
}

void GameInteractor_ExecuteOnDatabaseCreated() {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnDatabaseCreated>();
}

void GameInteractor_ExecuteOnLevelWarpListFilled(newuiListBox *list, int highestAllowed, int maxLevel) {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnLevelWarpListFilled>(list, highestAllowed, maxLevel);
}

void GameInteractor_ExecuteOnMissionListFilled(newuiListBox *list, std::vector<std::filesystem::path> *files) {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnMissionListFilled>(list, files);
}

void GameInteractor_ExecuteOnMissionStatusUpdate() {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnMissionStatusUpdate>();
}

void GameInteractor_ExecuteOnMovieEnd() { GameInteractor::Instance->ExecuteHooks<GameInteractor::OnMovieEnd>(); }

void GameInteractor_ExecuteOnMovieFrame(int x, int y, int width, int height) {
  GameInteractor::Instance->ExecuteHooks<GameInteractor::OnMovieFrame>(x, y, width, height);
}

bool GameInteractor_ShouldV(GIVanillaBehavior flag, uint32_t result, va_list args) {
  // Because of default argument promotion, even though our incoming "result" is just a bool, it needs to be typed as
  // an int to be permitted to be used in `va_start`, otherwise it is undefined behavior.
  // Here we downcast back to a bool for our actual hook handlers
  bool boolResult = static_cast<bool>(result);

  // Because there can be N subscribers registered to each template call, the subscribers will be responsible for
  // creating a copy of this va_list to avoid incrementing the original pointer between calls
  GameInteractor::Instance->ExecuteHooks<GameInteractor::ShouldVanillaBehavior>(flag, &boolResult, args);
  GameInteractor::Instance->ExecuteHooksForID<GameInteractor::ShouldVanillaBehavior>(flag, flag, &boolResult, args);
  GameInteractor::Instance->ExecuteHooksForFilter<GameInteractor::ShouldVanillaBehavior>(flag, &boolResult, args);

  return boolResult;
}

bool GameInteractor_Should(GIVanillaBehavior flag, uint32_t result, ...) {
  // Only the external function can use the Variadic Function syntax
  // To pass the va args to the next caller must be done using va_list and reading the args into it
  va_list args;
  va_start(args, result);
  bool boolResult = GameInteractor_ShouldV(flag, result, args);
  va_end(args);
  return boolResult;
}
