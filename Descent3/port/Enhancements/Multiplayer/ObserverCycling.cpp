#include <algorithm>
#include <vector>

#include "controller.h"
#include "controls.h"
#include "game.h"
#include "multi.h"
#include "multi_external.h"
#include "object.h"
#include "player.h"
#include "port/GameInteractor/GameInteractor.h"
#include "port/PortInit.hpp"

static void CycleObservedPlayer(float back, float ahead) {
  const int step = (ahead > 0.0f) - (back > 0.0f);
  if (!step) {
    return;
  }
  std::vector<int> views = {-1};
  for (int i = 0; i < MAX_PLAYERS; i++) {
    if (i != Player_num && (NetPlayers[i].flags & NPF_CONNECTED) && NetPlayers[i].sequence >= NETSEQ_PLAYING &&
        Objects[Players[i].objnum].type == OBJ_PLAYER) {
      views.push_back(Players[i].objnum);
    }
  }
  if (views.size() < 2) {
    return;
  }
  const int current = (int)(std::find(views.begin(), views.end(), Players[Player_num].piggy_objnum) - views.begin());
  const int next = (current + step + (int)views.size()) % (int)views.size();

  MultiSendRequestToObserve(OBSERVER_MODE_ROAM, 0, 0);
  if (views[next] == -1) {
    MultiSendRequestToObserve(OBSERVER_MODE_ROAM, 1, 0);
  } else {
    MultiSendRequestToObserve(OBSERVER_MODE_PIGGYBACK, 1, views[next]);
  }
}

static void ReadWatchingControls() {
  if (Player_object->type != OBJ_OBSERVER || Player_object->control_type == CT_FLYING || !Control_poll_flag) {
    return;
  }
  PollControls();
  ct_packet back, ahead;
  Controller->get_packet(ctfWPNSEL_PCYCLEBTN, &back);
  Controller->get_packet(ctfWPNSEL_SCYCLEBTN, &ahead);
  CycleObservedPlayer(back.value, ahead.value);
}

static void RegisterObserverCycling() {
  COND_HOOK(OnControlsProcessed, true, ReadWatchingControls);
  COND_VB_SHOULD(VB_CYCLE_WEAPONS, true, {
    if (Player_object->type == OBJ_OBSERVER) {
      const ct_packet *back = va_arg(args, ct_packet *);
      const ct_packet *ahead = va_arg(args, ct_packet *);
      CycleObservedPlayer(back->value, ahead->value);
      *should = false;
    }
  });
}

static RegisterPortInitFunc initFunc(RegisterObserverCycling);
